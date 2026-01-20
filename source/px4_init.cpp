/****************************************************************************
 *
 *   Copyright (c) 2019-2021 PX4 Development Team. All rights reserved.
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

#include <px4_platform_common/init.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/px4_manifest.h>
#include <px4_platform_common/console_buffer.h>
#include <px4_platform_common/defines.h>
#include <drivers/drv_hrt.h>
#include <lib/events/events.h>
#include <lib/parameters/param.h>
#include <px4_platform_common/px4_work_queue/WorkQueueManager.hpp>
#include <px4_platform/cpuload.h>
#include <uORB/uORB.h>

#include <fcntl.h>

#include <sys/mount.h>
#include <syslog.h>


#ifdef __cplusplus
extern "C" {
#endif

extern void oled_debug_step(int step, const char *msg);
extern void oled_debug_init(void);
extern void oled_debug_val(const char *label, int val);

#ifdef __cplusplus
}
#endif



#if defined(CONFIG_I2C)
# include <px4_platform_common/i2c.h>
# include <nuttx/i2c/i2c_master.h>
#endif // CONFIG_I2C

#if defined(PX4_CRYPTO)
#include <px4_platform_common/crypto.h>
#endif

#if !defined(CONFIG_BUILD_FLAT)
#include <px4_platform/board_ctrl.h>
#endif

#if !defined(CONFIG_BUILD_FLAT)
typedef CODE void (*initializer_t)(void);
extern initializer_t _sinit;
extern initializer_t _einit;
extern uint32_t _stext;
extern uint32_t _etext;

static void cxx_initialize(void)
{
	initializer_t *initp;

	/* Visit each entry in the initialization table */

	for (initp = &_sinit; initp != &_einit; initp++) {
		initializer_t initializer = *initp;

		/* Make sure that the address is non-NULL and lies in the text
		* region defined by the linker script.  Some toolchains may put
		* NULL values or counts in the initialization table.
		*/

		if ((FAR void *)initializer >= (FAR void *)&_stext &&
		    (FAR void *)initializer < (FAR void *)&_etext) {
			initializer();
		}
	}
}
#endif

#if defined(CONFIG_I2C)
void px4_platform_i2c_init()
{

	I2CBusIterator i2c_bus_iterator {I2CBusIterator::FilterType::All};
//	oled_debug_step(6, "P_i2c_bus_iterator");

	while (i2c_bus_iterator.next()) {
		
		
		
		
		// Per-bus debug prints before init
        char buf0[32];
        snprintf(buf0, sizeof(buf0), "Bus %d Init Start", i2c_bus_iterator.bus().bus);
        oled_debug_step(6, buf0);

        // Print relevant SEL register based on bus
        if (i2c_bus_iterator.bus().bus <= 3) {  // I2C1-3 use D2CCIP2R
            oled_debug_val("PerBus D2CCIP2R", getreg32(STM32_RCC_D2CCIP2R));
        } else if (i2c_bus_iterator.bus().bus == 4) {  // I2C4 uses D3CCIPR
            oled_debug_val("PerBus D3CCIPR", getreg32(STM32_RCC_D3CCIPR));
        }
        oled_debug_val("PerBus RCC_CR", getreg32(STM32_RCC_CR));
		
		
		
		
		
		
		
		i2c_master_s *i2c_dev = px4_i2cbus_initialize(i2c_bus_iterator.bus().bus);

// --- SAFETY CHECK START ---
        if (i2c_dev == nullptr) {
            oled_debug_step(6, "ERR: I2C Init Failed");
            continue; // Skip this bus, don't crash!
        }
        // --- SAFETY CHECK END ---


#if defined(CONFIG_I2C_RESET)
		I2C_RESET(i2c_dev);
#endif // CONFIG_I2C_RESET
//oled_debug_step(6, "P_px4_i2cbus_initialize OK");
		// send software reset to all
		uint8_t buf[1] {};
				buf[0] = 0x06; // software reset

		i2c_msg_s 	msg{};
					msg.frequency = I2C_SPEED_STANDARD;
					msg.addr = 0x00; // general call address
					msg.buffer = &buf[0];
					msg.length = 1;






		I2C_TRANSFER(i2c_dev, &msg, 1);

		px4_i2cbus_uninitialize(i2c_dev);
	}
}

#endif // CONFIG_I2C



#include <sched.h>
#include <nuttx/sched.h>
 
 

static void dump_all_tasks()
{
	syslog(LOG_INFO, "[TASK-DUMP] === All Running Tasks/Threads ===\n");
	syslog(LOG_INFO, "[TASK-DUMP] PID   PRI  STACK_SIZE  NAME\n");

	int count = 0;

	nxsched_foreach([](struct tcb_s *tcb, void *arg) {
		int *cnt = (int *)arg;
		(*cnt)++;

		syslog(LOG_INFO, "[TASK-DUMP] %3d   %3d  %5u      %s\n",
		       tcb->pid,
		       tcb->sched_priority,
		       (unsigned)tcb->adj_stack_size,
		       tcb->name ? tcb->name : "<unnamed>");
	}, &count);

	syslog(LOG_INFO, "[TASK-DUMP] Total tasks: %d\n", count);
	syslog(LOG_INFO, "[TASK-DUMP] ==================================\n");
}

int px4_platform_init()
{

#if !defined(CONFIG_BUILD_FLAT)
	cxx_initialize();

	/* initialize userspace-kernelspace call gate interface */
	kernel_ioctl_initialize();
#endif

	int ret = px4_console_buffer_init();

	if (ret < 0) {		return ret;	}
//oled_debug_step(6, "P_px4_console_buffer_init");
	// replace stdout with our buffered console
	int fd_buf = open(CONSOLE_BUFFER_DEVICE, O_WRONLY);

	if (fd_buf >= 0) {
		dup2(fd_buf, 1);
		// keep stderr(2) untouched: the buffered console will use it to output to the original console
		close(fd_buf);
	}

#if defined(PX4_CRYPTO)
	PX4Crypto::px4_crypto_init();
#endif

	hrt_init();
//oled_debug_step(6, "P_hrt_init");
#if !defined(CONFIG_BUILD_FLAT)
	hrt_ioctl_init();
	events_ioctl_init();
#endif

	/* configure CPU load estimation */
#ifdef CONFIG_SCHED_INSTRUMENTATION
	cpuload_initialize_once();
	oled_debug_step(6, "P_cpuload_initialize_once");
#endif


uint32_t d2 = getreg32(STM32_RCC_D2CCIP2R);
uint32_t d3 = getreg32(STM32_RCC_D3CCIPR);
uint32_t cr = getreg32(STM32_RCC_CR);
oled_debug_val("D2CCIP2R", d2);
oled_debug_val("D3CCIPR", d3);
oled_debug_val("RCC_CR", cr);



#if defined(CONFIG_I2C) && !defined(BOARD_I2C_LATEINIT)
	px4_platform_i2c_init();
//	oled_debug_step(6, "P_px4_platform_i2c_init");
#endif

#if defined(CONFIG_FS_PROCFS)
	int ret_mount_procfs = mount(nullptr, "/proc", "procfs", 0, nullptr);

	if (ret_mount_procfs < 0) {
		syslog(LOG_ERR, "ERROR: Failed to mount procfs at /proc: %d\n", ret_mount_procfs);
	}
	oled_debug_step(6, "P_CONFIG_FS_PROCFS");

#endif // CONFIG_FS_PROCFS

#if defined(CONFIG_FS_BINFS)
	int ret_mount_binfs = nx_mount(nullptr, "/bin", "binfs", 0, nullptr);

	if (ret_mount_binfs < 0) {
		syslog(LOG_ERR, "ERROR: Failed to mount binfs at /bin: %d\n", ret_mount_binfs);
	}
oled_debug_step(6, "before  WorkQueueManagerStart();	");
#endif // CONFIG_FS_BINFS




 

// --- ADD THIS DEBUG BLOCK START ---
    syslog(LOG_INFO, "[TIMER-DEBUG] Testing HRT Timer...\n");
    
    // 1. Get current time
    hrt_abstime t1 = hrt_absolute_time();
    
    // 2. Wait for roughly 10ms using a busy loop 
    // (We use a busy loop because if the timer is broken, usleep might hang too!)
    volatile int k = 0;
    for(int i=0; i<500000; i++) { k++; } 

    // 3. Get time again
    hrt_abstime t2 = hrt_absolute_time();
    
    syslog(LOG_INFO, "[TIMER-DEBUG] T1: %llu, T2: %llu, Diff: %llu\n", t1, t2, t2-t1);
    
    if (t2 == t1 || t2 == 0) {
        syslog(LOG_ERR, "[TIMER-DEBUG] CRITICAL FAILURE: HRT Timer is NOT counting! Check TIM2 clock in defconfig.\n");
        // Force a print to your OLED if available
        oled_debug_step(6, "HRT DEAD!");
    } else {
        syslog(LOG_INFO, "[TIMER-DEBUG] SUCCESS: Timer is running.\n");
        oled_debug_step(6, "HRT ALIVE");
    }
    // --- ADD THIS DEBUG BLOCK END ---










//	px4::WorkQueueManagerStart();		




int wq_ret = px4::WorkQueueManagerStart();		


// Immediately dump tasks to see if wq:manager appeared
dump_all_tasks();

// Also call the built-in status function (it prints queue info if running)
px4::WorkQueueManagerStatus();

if (wq_ret == PX4_OK) {
	syslog(LOG_INFO, "[WQ-DEBUG] WorkQueueManager started successfully\n");
	oled_debug_step(6, "WQ OK");
} else {
	syslog(LOG_ERR, "[WQ-DEBUG] WorkQueueManagerStart FAILED (%d)\n", wq_ret);
	oled_debug_step(6, "WQ FAIL");
}









	
	
	oled_debug_step(6, "P_WorkQueueManagerStart");

	param_init();		oled_debug_step(6, "P_param_init");

	uorb_start();  oled_debug_step(6, "P_uorb_start");

	px4_log_initialize(); oled_debug_step(6, "P_px4_log_initialize");

	return PX4_OK;
}

int px4_platform_configure(void)
{
	return px4_mft_configure(board_get_manifest());

}










