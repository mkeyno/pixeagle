/**
 * @file led.c
 *
 * Pixeagle WS2812B LED backend
 *
 * This file controls two WS2812B LEDs on PE14 for the Pixeagle board, based on
 * STM32H743VIT6, with BMI088 (SPI1, PA4 CS_ACC, PB2 CS_GYR, PA5–PA7, 20 MHz), // UPDATED: GYR CS to PB2
 * ICM-42688-P (SPI4, PE4 CS, PE2/PE5/PE6, PE3 DRDY, 20 MHz),
 * IST8310 (I2C3, PC9/PA8, 0x0E), BMP388 (I2C3, 0x76), BMP390 (I2C4, PB8/PB9, 0x76),
 * FM25V01A-GTR (SPI2, PD10 CS), MicroSD (SPI2, PB11 CS),
 * UART4 (PC10/PC11, debug, 5V), UART5 (PC12/PD2, sensor module, 5V),
 * USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V), // UPDATED: USART2 pins to PD3-PD6
 * UART7 (PE7/PE8, CM4/ESP32, 5V),
 * CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1). // UPDATED: CAN1 pins to PD0/PD1
 * Uses FastLED library for WS2812B control via TIM1 on PE14, with custom patterns
 * for drone statuses (init, armed, disarmed, low battery, error, panic, GPS lock, CAN active).
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/defines.h>
#include <drivers/drv_hrt.h>
#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>

#include "chip.h"
#include "stm32_gpio.h"
#include "stm32_tim.h"
#include "board_config.h"

#include <nuttx/board.h>
#include <arch/board/board.h>

// Include FastLED library for WS2812B control
#include <FastLED.h>

/* WS2812B LED configuration */
#define NUM_LEDS         2                 // Two WS2812B LEDs
#define LED_TYPE         WS2812B
#define COLOR_ORDER      GRB
#define BRIGHTNESS       50                // 0-255, adjust for visibility

/* Define LED status patterns */
typedef enum {
    LED_STATE_INIT,        // Booting/initializing
    LED_STATE_DISARMED,    // Disarmed, ready
    LED_STATE_ARMED,       // Armed, ready to fly
    LED_STATE_LOW_BAT,     // Low battery warning
    LED_STATE_ERROR,       // Sensor or system error
    LED_STATE_PANIC,       // Critical failure
    LED_STATE_GPS_LOCK,    // GPS lock acquired
    LED_STATE_CAN_ACTIVE,  // CAN connection active
} led_state_t;

/* FastLED global variables */
static CRGB leds[NUM_LEDS];
static led_state_t current_state = LED_STATE_INIT;
static uint32_t last_update_ms = 0;
static bool leds_initialized = false;

/* Timer for LED updates (50ms interval) */
#define LED_UPDATE_INTERVAL_US 50000

/* Function declarations */
__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
extern void led_toggle(int led);
__END_DECLS

#ifdef CONFIG_ARCH_LEDS
static bool nuttx_owns_leds = true;

/* Map PX4 LED indices to WS2812B states */
static const led_state_t led_state_map[] = {
    LED_STATE_INIT,         // LED_HEAPALLOCATE
    LED_STATE_INIT,         // LED_IRQSENABLED
    LED_STATE_DISARMED,     // LED_STACKCREATED
    LED_STATE_INIT,         // LED_INIRQ
    LED_STATE_INIT,         // LED_SIGNAL
    LED_STATE_ERROR,        // LED_ASSERTION
    LED_STATE_PANIC,        // LED_PANIC
    LED_STATE_DISARMED,     // LED_IDLE
};

#endif

/* Update LED patterns based on state */
static void led_update_pattern(void)
{
    uint32_t now_ms = hrt_absolute_time() / 1000;

    switch (current_state) {
    case LED_STATE_INIT:
        // Slow blue blink (1 Hz) on both LEDs
        if ((now_ms / 500) % 2) {
            leds[0] = CRGB(0, 0, 255);
            leds[1] = CRGB(0, 0, 255);
        } else {
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
        }
        break;

    case LED_STATE_DISARMED:
        // LED0 solid green, LED1 off
        leds[0] = CRGB(0, 255, 0);
        leds[1] = CRGB::Black;
        break;

    case LED_STATE_ARMED:
        // Both LEDs solid green
        leds[0] = CRGB(0, 255, 0);
        leds[1] = CRGB(0, 255, 0);
        break;

    case LED_STATE_LOW_BAT:
        // Both LEDs blinking orange (2 Hz)
        if ((now_ms / 250) % 2) {
            leds[0] = CRGB(255, 165, 0);
            leds[1] = CRGB(255, 165, 0);
        } else {
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
        }
        break;

    case LED_STATE_ERROR:
        // Both LEDs fast blinking red (4 Hz)
        if ((now_ms / 125) % 2) {
            leds[0] = CRGB(255, 0, 0);
            leds[1] = CRGB(255, 0, 0);
        } else {
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
        }
        break;

    case LED_STATE_PANIC:
        // Both LEDs solid red
        leds[0] = CRGB(255, 0, 0);
        leds[1] = CRGB(255, 0, 0);
        break;

    case LED_STATE_GPS_LOCK:
        // LED0 solid green, LED1 pulsing cyan (0.5 Hz)
        leds[0] = CRGB(0, 255, 0);
        if ((now_ms / 1000) % 2) {
            leds[1] = CRGB(0, 255, 255);
        } else {
            leds[1] = CRGB::Black;
        }
        break;

    case LED_STATE_CAN_ACTIVE:
        // LED0 solid green, LED1 slow blinking purple (1 Hz)
        leds[0] = CRGB(0, 255, 0);
        if ((now_ms / 500) % 2) {
            leds[1] = CRGB(128, 0, 128);
        } else {
            leds[1] = CRGB::Black;
        }
        break;
    }

    FastLED.show();
    last_update_ms = now_ms;
}

/* HRT callback for periodic LED updates */
static void led_update_callback(void *arg)
{
    led_update_pattern();
    hrt_call_after((struct hrt_call *)arg, LED_UPDATE_INTERVAL_US, led_update_callback, arg);
}

/* Initialize WS2812B LEDs with FastLED */
__EXPORT void led_init(void)
{
    if (!leds_initialized) {
        /* Configure PE14 as output for WS2812B data */
#ifndef GPIO_WS2812B_LED
#define GPIO_WS2812B_LED (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | GPIO_PORTE | GPIO_PIN14) // PE14 for WS2812B
#endif
        stm32_configgpio(GPIO_WS2812B_LED);

        /* Initialize FastLED with the correct pin mapping for STM32 */
        FastLED.addLeds<WS2812B, GPIOE, 14, GRB>(leds, NUM_LEDS);
        FastLED.setBrightness(BRIGHTNESS);

        /* Set initial state to off */
        leds[0] = CRGB::Black;
        leds[1] = CRGB::Black;
        FastLED.show();

        /* Schedule periodic updates */
        static struct hrt_call led_call;
        hrt_call_after(&led_call, LED_UPDATE_INTERVAL_US, led_update_callback, &led_call);

        leds_initialized = true;
        syslog(LOG_INFO, "WS2812B LEDs initialized on PE14\n");
    }
}

/* Set LED state (maps to drone status) */
__EXPORT void led_on(int led)
{
#ifdef CONFIG_ARCH_LEDS
    if (nuttx_owns_leds) {
        current_state = led_state_map[led];
    } else {
        current_state = (led_state_t)led;
    }
#else
    current_state = (led_state_t)led;
#endif
    led_update_pattern();
}

/* Turn off LEDs */
__EXPORT void led_off(int led)
{
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    FastLED.show();
}

/* Toggle LEDs (not used for WS2812B, fallback to init state) */
__EXPORT void led_toggle(int led)
{
    current_state = LED_STATE_INIT;
    led_update_pattern();
}

#ifdef CONFIG_ARCH_LEDS
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
    led_on(led);
}

/****************************************************************************
 * Name: board_autoled_off
 ****************************************************************************/

void board_autoled_off(int led)
{
    if (!nuttx_owns_leds) {
        return;
    }
    led_off(led);
}

#endif /* CONFIG_ARCH_LEDS */