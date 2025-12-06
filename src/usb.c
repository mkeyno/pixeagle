/****************************************************************************
 * boards/pixeagle/pixeagle/src/usb.c
 *
 * Pixeagle USB initialization
 *
 ****************************************************************************/

#include <px4_platform_common/px4_config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <debug.h>

#include <nuttx/usb/usbdev.h>
#include <nuttx/usb/usbdev_trace.h>

#include <arm_internal.h>
#include <chip.h>

#include <stm32_gpio.h>
#include <stm32_otg.h>

#include "board_config.h"

/************************************************************************************
 * Definitions
 ************************************************************************************/

/************************************************************************************
 * Private Functions
 ************************************************************************************/

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: stm32_usbinitialize
 *
 * Description:
 *   Called to setup USB-related GPIO pins for the Pixeagle board.
 *
 *   Pixeagle USB Configuration:
 *     - PA9:  OTG_FS_VBUS (VBUS sensing)
 *     - PA11: OTG_FS_DM   (USB D-)
 *     - PA12: OTG_FS_DP   (USB D+)
 *
 *   The STM32H7 OTG FS peripheral has an internal soft pull-up on D+,
 *   so no external pull-up resistor is needed.
 *
 ************************************************************************************/

__EXPORT void stm32_usbinitialize(void)
{
	/* The OTG FS has an internal soft pull-up */

	/* Configure the OTG FS VBUS sensing GPIO */
#ifdef CONFIG_STM32H7_OTGFS
	stm32_configgpio(GPIO_OTGFS_VBUS);
#endif
}

/************************************************************************************
 * Name:  stm32_usbsuspend
 *
 * Description:
 *   Board logic must provide the stm32_usbsuspend logic if the USBDEV driver is
 *   used.  This function is called whenever the USB enters or leaves suspend mode.
 *   This is an opportunity for the board logic to shutdown clocks, power, etc.
 *   while the USB is suspended.
 *
 ************************************************************************************/

__EXPORT void stm32_usbsuspend(FAR struct usbdev_s *dev, bool resume)
{
	uinfo("resume: %d\n", resume);
	
	/* Pixeagle: No special power management needed for USB suspend/resume */
}