#include <nuttx/config.h>
#include <nuttx/board.h>
#include <syslog.h>
#include <errno.h>
#include <stdint.h>
 
#include "board_config.h"

__EXPORT void board_early_initialize(void)
{
}

__EXPORT void board_late_initialize(void)
{
}

/* Update this function to match the single-argument signature */
__EXPORT int board_uniqueid(uint8_t *uniqueid)
{
    if (uniqueid == NULL) {
        return -EINVAL;
    }
    
    /* STM32H7 Unique Device ID register address */
    const uint32_t *uid = (const uint32_t *)0x1FF1E800;
    
    uniqueid[0] = (uid[0] >> 24) & 0xFF;
    uniqueid[1] = (uid[0] >> 16) & 0xFF;
    uniqueid[2] = (uid[0] >> 8) & 0xFF;
    uniqueid[3] = uid[0] & 0xFF;
    
    uniqueid[4] = (uid[1] >> 24) & 0xFF;
    uniqueid[5] = (uid[1] >> 16) & 0xFF;
    uniqueid[6] = (uid[1] >> 8) & 0xFF;
    uniqueid[7] = uid[1] & 0xFF;
    
    uniqueid[8] = (uid[2] >> 24) & 0xFF;
    uniqueid[9] = (uid[2] >> 16) & 0xFF;
    uniqueid[10] = (uid[2] >> 8) & 0xFF;
    uniqueid[11] = uid[2] & 0xFF;
    
    return 0; /* Return OK/0 as seen in stm32_uid.c */
}

__EXPORT int board_boot_image(FAR const char *path, uint32_t hdrtype)
{
    return -ENOSYS;
}

__EXPORT int board_ioctl(unsigned int cmd, uintptr_t arg)
{
    return -ENOTTY;
}
