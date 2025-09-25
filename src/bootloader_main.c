/**
 * @file bootloader_main.c
 *
 * Pixeagle-specific early startup code for bootloader
 *
 * This file configures early hardware initialization for the Pixeagle board,
 * based on STM32H743VIT6, with BMI088 (SPI1), ICM-42688-P (SPI4), IST8310 (I2C3, 0x0E),
 * BMP388 (I2C3, 0x76), BMP390 (I2C4, 0x76), FM25V01A-GTR (SPI2), MicroSD (SPI2),
 * dual WS2812B LEDs on PE14, UART4 (PC10/PC11, debug, 5V), UART5 (PC12/PD2, sensor module, 5V),
 * USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry with flow control, 5V), UART7 (PE7/PE8, CM4/ESP32, 5V).  // UPDATED: USART2 pins
 * Initializes sensor power rail (PA15, active low) and serial console (UART4, /dev/ttyS3).
 */

#include "board_config.h"
#include "bl.h"

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <chip.h>
#include <stm32_uart.h>
#include <arch/board/board.h>
#include <px4_platform_common/init.h>

/**
 * @brief Late initialization for the board
 *
 * This function is called after basic hardware setup to initialize the serial
 * console and power up peripherals. For Pixeagle, it enables the sensor power
 * rail (PA15, active high) to power sensors (BMI088, ICM-42688-P, IST8310, BMP388,
 * BMP390). The serial console is set up on UART4 (PC10/PC11, /dev/ttyS3, 5V) for
 * debug output. Other UARTs (UART5, USART2, UART7) are configured in the main firmware.
 */
void board_late_initialize(void)
{
    /* Enable sensor power rail (PA15, active low) */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_nEN, 1); // Enable (active high)

    /* Initialize serial console on UART4 (PC10/PC11, /dev/ttyS3, 5V) for bootloader debug */
    sercon_main(0, NULL);
}

/**
 * @brief Timer hook for system tick handling
 *
 * This function is called by the system tick interrupt to handle timing-related
 * tasks in the bootloader. It ensures proper timing for bootloader operations
 * on the STM32H743VIT6.
 */
extern void sys_tick_handler(void);
void board_timerhook(void)
{
    sys_tick_handler();
}