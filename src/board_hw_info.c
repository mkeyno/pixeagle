/****************************************************************************
 * boards/px4/pixeagle/src/board_hw_info.c
 *
 * Pixeagle hardware version detection
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <syslog.h>

/* Global hardware version variables - must be uint32_t */
uint32_t board_hw_version  = 0;
uint32_t board_hw_revision = 0;

/* Hardware version detection function */
int board_determine_hw_info(void)
{
    /* Pixeagle has a single hardware revision */
    board_hw_version  = 0;
    board_hw_revision = 0;

    syslog(LOG_INFO, "[boot] Pixeagle hardware version: %u, revision: %u\n",
           (unsigned)board_hw_version, 
           (unsigned)board_hw_revision);

    return 0; /* OK */
}