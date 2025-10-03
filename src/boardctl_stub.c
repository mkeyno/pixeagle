/**
 * @file boardctl_stub.c
 * Stub implementation for boardctl
 */

#include <nuttx/config.h>
#include <sys/boardctl.h>
#include <errno.h>
#include <syslog.h>

int boardctl(unsigned int cmd, uintptr_t arg)
{
    syslog(LOG_INFO, "boardctl called: cmd=%u\n", cmd);
    
    switch (cmd) {
        case BOARDIOC_RESET:
            /* System reset */
            return 0;
            
        case BOARDIOC_POWEROFF:  
            /* Power off */
            return 0;
            
        default:
            return -ENOSYS;
    }
}