/****************************************************************************
 * boards/pixeagle/pixeagle/src/bootloader_main.c
 *
 * Pixeagle bootloader main initialization
 *
 ****************************************************************************/

#include "board_config.h"
#include "bl.h"

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <chip.h>
#include <stm32_uart.h>
#include <arch/board/board.h>

#include "arm_internal.h"

#include <px4_platform_common/init.h>

extern int sercon_main(int c, char **argv);

/****************************************************************************
 * Name: board_on_reset
 *
 * Description:
 *   Called on reset - allows board-specific cleanup before reset
 *
 ****************************************************************************/

__EXPORT void board_on_reset(int status) 
{
	/* Pixeagle: No specific reset actions needed */
}

/****************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description:
 *   All STM32 architectures must provide the following entry point. This
 *   entry point is called early in the initialization -- after basic CPU
 *   configuration is complete but before any devices have been initialized.
 *
 ****************************************************************************/

__EXPORT void stm32_boardinitialize(void)
{
	/* Configure USB interfaces for Pixeagle bootloader */
	stm32_usbinitialize();
}

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform application specific initialization.  This function is never
 *   called directly from application code, but only indirectly via the
 *   (non-standard) boardctl() interface.
 *
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
	/* Pixeagle: No application-specific initialization needed in bootloader */
	return 0;
}

/****************************************************************************
 * Name: board_late_initialize
 *
 * Description:
 *   Called during late bootloader initialization to setup serial console
 *
 ****************************************************************************/

void board_late_initialize(void)
{
	/* Initialize serial console for bootloader communication */
	sercon_main(0, NULL);
}

/****************************************************************************
 * Name: sys_tick_handler
 *
 * Description:
 *   External system tick handler
 *
 ****************************************************************************/

extern void sys_tick_handler(void);

/****************************************************************************
 * Name: board_timerhook
 *
 * Description:
 *   Timer hook called from the system tick
 *
 ****************************************************************************/

void board_timerhook(void)
{
	/* Call bootloader system tick handler */
	sys_tick_handler();
}

/****************************************************************************
 * Name: arm_netinitialize
 *
 * Description:
 *   Stub for networking initialization - bootloader doesn't need it
 *
 ****************************************************************************/

__EXPORT void arm_netinitialize(void)
{
	/* Bootloader doesn't use networking */
}