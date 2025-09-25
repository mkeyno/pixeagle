/**
 * @file sdio.c
 *
 * Pixeagle MicroSD initialization (disabled, uses SPI2 instead)
 *
 * This file is intended for SDIO-based MMC/SD card support, but for the Pixeagle board
 * (STM32H743VIT6), MicroSD is implemented on SPI2 (PB11 CS, PB10, PB14, PB15) along with
 * FM25V01A-GTR (PD10 CS). SDIO/SDMMC is disabled to avoid conflicts with UART5 (PC12/PD2, sensor module).
 * See spi.cpp for MicroSD and FRAM initialization. Hardware includes:
 * - BMI088 (SPI1, PA4–PA7, CS: PA4/PC4)
 * - ICM-42688-P (SPI4, PE2–PE6, CS: PE4, DRDY: PE3)  // FIXED: CS on PE4, DRDY on PE3
 * - IST8310 (I2C3, PC9/PA8, 0x0E)
 * - BMP388 (I2C3, 0x76)
 * - BMP390 (I2C4, PB8/PB9, 0x76)
 * - Dual WS2812B LEDs (PE14, TIM1)
 * - UART4 (PC10/PC11, debug, 5V)
 * - UART5 (PC12/PD2, sensor module, 5V)
 * - USART2 (PA2/PA3, telemetry with flow control on PA0/PA1, 5V)
 * - UART7 (PE7/PE8, CM4/ESP32, 5V)
 * - CAN1 (PB8/PB9, 5V via TCAN1044VDRQ1)
 * - CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1)
 */

#include <nuttx/config.h>
#include <board_config.h>

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <errno.h>
#include <syslog.h>  // Added for syslog

#include <nuttx/sdio.h>
#include <nuttx/mmcsd.h>

#include "chip.h"
#include "stm32_gpio.h"
#include "stm32_sdmmc.h"

#ifdef CONFIG_MMCSD

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   SDIO-based MMC/SD card support is disabled for Pixeagle. MicroSD is
 *   initialized on SPI2 (PB11 CS, PB10, PB14, PB15) via stm32_spisd_initialize()
 *   in init.c and spi.cpp. This function returns an error to prevent SDIO usage.
 *
 * Returned Value:
 *   -ENODEV (device not supported)
 *
 ****************************************************************************/

int stm32_sdio_initialize(void)
{
    syslog(LOG_ERR, "[boot] SDIO/SDMMC is disabled on Pixeagle. MicroSD uses SPI2 (PB11 CS). See spi.cpp.\n");
    return -ENODEV;
}

#endif /* CONFIG_MMCSD */