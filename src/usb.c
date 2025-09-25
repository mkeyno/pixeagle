/**
 * @file usb.c
 *
 * Pixeagle USB OTG FS configuration
 *
 * Configures USB OTG Full Speed (FS) for the Pixeagle board (STM32H743VIT6, version V1C00).
 * USB pins: PA9 (VBUS), PA11 (DM), PA12 (DP). No power control or overcurrent detection.
 * Hardware:
 * - BMI088 (SPI1, PA4 CS_ACC, PB2 CS_GYR, PA5–PA7, 20 MHz)  // UPDATED: GYR CS
 * - ICM-42688-P (SPI4, PE4 CS, PE2/PE5/PE6, 20 MHz)
 * - IST8310 (I2C3, PA8 SCL/PC9 SDA, 0x0E), BMP388 (I2C3, 0x76), BMP390 (I2C4, PB8 SCL/PB9 SDA, 0x76)
 * - MicroSD (SPI2, PB11 CS, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz)
 * - FM25V01A-GTR (SPI2, PD10 CS, 20 MHz)
 * - External SPI (SPI3, PB3 SCK, PB4 MISO, PB5 MOSI, PD7 CS, 10 MHz)
 * - Dual WS2812B LEDs (PE14, TIM1)
 * - UART4 (PC10/PC11, debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5 (PC12/PD2, sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V, /dev/ttyS0, 57600 baud)  // UPDATED: Pins
 * - UART7 (PE7/PE8, CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - UART3 (PD8/PD9, SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent: 0 normal, 1 inverted)
 * - CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1)  // UPDATED: CAN1
 * No PX4IO co-processor. USB operates at 5V with internal pull-up.
 */

/************************************************************************************
 * Included Files
 ************************************************************************************/

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

#ifdef CONFIG_STM32H7_OTGFS
/* USB OTG FS pins */
#define GPIO_OTGFS_VBUS  (GPIO_INPUT | GPIO_FLOAT | GPIO_SPEED_100MHz | GPIO_OPENDRAIN | GPIO_PORTA | GPIO_PIN9)
#define GPIO_OTGFS_DM    (GPIO_ALT | GPIO_AF10 | GPIO_SPEED_100MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN11)
#define GPIO_OTGFS_DP    (GPIO_ALT | GPIO_AF10 | GPIO_SPEED_100MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN12)
#endif

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: stm32_usbinitialize
 *
 * Description:
 *   Initializes USB OTG FS for Pixeagle, configuring VBUS (PA9), DM (PA11), DP (PA12).
 *   No power control or overcurrent detection per board design.
 *
 ************************************************************************************/

__EXPORT void stm32_usbinitialize(void)
{
#ifdef CONFIG_STM32H7_OTGFS
    /* Configure USB OTG FS pins */
    stm32_configgpio(GPIO_OTGFS_VBUS);  // VBUS sensing (PA9)
    stm32_configgpio(GPIO_OTGFS_DM);    // Data minus (PA11)
    stm32_configgpio(GPIO_OTGFS_DP);    // Data plus (PA12)

    uinfo("USB OTG FS initialized: VBUS PA9, DM PA11, DP PA12\n");
#endif
}

/************************************************************************************
 * Name: stm32_usbsuspend
 *
 * Description:
 *   Handles USB suspend/resume events with logging. No power control implemented.
 *
 ************************************************************************************/

__EXPORT void stm32_usbsuspend(FAR struct usbdev_s *dev, bool resume)
{
#ifdef CONFIG_STM32H7_OTGFS
    uinfo("USB OTG FS %s\n", resume ? "resumed" : "suspended");
#endif
}
