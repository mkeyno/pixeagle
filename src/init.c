/****************************************************************************
 * boards/pixeagle/pixeagle/src/init.c
 *
 * Pixeagle board initialization
 *
 ****************************************************************************/

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
#include <nuttx/sdio.h>
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

#include <mpu.h>

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/
 /* Hardware version detection */
uint32_t board_hw_version  = 0;
uint32_t board_hw_revision = 0;
 
/* Configuration ************************************************************/

/*
 * Ideally we'd be able to get these from arm_internal.h,
 * but since we want to be able to disable the NuttX use
 * of leds for system indication at will and there is no
 * separate switch, we need to build independent of the
 * CONFIG_ARCH_LEDS configuration switch.
 */
__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
__END_DECLS


/************************************************************************************
 * Name: board_peripheral_reset
 *
 * Description:
 *   Reset peripheral power rails
 *
 ************************************************************************************/
__EXPORT void board_peripheral_reset(int ms)
{
	/* Set the peripheral rails off */
	/* Pixeagle: PA15 controls 3.3V sensor power (active high) */
	VDD_3V3_SENSORS_EN(false);
	board_control_spi_sensors_power(false, 0xffff);

	/* Wait for the peripheral rail to reach GND */
	usleep(ms * 1000);
	syslog(LOG_DEBUG, "reset done, %d ms\n", ms);

	/* Re-enable power */
	board_control_spi_sensors_power(true, 0xffff);
	VDD_3V3_SENSORS_EN(true);
}

/************************************************************************************
 * Name: board_on_reset
 *
 * Description:
 *   Optionally provided function called on entry to board_system_reset
 *   It should perform any house keeping prior to the reset.
 *
 * status - 1 if resetting to boot loader
 *          0 if just resetting
 *
 ************************************************************************************/
__EXPORT void board_on_reset(int status)
{
	/* Configure all PWM pins as inputs to prevent spurious signals during reset */
	for (int i = 0; i < DIRECT_PWM_OUTPUT_CHANNELS; ++i) {
		px4_arch_configgpio(PX4_MAKE_GPIO_INPUT(io_timer_channel_get_as_pwm_input(i)));
	}

	/*
	 * On resets invoked from system (not boot) ensure we establish a low
	 * output state on PWM pins to disarm the ESCs and prevent the reset from
	 * potentially spinning up the motors.
	 */
	if (status >= 0) {
		up_mdelay(100);
	}
}

/************************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description:
 *   All STM32 architectures must provide the following entry point. This entry point
 *   is called early in the initialization -- after all memory has been configured
 *   and mapped but before any devices have been initialized.
 *
 ************************************************************************************/

__EXPORT void
stm32_boardinitialize(void)
{
	/* Reset PWM first thing */
	board_on_reset(-1);

	/* Configure LEDs (including WS2812B initialization) */
	board_autoled_initialize();

	/* Configure GPIO pins */
	const uint32_t gpio[] = PX4_GPIO_INIT_LIST;
	px4_gpio_init(gpio, arraySize(gpio));

	/* Configure SPI sensor power control */
	board_control_spi_sensors_power_configgpio();

	/* Configure USB interfaces */
	stm32_usbinitialize();
}

/****************************************************************************
 * Name: board_app_initialize
 *
 * Description:
 *   Perform application specific initialization. This function is never
 *   called directly from application code, but only indirectly via the
 *   (non-standard) boardctl() interface using the command BOARDIOC_INIT.
 *
 * Input Parameters:
 *   arg - The boardctl() argument is passed to the board_app_initialize()
 *         implementation without modification. The argument has no
 *         meaning to NuttX; the meaning of the argument is a contract
 *         between the board-specific initalization logic and the
 *         matching application logic. The value could be such things as a
 *         mode enumeration value, a set of DIP switch switch settings, a
 *         pointer to configuration data read from a file or serial FLASH,
 *         or whatever you would like to do with it. Every implementation
 *         should accept zero/NULL as a default configuration.
 *
 * Returned Value:
 *   Zero (OK) is returned on success; a negated errno value is returned on
 *   any failure to indicate the nature of the failure.
 *
 ****************************************************************************/

__EXPORT int board_app_initialize(uintptr_t arg)
{
	/* Power on Interfaces */
	/* Pixeagle: Only has 3.3V sensor power on PA15 */
	VDD_3V3_SENSORS_EN(true);
	board_control_spi_sensors_power(true, 0xffff);

	/* Need hrt running before using the ADC */
	px4_platform_init();

	/* Initialize SPI buses */
	stm32_spiinitialize();

	/* Configure the HW based on the manifest */
	px4_platform_configure();

	/* Determine hardware version (Pixeagle doesn't have HW version detection) */
	if (OK == board_determine_hw_info()) {
		syslog(LOG_INFO, "[boot] Pixeagle Rev 0x%1x : Ver 0x%1x %s\n", 
		       board_get_hw_revision(), 
		       board_get_hw_version(),
		       board_get_hw_type_name());
	} else {
		syslog(LOG_INFO, "[boot] Pixeagle board initialized\n");
	}

	/* Configure the actual SPI interfaces (after we determined the HW version) */
	stm32_spiinitialize();

	/* Configure the DMA allocator */
	if (board_dma_alloc_init() < 0) {
		syslog(LOG_ERR, "[boot] DMA alloc FAILED\n");
	}

#if defined(SERIAL_HAVE_RXDMA)
	/* Set up the serial DMA polling at 1ms intervals for received bytes that have not triggered a DMA event */
	static struct hrt_call serial_dma_call;
	hrt_call_every(&serial_dma_call, 1000, 1000, (hrt_callout)stm32_serial_dma_poll, NULL);
#endif

	/* Initial LED state */
	drv_led_start();
	led_off(LED_RED);
	led_on(LED_GREEN);  /* Indicate power is on */
	/* Note: WS2812B LED on PE14 can be controlled via bitbang in led.c */

	/* Initialize hardfault logging */
	if (board_hardfault_init(2, true) != 0) {
		led_on(LED_RED);
	}

	/* Initialize SD card - no separate SD card power control on Pixeagle */
	/* SD card is on SPI2 with CS on PB11 */
	usleep(500 * 1000);

#ifdef CONFIG_MMCSD
	int ret = stm32_sdio_initialize();

	if (ret != OK) {
		led_on(LED_RED);
		syslog(LOG_ERR, "[boot] Failed to initialize SD card: %d\n", ret);
	}
#endif /* CONFIG_MMCSD */

	return OK;
}


/****************************************************************************
 * Name: board_determine_hw_info
 *
 * Description:
 *   This function is called very early to set hardware version/revision.
 *   Pixeagle has only one hardware version.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on any failure.
 *
 ****************************************************************************/

__EXPORT int board_determine_hw_info(void)
{
    board_hw_version  = 0;
    board_hw_revision = 0;

    syslog(LOG_INFO, "[boot] Pixeagle hardware version: %u, revision: %u\n",  
           (unsigned)board_hw_version, (unsigned)board_hw_revision);

    return OK;
}