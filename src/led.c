/****************************************************************************
 * boards/pixeagle/pixeagle/src/led.c
 *
 * Pixeagle LED control including WS2812B RGB LED bitbang implementation
 *
 ****************************************************************************/

#include <px4_platform_common/px4_config.h>

#include <stdbool.h>

#include "chip.h"
#include "stm32_gpio.h"
#include "board_config.h"

#include <nuttx/board.h>
#include <arch/board/board.h>

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
extern void led_toggle(int led);
__END_DECLS

/****************************************************************************
 * Pixeagle LED Configuration:
 * - PE13: Red LED (open drain, active low)
 * - PE14: WS2812B RGB LED (bitbang, active high)
 * - PE15: Green LED / Safety Light (open drain, active low)
 ****************************************************************************/

#ifdef CONFIG_ARCH_LEDS
static bool nuttx_owns_leds = true;
//                                R  G  S  (no separate blue, using WS2812B)
//                                0  1  2
static const uint8_t xlatpx4[] = {0, 1, 2};
#  define xlat(p) xlatpx4[(p)]

static uint32_t g_ledmap[] = {
	GPIO_nLED_RED,               // Indexed by BOARD_LED_RED / LED_RED
	GPIO_nLED_GREEN,             // Indexed by BOARD_LED_GREEN / LED_GREEN
	GPIO_SAFETY_SWITCH_LED,      // Indexed by LED_SAFETY
};

#else

#  define xlat(p) (p)
static uint32_t g_ledmap[] = {
	GPIO_nLED_RED,               // Indexed by LED_RED / LED_AMBER
	GPIO_nLED_GREEN,             // Indexed by LED_GREEN
	GPIO_SAFETY_SWITCH_LED,      // Indexed by LED_SAFETY
};

#endif

/****************************************************************************
 * WS2812B Control (PE14)
 * Simple bitbang implementation for RGB LED
 ****************************************************************************/

/* WS2812B timing (at 480MHz CPU clock, adjust cycles as needed):
 * - T0H: 0.4us (HIGH for 0 bit) ~192 cycles
 * - T0L: 0.85us (LOW for 0 bit) ~408 cycles
 * - T1H: 0.8us (HIGH for 1 bit) ~384 cycles
 * - T1L: 0.45us (LOW for 1 bit) ~216 cycles
 * These are approximate and may need tuning
 */

#define WS2812B_PIN GPIO_LED_WS2812B

static inline void ws2812b_send_bit(bool bit)
{
	if (bit) {
		/* Send '1' bit: long HIGH, short LOW */
		stm32_gpiowrite(WS2812B_PIN, true);
		for (volatile int i = 0; i < 38; i++) { __asm__ __volatile__("nop"); } /* ~0.8us HIGH */
		stm32_gpiowrite(WS2812B_PIN, false);
		for (volatile int i = 0; i < 22; i++) { __asm__ __volatile__("nop"); } /* ~0.45us LOW */
	} else {
		/* Send '0' bit: short HIGH, long LOW */
		stm32_gpiowrite(WS2812B_PIN, true);
		for (volatile int i = 0; i < 19; i++) { __asm__ __volatile__("nop"); } /* ~0.4us HIGH */
		stm32_gpiowrite(WS2812B_PIN, false);
		for (volatile int i = 0; i < 41; i++) { __asm__ __volatile__("nop"); } /* ~0.85us LOW */
	}
}

static void ws2812b_send_byte(uint8_t byte)
{
	for (int i = 7; i >= 0; i--) {
		ws2812b_send_bit((byte >> i) & 0x01);
	}
}

/**
 * Set WS2812B RGB LED color
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 */
__EXPORT void ws2812b_set_color(uint8_t r, uint8_t g, uint8_t b)
{
	irqstate_t flags = enter_critical_section();
	
	/* WS2812B expects GRB order */
	ws2812b_send_byte(g);
	ws2812b_send_byte(r);
	ws2812b_send_byte(b);
	
	leave_critical_section(flags);
	
	/* Reset pulse (>50us LOW) */
	stm32_gpiowrite(WS2812B_PIN, false);
	usleep(60);
}

/****************************************************************************
 * Standard LED Functions
 ****************************************************************************/

__EXPORT void led_init(void)
{
	/* Initialize standard LEDs */
	for (size_t l = 0; l < (sizeof(g_ledmap) / sizeof(g_ledmap[0])); l++) {
		if (g_ledmap[l] != 0) {
			stm32_configgpio(g_ledmap[l]);
		}
	}
	
	/* Initialize WS2812B LED */
	stm32_configgpio(WS2812B_PIN);
	ws2812b_set_color(0, 0, 0); /* Start with LED off */
}

static void phy_set_led(int led, bool state)
{
	/* Drive Low to switch on (open drain, active low) */
	if (g_ledmap[led] != 0) {
		stm32_gpiowrite(g_ledmap[led], !state);
	}
}

static bool phy_get_led(int led)
{
	/* If Low it is on */
	if (g_ledmap[led] != 0) {
		return !stm32_gpioread(g_ledmap[led]);
	}

	return false;
}

__EXPORT void led_on(int led)
{
	phy_set_led(xlat(led), true);
}

__EXPORT void led_off(int led)
{
	phy_set_led(xlat(led), false);
}

__EXPORT void led_toggle(int led)
{
	phy_set_led(xlat(led), !phy_get_led(xlat(led)));
}

#ifdef CONFIG_ARCH_LEDS
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_autoled_initialize
 ****************************************************************************/

void board_autoled_initialize(void)
{
	led_init();
}

/****************************************************************************
 * Name: board_autoled_on
 ****************************************************************************/

void board_autoled_on(int led)
{
	if (!nuttx_owns_leds) {
		return;
	}

	switch (led) {
	default:
		break;

	case LED_HEAPALLOCATE:
		/* Blue on WS2812B */
		ws2812b_set_color(0, 0, 64);
		break;

	case LED_IRQSENABLED:
		/* Green on standard LED */
		phy_set_led(BOARD_LED_GREEN, true);
		ws2812b_set_color(0, 0, 0);
		break;

	case LED_STACKCREATED:
		/* Green + Cyan on WS2812B */
		phy_set_led(BOARD_LED_GREEN, true);
		ws2812b_set_color(0, 64, 64);
		break;

	case LED_INIRQ:
		/* Blue pulse on WS2812B */
		ws2812b_set_color(0, 0, 32);
		break;

	case LED_SIGNAL:
		/* Green pulse on WS2812B */
		ws2812b_set_color(0, 32, 0);
		break;

	case LED_ASSERTION:
		/* Red on both */
		phy_set_led(BOARD_LED_RED, true);
		ws2812b_set_color(128, 0, 0);
		break;

	case LED_PANIC:
		/* Bright red on both */
		phy_set_led(BOARD_LED_RED, true);
		ws2812b_set_color(255, 0, 0);
		break;

	case LED_IDLE:
		/* Dim red */
		phy_set_led(BOARD_LED_RED, true);
		ws2812b_set_color(16, 0, 0);
		break;
	}
}

/****************************************************************************
 * Name: board_autoled_off
 ****************************************************************************/

void board_autoled_off(int led)
{
	if (!nuttx_owns_leds) {
		return;
	}

	switch (led) {
	default:
		break;

	case LED_SIGNAL:
		phy_set_led(BOARD_LED_GREEN, false);
		ws2812b_set_color(0, 0, 0);
		break;

	case LED_INIRQ:
		ws2812b_set_color(0, 0, 0);
		break;

	case LED_ASSERTION:
		phy_set_led(BOARD_LED_RED, false);
		ws2812b_set_color(0, 0, 0);
		break;

	case LED_PANIC:
		phy_set_led(BOARD_LED_RED, false);
		ws2812b_set_color(0, 0, 0);
		break;

	case LED_IDLE:
		phy_set_led(BOARD_LED_RED, false);
		ws2812b_set_color(0, 0, 0);
		break;
	}
}

#endif /* CONFIG_ARCH_LEDS */