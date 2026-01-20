/****************************************************************************
 *
 *   Copyright (C) 2012 PX4 Development Team. All rights reserved.
 *   Author: @author Lorenz Meier <lm@inf.ethz.ch>
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

/**
 * @file tasks.cpp
 * Implementation of existing task API for NuttX
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/tasks.h>

#include <nuttx/board.h>
#include <nuttx/kthread.h>

#include <sys/wait.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <stdbool.h>


#include <syslog.h> // Ensure this is included
 


int px4_task_spawn_cmd(const char *name, int scheduler, int priority, int stack_size, main_t entry, char *const argv[])
{
	
	// 1. Data Source Validation (Before Kernel call)
	syslog(LOG_INFO, "[SPAWN-IN] Name: %s, Prio: %d, Stack: %d, Sch: %d\n", name, priority, stack_size, scheduler);
	
	// Log caller info: return address (helps map which module called spawn) 
	void *caller = __builtin_return_address(0); 
	syslog(LOG_INFO, "[SPAWN-IN] Caller return address: %p\n", caller); // Optionally, if backtrace is available: 
	
#ifdef CONFIG_SCHED_BACKTRACE
	FAR struct tcb_s *current_tcb = nxsched_self();
	void *bt[8];
	int bt_size = up_backtrace(current_tcb, bt, 8, 0);  // Added 4th argument 'skip = 0'

	syslog(LOG_INFO, "[SPAWN-IN] Caller backtrace (%d frames):\n", bt_size);
	for (int i = 0; i < bt_size; i++) {
		syslog(LOG_INFO, "  [%d] %p\n", i, bt[i]);
	}
#endif
	
	
	
	
	
	
	if (argv) {
				int i = 0;
				while (argv[i] != nullptr) {
					syslog(LOG_INFO, "   > Arg[%d]: %s\n", i, argv[i]);
					i++;				}
			} 
	else    syslog(LOG_INFO, "   > Args: NULL\n");


	sched_lock();

#if !defined(CONFIG_DISABLE_ENVIRON)
	/* None of the modules access the environment variables (via getenv() for instance), so delete them
	 * all. They are only used within the startup script, and NuttX automatically exports them to the children
	 * tasks.
	 * This frees up a considerable amount of RAM.
	 */
	clearenv();
#endif

#if !defined(__KERNEL__)
	/* create the task */
	int pid = task_create(name, priority, stack_size, entry, argv);
#else
	int pid = kthread_create(name, priority, stack_size, entry, argv);
#endif

if (pid > 0) {
			/* OS Accepted the request */
			struct sched_param param = { .sched_priority = priority };

			/* Verify the scheduler type requested */
			int sched_policy = (scheduler == SCHED_FIFO) ? SCHED_FIFO : SCHED_RR;
			
			int ret = sched_setscheduler(pid, sched_policy, &param);
			
			if (ret != OK)		syslog(LOG_ERR, "[SPAWN-ERR] %s: sched_setscheduler failed (errno=%d)\n", name, errno);
			else 				syslog(LOG_INFO, "[SPAWN-OK] %s spawned with PID %d\n", name, pid);
			

		} 
	else {
		/* OS Rejected the request - This is where we need the most depth */
		int err_code = errno;
		syslog(LOG_ERR, "[SPAWN-REJECT] %s failed!\n", name);
		syslog(LOG_ERR, "   > Reason Code (errno): %d\n", err_code);
		
		#ifdef CONFIG_SCHED_HAVE_PARENT // call nxsched_foreach to count tasks and print names 
			int tcount = 0; 
			nxsched_foreach([](struct tcb_s *tcb, void *arg) { 
																int *cnt = (int *)arg; 
																(*cnt)++; 
																syslog(LOG_INFO, "[KERN-TCB] PID=%d PRI=%d NAME=%s\n", tcb->pid, tcb->sched_priority, tcb->name ? tcb->name : "<null>");
																}, &tcount); 
			syslog(LOG_INFO, "[KERN-TCB] total kernel tasks: %d\n", tcount); 
		#endif
		
		/* Depth: Interpretation of common NuttX rejection codes */
		switch(err_code) {
						case ENOMEM: // 12
							syslog(LOG_ERR, "   > Depth: Kernel failed to allocate %d bytes for stack.\n", stack_size);
							break;
						case ENOSPC: // 28
						case 128:    // Often maps to task limit in custom defconfigs
							syslog(LOG_ERR, "   > Depth: Task Table Full (CONFIG_MAX_TASKS reached).\n");
							break;
						default:
							syslog(LOG_ERR, "   > Depth: Unexpected rejection. Check kernel heap/limits.\n");
							break;
						}
	}

	sched_unlock();

	return pid;
}





int px4_task_delete(int pid)
{
    // Depth: Trace who is killing which PID
    syslog(LOG_KERN, "[TASK-TRACE] Request to DELETE PID %d\n", pid);
    
    int ret = task_delete(pid);
    
    if (ret != OK) {
        syslog(LOG_ERR, "[TASK-TRACE] Delete PID %d FAILED (errno=%d)\n", pid, errno);
    } else {
        syslog(LOG_INFO, "[TASK-TRACE] Delete PID %d SUCCESS\n", pid);
    }
    
	return ret;
}

const char *px4_get_taskname(void)
{
#if CONFIG_TASK_NAME_SIZE > 0 && (defined(__KERNEL__) || defined(CONFIG_BUILD_FLAT))
	FAR struct tcb_s *thisproc = nxsched_self();

    // Depth: Ensure we aren't getting a NULL TCB or empty name
    if (thisproc == nullptr) {
        return "unknown_tcb";
    }

	return thisproc->name;
#else
    // If this returns "app", it means your NuttX config has 
    // CONFIG_TASK_NAME_SIZE=0 or you are in a protected build.
	return "app";
#endif
}
