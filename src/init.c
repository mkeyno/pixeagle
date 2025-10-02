/**
 * @file init.c
 *
 * Pixeagle-specific early startup code. This file implements the
 * board_app_initialize() function called early by nsh during startup.
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

#ifdef CONFIG_CAN
#include <nuttx/can/can.h>
#endif

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

#ifndef CONFIG_STM32_SDMMC
#  define CONFIG_STM32_SDMMC 0
#endif

/* LED definitions */
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
 ************************************************************************************/
__EXPORT void board_peripheral_reset(int ms)
{
    /* Disable sensor power rail */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_EN, 0);

    /* Wait for power rail to stabilize */
    usleep(ms * 1000);
    syslog(LOG_DEBUG, "reset done, %d ms\n", ms);

    /* Re-enable sensor power rail */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_EN, 1);
}

/************************************************************************************
 * Name: board_on_reset
 *
 * Description: Called on entry to board_system_reset for housekeeping.
 *
 ************************************************************************************/
__EXPORT void board_on_reset(int status)
{
    /* Configure PWM pins as GPIO outputs during reset */
    for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
        px4_arch_configgpio(io_timer_channel_get_gpio_output(i));
    }

    /* On non-boot resets, set PWM pins low to disarm ESCs */
    if (status >= 0) {
        up_mdelay(100);
    }
}

/************************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description: Early STM32 initialization after memory configuration.
 *
 ************************************************************************************/
__EXPORT void stm32_boardinitialize(void)
{
    board_on_reset(-1);

    /* Configure LEDs */
    led_init();

    /* Configure GPIO pins */
    const uint32_t gpio[] = PX4_GPIO_INIT_LIST;
    px4_gpio_init(gpio, arraySize(gpio));

    /* Configure USB interfaces */
    stm32_usbinitialize();

    /* Initialize SPI buses */
    stm32_spiinitialize();
}

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description: Perform application-specific initialization.
 *
 ****************************************************************************/
__EXPORT int board_app_initialize(uintptr_t arg)
{
#if !defined(BOOTLOADER)

    /* Power on MCU and sensor rails */
    px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_EN, 1);

    /* Initialize high-resolution timer */
    px4_platform_init();

    /* Log hardware version info */
    if (OK == board_determine_hw_info()) {
        syslog(LOG_INFO, "[boot] Rev 0x%1x : Ver 0x%1x %s\n", 
               board_get_hw_revision(), board_get_hw_version(), board_get_hw_type_name());
    } else {
        syslog(LOG_ERR, "[boot] Failed to read HW revision and version\n");
    }

    /* Reset SPI sensors */
    board_peripheral_reset(10);

    /* Configure DMA allocator */
    if (board_dma_alloc_init() < 0) {
        syslog(LOG_ERR, "[boot] DMA alloc FAILED\n");
    }

#if defined(SERIAL_HAVE_RXDMA)
    /* Poll serial DMA at 1ms intervals */
    static struct hrt_call serial_dma_call;
    hrt_call_every(&serial_dma_call, 1000, 1000, (hrt_callout)stm32_serial_dma_poll, NULL);
#endif

    /* Initialize MicroSD on SPI2 */
#ifdef CONFIG_MMCSD_SPI
    struct spi_dev_s *spi = stm32_spibus_initialize(2);
    if (spi != NULL) {
        int ret = mmcsd_spislotinitialize(0, 0, spi);
        if (ret != OK) {
            syslog(LOG_ERR, "[boot] MicroSD initialization failed: %d\n", ret);
        }
    }
#endif

    /* Initialize hardfault handler */
    if (board_hardfault_init(2, true) != 0) {
        led_on(6);  /* LED state 6 is error state from led.cpp */
    }

    /* Configure hardware based on manifest */
    px4_platform_configure();

#endif
    return OK;
}
