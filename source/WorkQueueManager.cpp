/****************************************************************************
 *
 *   Copyright (c) 2019 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <px4_platform_common/px4_work_queue/WorkQueueManager.hpp>

#include <px4_platform_common/px4_work_queue/WorkQueue.hpp>

#include <drivers/drv_hrt.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/time.h>
#include <px4_platform_common/atomic.h>
#include <containers/BlockingList.hpp>
#include <containers/BlockingQueue.hpp>
#include <lib/drivers/device/Device.hpp>
#include <lib/mathlib/mathlib.h>

#include <limits.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>  // Add this for errno logging

#include <malloc.h> 
#include <nuttx/mm/mm.h> // for mm_foreach if available 
#include <nuttx/sched.h> // for nxsched_foreach 
#include <nuttx/arch.h> // for up_getsp / tcb access if needed 
#include <sys/types.h> 
#include <inttypes.h>



using namespace time_literals;

namespace px4
{

// list of current work queues
static BlockingList<WorkQueue *> *_wq_manager_wqs_list{nullptr};

// queue of WorkQueues to be created (as threads in the wq manager task)
static BlockingQueue<const wq_config_t *, 1> *_wq_manager_create_queue{nullptr};

static px4::atomic_bool _wq_manager_should_exit{true};
static px4::atomic_bool _wq_manager_running{false};


static void DumpMemoryInfo(const char *tag)
{
    syslog(LOG_INFO, "[MEM-DUMP] === Memory dump: %s ===", tag);

    // Portable mallinfo usage: print fields that are commonly available
    struct mallinfo mi = mallinfo();
    syslog(LOG_INFO, "[MEM-DUMP] mallinfo: arena=%ld ordblks=%ld uordblks=%ld fordblks=%ld",
           (long)mi.arena, (long)mi.ordblks, (long)mi.uordblks, (long)mi.fordblks);

#if defined(CONFIG_MM_REGIONS) || defined(CONFIG_MM_KERNEL_HEAP)
    // If mm_foreach is available in your NuttX build, it will already be linked.
    // We attempt to call mm_foreach-style diagnostics if available at compile time.
    syslog(LOG_INFO, "[MEM-DUMP] Attempting mm_foreach style region dump (if supported)...");
    // The actual mm_foreach output may already be produced by other code paths.
#endif

#if defined(CONFIG_SCHED_HAVE_PARENT) || defined(CONFIG_SCHED)
    // Try to enumerate tasks and print basic stack info if nxsched_foreach is available.
    syslog(LOG_INFO, "[MEM-DUMP] Task list (if nxsched_foreach available):");
#ifdef CONFIG_SCHED_HAVE_PARENT
    int tcount = 0;
    nxsched_foreach([](struct tcb_s *tcb, void *arg) {
        int *cnt = (int *)arg;
        (*cnt)++;
        const char *name = tcb->name ? tcb->name : "<unnamed>";
        int pid = tcb->pid;
        int pri = tcb->sched_priority;
        uintptr_t sp = (uintptr_t)tcb->adj_stack_ptr;
        unsigned adj_stack = (unsigned)tcb->adj_stack_size;
        syslog(LOG_INFO, "[MEM-DUMP] PID=%3d PRI=%3d STACK=%6u NAME=%s SP=0x%08" PRIxPTR,
               pid, pri, adj_stack, name, sp);
    }, &tcount);
    syslog(LOG_INFO, "[MEM-DUMP] Total tasks enumerated: %d", tcount);
#endif
#endif

#if defined(HAVE_MALLOC_HEAP)
    void *brk = sbrk(0);
    syslog(LOG_INFO, "[MEM-DUMP] sbrk(0) = %p", brk);
#endif

    syslog(LOG_INFO, "[MEM-DUMP] === End memory dump: %s ===", tag);
}





static WorkQueue *
FindWorkQueueByName(const char *name)
{
	if (!_wq_manager_running.load()) {
		PX4_ERR("not running");
		return nullptr;
	}

	LockGuard lg{_wq_manager_wqs_list->mutex()};

	// search list
	for (WorkQueue *wq : *_wq_manager_wqs_list) {
		if (strcmp(wq->get_name(), name) == 0) {
			return wq;
		}
	}

	return nullptr;
}

WorkQueue *
WorkQueueFindOrCreate(const wq_config_t &new_wq)
{
	if (!_wq_manager_running.load()) {
		PX4_ERR("not running");
		return nullptr;
	}

	// search list for existing work queue
	WorkQueue *wq = FindWorkQueueByName(new_wq.name);

	// create work queue if it doesn't already exist
	if (wq == nullptr) {
		// add WQ config to list
		//  main thread wakes up, creates the thread
		_wq_manager_create_queue->push(&new_wq);

		// we wait until new wq is created, then return
		uint64_t t = 0;

		while (wq == nullptr && t < 10_s) {
			// Wait up to 10 seconds, checking every 1 ms
			t += 1_ms;
			px4_usleep(1_ms);

			wq = FindWorkQueueByName(new_wq.name);
		}

		if (wq == nullptr) {
			PX4_ERR("failed to create %s", new_wq.name);
		}
	}

	return wq;
}

const wq_config_t &
device_bus_to_wq(uint32_t device_id_int)
{
	union device::Device::DeviceId device_id;
	device_id.devid = device_id_int;

	const device::Device::DeviceBusType bus_type = device_id.devid_s.bus_type;
	const uint8_t bus = device_id.devid_s.bus;

	if (bus_type == device::Device::DeviceBusType_I2C) {
		switch (bus) {
		case 0: return wq_configurations::I2C0;

		case 1: return wq_configurations::I2C1;

		case 2: return wq_configurations::I2C2;

		case 3: return wq_configurations::I2C3;

		case 4: return wq_configurations::I2C4;
		}

	} else if (bus_type == device::Device::DeviceBusType_SPI) {
		switch (bus) {
		case 0: return wq_configurations::SPI0;

		case 1: return wq_configurations::SPI1;

		case 2: return wq_configurations::SPI2;

		case 3: return wq_configurations::SPI3;

		case 4: return wq_configurations::SPI4;

		case 5: return wq_configurations::SPI5;

		case 6: return wq_configurations::SPI6;
		}
	}

	// otherwise use high priority
	return wq_configurations::hp_default;
};

const wq_config_t &
serial_port_to_wq(const char *serial)
{
	if (serial == nullptr) {
		return wq_configurations::ttyUnknown;

	} else if (strstr(serial, "ttyS0")) {
		return wq_configurations::ttyS0;

	} else if (strstr(serial, "ttyS1")) {
		return wq_configurations::ttyS1;

	} else if (strstr(serial, "ttyS2")) {
		return wq_configurations::ttyS2;

	} else if (strstr(serial, "ttyS3")) {
		return wq_configurations::ttyS3;

	} else if (strstr(serial, "ttyS4")) {
		return wq_configurations::ttyS4;

	} else if (strstr(serial, "ttyS5")) {
		return wq_configurations::ttyS5;

	} else if (strstr(serial, "ttyS6")) {
		return wq_configurations::ttyS6;

	} else if (strstr(serial, "ttyS7")) {
		return wq_configurations::ttyS7;

	} else if (strstr(serial, "ttyS8")) {
		return wq_configurations::ttyS8;

	} else if (strstr(serial, "ttyS9")) {
		return wq_configurations::ttyS9;

	} else if (strstr(serial, "ttyACM0")) {
		return wq_configurations::ttyACM0;
	}

	PX4_DEBUG("unknown serial port: %s", serial);

	return wq_configurations::ttyUnknown;
}

const wq_config_t &ins_instance_to_wq(uint8_t instance)
{
	switch (instance) {
	case 0: return wq_configurations::INS0;

	case 1: return wq_configurations::INS1;

	case 2: return wq_configurations::INS2;

	case 3: return wq_configurations::INS3;
	}

	PX4_WARN("no INS%d wq configuration, using INS0", instance);

	return wq_configurations::INS0;
}

static void *
WorkQueueRunner(void *context)
{
	wq_config_t *config = static_cast<wq_config_t *>(context);
	syslog(LOG_INFO, "[WQ-WORKER] Runner started for: %s\n", config->name);

	WorkQueue wq(*config);

	syslog(LOG_INFO, "[WQ-WORKER] Adding %s to manager list\n", config->name);
	_wq_manager_wqs_list->add(&wq);

	syslog(LOG_INFO, "[WQ-WORKER] %s entering loop\n", config->name);
	wq.Run();

	syslog(LOG_INFO, "[WQ-WORKER] %s exiting loop, removing from list\n", config->name);
	_wq_manager_wqs_list->remove(&wq);

	return nullptr;
}


#if defined(__PX4_NUTTX) && !defined(CONFIG_BUILD_FLAT)
// Wrapper for px4_task_spawn_cmd interface
inline static int
WorkQueueRunner(int argc, char *argv[])
{
	wq_config_t *context = (wq_config_t *)strtoul(argv[argc - 1], nullptr, 16);
	WorkQueueRunner(context);
	return 0;
}
#endif


// Add near the top of the file (file-scope)
static 			px4_sem_t 					_wq_manager_start_sem;
static bool 	_wq_manager_start_sem_init = false;


// WorkQueueManagerRun: manager thread main loop
static int WorkQueueManagerRun(int, char **){
    syslog(LOG_INFO, "[WQ-DBG] WorkQueueManagerRun: Thread started. Initializing lists...");
    syslog(LOG_INFO, "[WQ-MANAGER] Thread started (PID: %d)", getpid());

    _wq_manager_wqs_list = new BlockingList<WorkQueue *>();
    _wq_manager_create_queue = new BlockingQueue<const wq_config_t *, 1>();

    if (!_wq_manager_wqs_list || !_wq_manager_create_queue) {
        syslog(LOG_ERR, "[WQ-MANAGER] Failed to allocate structures");
        if (_wq_manager_start_sem_init) {
            _wq_manager_running.store(false);
            px4_sem_post(&_wq_manager_start_sem);
        }
        return -ENOMEM;
    }

    syslog(LOG_INFO, "[WQ-MANAGER] Structures allocated OK");

    // Mark running and notify starter
    _wq_manager_running.store(true);

    if (_wq_manager_start_sem_init) {
        px4_sem_post(&_wq_manager_start_sem);
    }

    syslog(LOG_INFO, "[WQ-DBG] WorkQueueManagerRun: Entering main loop");

    while (!_wq_manager_should_exit.load()) {
        const wq_config_t *wq = _wq_manager_create_queue->pop();

        if (wq != nullptr) {
            syslog(LOG_INFO, "[WQ-DBG] Manager: Pop request for queue: %s", wq->name);

            // stack size calculation
#if defined(__PX4_NUTTX) || defined(__PX4_QURT)
            const size_t stacksize = math::max(PTHREAD_STACK_MIN, PX4_STACK_ADJUSTED(wq->stacksize));
#elif defined(__PX4_POSIX)
            const unsigned int page_size = sysconf(_SC_PAGESIZE);
            const size_t stacksize_adj = math::max((int)PTHREAD_STACK_MIN, PX4_STACK_ADJUSTED(wq->stacksize));
            const size_t stacksize = (stacksize_adj + page_size - (stacksize_adj % page_size));
#endif

            // priority calculation - clamp into valid range for SCHED_FIFO
            int sched_priority = 0;
            int maxp = 0;
            int minp = 0;

            if (sched_get_priority_max(SCHED_FIFO) > 0) {
                maxp = sched_get_priority_max(SCHED_FIFO);
                minp = sched_get_priority_min(SCHED_FIFO);
                sched_priority = maxp + wq->relative_priority;
                if (sched_priority > maxp) { sched_priority = maxp; }
                if (sched_priority < minp) { sched_priority = minp; }
            } else {
                // fallback
                sched_priority = SCHED_PRIORITY_DEFAULT;
            }

#if !defined(__PX4_NUTTX) || defined(CONFIG_BUILD_FLAT)
            // POSIX pthread path
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, stacksize);
            pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

            sched_param param;
            pthread_attr_getschedparam(&attr, &param);
            param.sched_priority = sched_priority;
            pthread_attr_setschedparam(&attr, &param);

            pthread_t thread;
            syslog(LOG_INFO, "[WQ-DBG] Manager: pthread_create for %s (pri: %d, stack: %zu)", wq->name, sched_priority, stacksize);
            int ret_create = pthread_create(&thread, &attr, WorkQueueRunner, (void *)wq);

            if (ret_create != 0) {
                syslog(LOG_ERR, "[WQ-DBG] pthread_create failed for %s (%d: %s)", wq->name, ret_create, strerror(ret_create));
            }

            pthread_attr_destroy(&attr);

#else
            // NuttX task spawn path
            char arg1[sizeof(void *) * 3];
            sprintf(arg1, "%lx", (long unsigned)wq);
            const char *arg[2] = {arg1, nullptr};

            syslog(LOG_INFO, "[WQ-DBG] Manager: px4_task_spawn_cmd for %s (policy=SCHED_FIFO pri=%d stack=%zu)",
                   wq->name, sched_priority, stacksize);

            int pid = px4_task_spawn_cmd(wq->name,
                                         SCHED_FIFO,
                                         sched_priority,
                                         stacksize,
                                         WorkQueueRunner,
                                         (char *const *)arg);

            if (pid <= 0) {
                int err = errno;
                syslog(LOG_ERR, "[WQ-DBG] px4_task_spawn_cmd failed for %s (ret=%d errno=%d %s)",
                       wq->name, pid, err, strerror(err));
                // Dump memory to help debugging
                DumpMemoryInfo("wq_spawn_failed");
            }
#endif
        }
    }

    _wq_manager_running.store(false);
    syslog(LOG_INFO, "[WQ-DBG] WorkQueueManagerRun: Thread exiting");
    return 0;
}


// WorkQueueManagerStart: spawn manager with diagnostics and fallback attempts
int WorkQueueManagerStart(){
    syslog(LOG_INFO, "[WQ-DBG] WorkQueueManagerStart: Entry");
    syslog(LOG_INFO, "[WQ-DBG] Flags on entry: should_exit=%d running=%d",
           (int)_wq_manager_should_exit.load(), (int)_wq_manager_running.load());

    if (_wq_manager_running.load()) {
        syslog(LOG_INFO, "[WQ-DBG] Already running");
        return PX4_OK;
    }

    // Initialize start semaphore once
    if (!_wq_manager_start_sem_init) {
        px4_sem_init(&_wq_manager_start_sem, 0, 0);
        _wq_manager_start_sem_init = true;
    }

    _wq_manager_should_exit.store(false);

    // Choose safe defaults
    int requested_stack = PX4_STACK_ADJUSTED(4096);
    int maxp = sched_get_priority_max(SCHED_FIFO);
    int minp = sched_get_priority_min(SCHED_FIFO);
    int requested_prio = (maxp > 0) ? (maxp - 5) : SCHED_PRIORITY_DEFAULT;

    if (requested_prio < minp) {
        requested_prio = minp;
    }

    struct mallinfo mem_info = mallinfo();
    syslog(LOG_INFO, "[WQ-DBG] Spawning wq:manager task... (stack=%d prio=%d) heap_free=%d  SCHED_FIFO max=%d min=%d requested=%d ", requested_stack, requested_prio, mem_info.fordblks, maxp, minp, requested_prio);

    DumpMemoryInfo("pre-spawn");

#if defined(__PX4_NUTTX)
    // Try primary spawn
    int task_id = px4_task_spawn_cmd("wq:manager",
                                     SCHED_FIFO,
                                     requested_prio,
                                     requested_stack,
                                     (px4_main_t)&WorkQueueManagerRun,
                                     nullptr);

    if (task_id < 0) {
        int err = errno;
        syslog(LOG_ERR, "[WQ-DBG] px4_task_spawn_cmd failed: ret=%d errno=%d (%s)", task_id, err, strerror(err));
        DumpMemoryInfo("spawn-failed");

        // Fallback attempt: reduce stack and priority and try again
        int fallback_stack = PX4_STACK_ADJUSTED(2048);
        int fallback_prio = (minp + maxp) / 2;
        syslog(LOG_INFO, "[WQ-DBG] Attempting fallback spawn (stack=%d prio=%d)...", fallback_stack, fallback_prio);

        int task_id2 = px4_task_spawn_cmd("wq:manager",
                                          SCHED_DEFAULT,
                                          fallback_prio,
                                          fallback_stack,
                                          (px4_main_t)&WorkQueueManagerRun,
                                          nullptr);

        if (task_id2 < 0) {
            int err2 = errno;
            syslog(LOG_ERR, "[WQ-DBG] Fallback spawn also failed: ret=%d errno=%d (%s)", task_id2, err2, strerror(err2));
            DumpMemoryInfo("fallback-spawn-failed");
            return PX4_ERROR;
        }

        task_id = task_id2;
    }

#else
    // POSIX path: spawn as pthread wrapper via px4_task_spawn_cmd or directly create a thread
    int task_id = px4_task_spawn_cmd("wq:manager",
                                     SCHED_DEFAULT,
                                     requested_prio,
                                     requested_stack,
                                     (px4_main_t)&WorkQueueManagerRun,
                                     nullptr);

    if (task_id < 0) {
        syslog(LOG_ERR, "[WQ-DBG] px4_task_spawn_cmd (POSIX) failed: ret=%d", task_id);
        DumpMemoryInfo("spawn-failed-posix");
        return PX4_ERROR;
    }
#endif

    DumpMemoryInfo("post-spawn");

    syslog(LOG_INFO, "[WQ-DBG] Manager spawn requested (task_id=%d). Waiting for manager to signal running...", task_id);

    // Wait for the manager to post the semaphore (bounded wait)
    const int wait_ms = 5000; // 5 seconds max
    int waited = 0;
    const int step_ms = 10;

    while (waited < wait_ms) {
        if (px4_sem_trywait(&_wq_manager_start_sem) == 0) {
            syslog(LOG_INFO, "[WQ-DBG] Manager signaled running.");
            return PX4_OK;
        }
        px4_usleep(step_ms * 1000);
        waited += step_ms;
    }

    syslog(LOG_ERR, "[WQ-DBG] Timeout waiting for wq:manager to start (task_id=%d)", task_id);
    DumpMemoryInfo("timeout-waiting-for-manager");

    return PX4_ERROR;
}


int WorkQueueManagerStop(){
	if (!_wq_manager_should_exit.load()) {

		// error can't shutdown until all WorkItems are removed/stopped
		if (_wq_manager_running.load() && (_wq_manager_wqs_list->size() > 0)) {
			PX4_ERR("can't shutdown with active WQs");
			WorkQueueManagerStatus();
			return PX4_ERROR;
		}

		// first ask all WQs to stop
		if (_wq_manager_wqs_list != nullptr) {
			{
				LockGuard lg{_wq_manager_wqs_list->mutex()};

				// ask all work queues (threads) to stop
				// NOTE: not currently safe without all WorkItems stopping first
				for (WorkQueue *wq : *_wq_manager_wqs_list) {
					wq->request_stop();
				}
			}

			// wait until they're all stopped (empty list)
			while (_wq_manager_wqs_list->size() > 0) {
				px4_usleep(1000);
			}

			delete _wq_manager_wqs_list;
			_wq_manager_wqs_list = nullptr;
		}

		_wq_manager_should_exit.store(true);

		if (_wq_manager_create_queue != nullptr) {
			// push nullptr to wake the wq manager task
			_wq_manager_create_queue->push(nullptr);

			px4_usleep(10000);

			delete _wq_manager_create_queue;
			_wq_manager_create_queue = nullptr;
		}

	} else {
		PX4_WARN("not running");
		return PX4_ERROR;
	}

	return PX4_OK;
}

int WorkQueueManagerStatus(){
	if (!_wq_manager_should_exit.load() && _wq_manager_running.load()) {

		const size_t num_wqs = _wq_manager_wqs_list->size();
		PX4_INFO_RAW("\nWork Queue: %-2zu threads                          RATE        INTERVAL\n", num_wqs);

		LockGuard lg{_wq_manager_wqs_list->mutex()};
		size_t i = 0;

		for (WorkQueue *wq : *_wq_manager_wqs_list) {
			i++;

			const bool last_wq = (i >= num_wqs);

			if (!last_wq) {
				PX4_INFO_RAW("|__ %zu) ", i);

			} else {
				PX4_INFO_RAW("\\__ %zu) ", i);
			}

			wq->print_status(last_wq);
		}

	} else {
		PX4_INFO("not running");
	}

	return PX4_OK;
}



} // namespace px4


 


