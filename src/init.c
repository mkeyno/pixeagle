/**
 * @file init.c
 *
 * Pixeagle-specific early startup code. This file implements the
 * board_app_initialize() function called early by nsh during startup.
 *
 * Code here runs before the rcS script, starting required subsystems and
 * performing board-specific initialization for STM32H743VIT6, with BMI088 (SPI1),
 * ICM-42688-P (SPI4), IST8310 (I2C3, 0x0E), BMP388 (I2C3, 0x76), BMP390 (I2C4, 0x76),
 * FM25V01A-GTR (SPI2), MicroSD (SPI2, PB11 CS), dual WS2812B LEDs (PE14),
 * UART4 (PC10/PC11, debug, 5V), UART5 (PC12/PD2, sensor module, 5V),
 * USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry with flow control, 5V), UART7 (PE7/PE8, CM4/ESP32, 5V),  // UPDATED: USART2 pins
 * CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1).  // UPDATED: CAN1 pins
 */

#include "board_config.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <errno.h>
#include <syslog.h>

#include <nuttx/config.h>
#include <nuttx/board.h>
#include <nuttx/spi/spi.h>
#include <nuttx/mmcsd.h>
#include <nuttx/analog/adc.h>
#include <nuttx/mm/gran.h>
#include <chip.h>
#include <stm32_uart.h>
#include <arch/board/board.h>
#include "arm_internal.h"

#include <drivers/drv_hrt.h>
#include <drivers/drv_board_led.h>
#include <systemlib/px4_macros.h>
#include <px4_arch/io_timer.h>
#include <px4_platform_common/init.h>
#include <px4_platform/gpio.h>
#include <px4_platform/board_determine_hw_info.h>
#include <px4_platform/board_dma_alloc.h>

/* Include CAN initialization header */
#ifdef CONFIG_CAN
#include <nuttx/can/can.h>
#endif

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifndef CONFIG_STM32_SDMMC
#  define CONFIG_STM32_SDMMC 0  // Use SPI2 for MicroSD (PB11 CS), not SDMMC
#endif

/* LED definitions for Pixeagle (no CONFIG_ARCH_LEDS, using custom LEDs) */
__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
__END_DECLS

/************************************************************************************
 * Name: board_peripheral_reset
 *
 * Description: Reset peripheral power rails (sensors via PA15, active High).
 *
 * Input Parameters: ms - Duration to hold reset in milliseconds.
 *
 ************************************************************************************/
__EXPORT void board_peripheral_reset(int ms)
{
    /* Disable sensor power rail */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_nEN, 0); // Active low to disable

    /* Wait for power rail to stabilize */
    usleep(ms * 1000);
    syslog(LOG_DEBUG, "reset done, %d ms\n", ms);

    /* Re-enable sensor power rail */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_nEN, 1); // Active high to enable
}

/************************************************************************************
 * Name: board_on_reset
 *
 * Description: Called on entry to board_system_reset for housekeeping.
 *
 * Input Parameters: status - 1 if resetting to bootloader, 0 if just resetting.
 *
 ************************************************************************************/
__EXPORT void board_on_reset(int status)
{
    /* Configure PWM pins as GPIO outputs during reset */
    for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) { px4_arch_configgpio(io_timer_channel_get_gpio_output(i));    }

    /* On non-boot resets, set PWM pins low to disarm ESCs */
    if (status >= 0) {        up_mdelay(100);    }
}

/************************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description: Early STM32 initialization after memory configuration.
 *
 ************************************************************************************/
__EXPORT void stm32_boardinitialize(void)
{
    board_on_reset(-1); /* Reset PWM first */

    /* Configure LEDs (dual WS2812B on PE14, safety light on PE15, buzzer on PE13) */
    board_autoled_initialize();

    /* Configure GPIO pins */
    const uint32_t gpio[] = PX4_GPIO_INIT_LIST;
    px4_gpio_init(gpio, arraySize(gpio));

    /* Configure USB interfaces (PA11/PA12, VBUS on PA9) */
    stm32_usbinitialize();

    /* Initialize CAN interfaces (CAN1: PD0/PD1, CAN2: PB12/PB13, 5V via TCAN1044VDRQ1) */  // UPDATED: CAN1 pins
#ifdef CONFIG_CAN
    int ret = can_devinit();
    if (ret != OK) {
        syslog(LOG_ERR, "[boot] CAN initialization failed: %d\n", ret);
    }
#endif
}

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description: Perform application-specific initialization via boardctl(BOARDIOC_INIT).
 *
 * Input Parameters: arg - Boardctl argument (board/application-specific).
 *
 * Returned Value: 0 (OK) on success; negated errno on failure.
 *
 ****************************************************************************/
__EXPORT int board_app_initialize(uintptr_t arg)
{
#if !defined(BOOTLOADER)

    /* Power on MCU and sensor rails (PA15, active low) */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_nEN, 0); // Enable sensor power rail

    /* Initialize high-resolution timer (HRT) before ADC use */
    px4_platform_init();

    /* Log hardware version info */
    if (OK == board_determine_hw_info())	syslog(LOG_INFO, "[boot] Rev 0x%1x : Ver 0x%1x %s\n", board_get_hw_revision(), board_get_hw_version(), board_get_hw_type_name());					 
	else         							syslog(LOG_ERR, "[boot] Failed to read HW revision and version\n");
    
    /* Initialize SPI buses (SPI1 for BMI088, SPI2 for FRAM/SD, SPI4 for ICM-42688-P) */
    stm32_spiinitialize();

    /* Reset SPI sensors (BMI088, ICM-42688-P) */
    board_peripheral_reset(10); // Use our custom reset function

    /* Configure DMA allocator */
    if (board_dma_alloc_init() < 0)        syslog(LOG_ERR, "[boot] DMA alloc FAILED\n");
   

#if defined(SERIAL_HAVE_RXDMA)
    /* Poll serial DMA at 1ms intervals for UART4, UART5, USART2, UART7 */
    static struct hrt_call 		serial_dma_call;
				hrt_call_every(&serial_dma_call, 1000, 1000, (hrt_callout)stm32_serial_dma_poll, NULL);
#endif

    /* Initialize LED driver (WS2812B on PE14) */
    led_init(); // Use our custom LED init function

    /* Initialize hardfault handler */
    if (board_hardfault_init(2, true) != 0) led_on(6); 	// Use LED_STATE_ERROR (value 6) from our custom LED implementation

    /* Initialize MicroSD on SPI2 (PB11 CS, PB10, PB14, PB15) */
    int ret = stm32_spisd_initialize();
    if (ret != OK) {
        syslog(LOG_ERR, "[boot] MicroSD (SPI2) initialization failed: %d\n", ret);
        led_on(6); // Use LED_STATE_ERROR (value 6) from our custom LED implementation
    }

    /* Configure hardware based on manifest (UARTs, CAN, PWM, etc.) */
    px4_platform_configure();

#endif
    return OK;
}