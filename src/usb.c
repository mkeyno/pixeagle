/**
 * @file usb.c
 *
 * Pixeagle USB OTG FS configuration
 *
 * Configures USB OTG Full Speed (FS) for the Pixeagle board (STM32H743VIT6, version V1C00).
 * USB pins: PA9 (VBUS), PA11 (DM), PA12 (DP). No power control or overcurrent detection.
 * Hardware: [as per previous]
 */

#include <px4_platform_common/px4_config.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <debug.h>
#include <nuttx/usb/usbdev.h>
#include <nuttx/usb/composite.h>
#include <nuttx/usb/usbdev_trace.h>
#include <arm_internal.h>
#include <chip.h>
#include <stm32_gpio.h>
#include <stm32_otg.h>
#include "board_config.h"

/************************************************************************************
 * Definitions
 ************************************************************************************/

#ifdef CONFIG_STM32H7_OTGFS
#ifndef GPIO_OTGFS_VBUS
#define GPIO_OTGFS_VBUS  (GPIO_INPUT | GPIO_FLOAT | GPIO_SPEED_100MHz | GPIO_OPENDRAIN | GPIO_PORTA | GPIO_PIN9)
#endif

#define BOARD_HAS_OTGFS_VBUS_SENSE 1


#ifndef GPIO_OTGFS_DM
#define GPIO_OTGFS_DM    (GPIO_ALT | GPIO_AF10 | GPIO_SPEED_100MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN11)
#endif
#ifndef GPIO_OTGFS_DP
#define GPIO_OTGFS_DP    (GPIO_ALT | GPIO_AF10 | GPIO_SPEED_100MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN12)
#endif
#endif

/************************************************************************************
 * Public Functions
 ************************************************************************************/

__EXPORT void stm32_usbinitialize(void)
{
#ifdef CONFIG_STM32H7_OTGFS
    /* Enable USB OTG FS clock */
    stm32_rcc_enable_usb_otg_fs();  // Or directly: modify_rcc(RCC->AHB1ENR, 0, RCC_AHB1ENR_OTGHSEN);

    /* Configure pins */
    stm32_configgpio(GPIO_OTGFS_VBUS);
    stm32_configgpio(GPIO_OTGFS_DM);
    stm32_configgpio(GPIO_OTGFS_DP);

    /* Enable VBUS detection (critical for pull-up trigger) */
    USB_OTG_FS->GCCFG |= OTG_GCCFG_VBDEN;  // Set VBDEN bit
composite_initialize();
    uinfo("USB OTG FS initialized: VBUS PA9, DM PA11, DP PA12\n");
#endif
}

__EXPORT void stm32_usbsuspend(FAR struct usbdev_s *dev, bool resume)
{
#ifdef CONFIG_STM32H7_OTGFS
    uinfo("USB OTG FS %s\n", resume ? "resumed" : "suspended");
#endif
}
