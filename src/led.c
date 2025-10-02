/**
 * @file led.cpp
 * Pixeagle WS2812B LED driver with bitbang implementation
 */

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/defines.h>
#include <drivers/drv_hrt.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <syslog.h>

#include "chip.h"
#include "stm32_gpio.h"
#include "stm32_tim.h"
#include "board_config.h"

#include <nuttx/board.h>
#include <arch/board/board.h>

/* WS2812B LED configuration */
#define NUM_LEDS         2
#define BRIGHTNESS       50

/* WS2812B timing in nanoseconds (approximate for STM32H7 at 480MHz) */
#define T0H_NS          350  // 0 code high time
#define T1H_NS          700  // 1 code high time  
#define T0L_NS          800  // 0 code low time
#define T1L_NS          600  // 1 code low time
#define RESET_US        50   // Reset time in microseconds

/* LED color structure */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_color_t;

/* LED status patterns */
typedef enum {
    LED_STATE_INIT,
    LED_STATE_DISARMED,
    LED_STATE_ARMED,
    LED_STATE_LOW_BAT,
    LED_STATE_ERROR,
    LED_STATE_PANIC,
    LED_STATE_GPS_LOCK,
    LED_STATE_CAN_ACTIVE,
} led_state_t;

/* Global variables */
static led_color_t leds[NUM_LEDS];
static led_state_t current_state = LED_STATE_INIT;
static uint32_t last_update_ms = 0;
static bool leds_initialized = false;

#define LED_UPDATE_INTERVAL_US 50000

__BEGIN_DECLS
extern void led_init(void);
extern void led_on(int led);
extern void led_off(int led);
extern void led_toggle(int led);
__END_DECLS

#ifdef CONFIG_ARCH_LEDS
static bool nuttx_owns_leds = true;

static const led_state_t led_state_map[] = {
    LED_STATE_INIT,
    LED_STATE_INIT,
    LED_STATE_DISARMED,
    LED_STATE_INIT,
    LED_STATE_INIT,
    LED_STATE_ERROR,
    LED_STATE_PANIC,
    LED_STATE_GPS_LOCK,
    LED_STATE_CAN_ACTIVE,
};
#endif

/* Simple delay loop for precise timing */
static void delay_ns(uint32_t ns)
{
    /* STM32H7 at 480MHz = ~2.08ns per cycle */
    volatile uint32_t cycles = ns / 2;
    while (cycles--) {
        __asm__("nop");
    }
}

/* Set WS2812B data pin state */
static void ws2812b_set_pin(bool state)
{
    if (state) {
        stm32_gpiowrite(GPIO_WS2812B_LED, true);
    } else {
        stm32_gpiowrite(GPIO_WS2812B_LED, false);
    }
}

/* Send a single byte to WS2812B */
static void ws2812b_send_byte(uint8_t byte)
{
    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i)) {
            // Send '1' bit
            ws2812b_set_pin(true);
            delay_ns(T1H_NS);
            ws2812b_set_pin(false);
            delay_ns(T1L_NS);
        } else {
            // Send '0' bit
            ws2812b_set_pin(true);
            delay_ns(T0H_NS);
            ws2812b_set_pin(false);
            delay_ns(T0L_NS);
        }
    }
}

/* Send reset signal */
static void ws2812b_reset(void)
{
    ws2812b_set_pin(false);
    up_udelay(RESET_US);
}

/* Update all LEDs with current colors */
static void ws2812b_update(void)
{
    // Disable interrupts for precise timing
    irqstate_t flags = enter_critical_section();
    
    for (int i = 0; i < NUM_LEDS; i++) {
        // WS2812B expects GRB order
        ws2812b_send_byte(leds[i].green);
        ws2812b_send_byte(leds[i].red);
        ws2812b_send_byte(leds[i].blue);
    }
    
    ws2812b_reset();
    
    // Restore interrupts
    leave_critical_section(flags);
}

/* Set LED color with brightness adjustment */
static void set_led_color(uint8_t led_index, uint8_t red, uint8_t green, uint8_t blue)
{
    if (led_index >= NUM_LEDS) return;
    
    // Apply brightness (simple scaling)
    leds[led_index].red = (red * BRIGHTNESS) / 100;
    leds[led_index].green = (green * BRIGHTNESS) / 100;
    leds[led_index].blue = (blue * BRIGHTNESS) / 100;
}

/* Set all LEDs to same color */
static void set_all_leds(uint8_t red, uint8_t green, uint8_t blue)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_color(i, red, green, blue);
    }
}

static uint32_t now_ms(void)
{
    return hrt_absolute_time() / 1000;
}

static void led_update_pattern(void)
{
    uint32_t now_ms_val = now_ms();
    if (now_ms_val - last_update_ms < 50) {
        return;  // Throttle updates
    }

    switch (current_state) {
    case LED_STATE_INIT:
        set_all_leds(0, 0, 255);  // Blue
        break;

    case LED_STATE_DISARMED:
        set_led_color(0, 0, 255, 0);  // Green
        set_led_color(1, 0, 0, 0);    // Off
        break;

    case LED_STATE_ARMED:
        set_led_color(0, 255, 0, 0);  // Red
        set_led_color(1, 0, 255, 0);  // Green
        break;

    case LED_STATE_LOW_BAT:
        set_led_color(0, 255, 255, 0);  // Yellow
        set_led_color(1, 0, 0, 0);      // Off
        break;

    case LED_STATE_ERROR:
        set_all_leds(255, 0, 0);  // Red
        break;

    case LED_STATE_GPS_LOCK:
        set_all_leds(0, 255, 0);  // Green
        break;

    case LED_STATE_CAN_ACTIVE:
        set_led_color(0, 0, 255, 255);  // Cyan
        set_led_color(1, 0, 0, 0);      // Off
        break;

    default:
        set_all_leds(0, 0, 0);  // Off
        break;
    }

    ws2812b_update();
    last_update_ms = now_ms_val;
}

static void led_update_callback(void *arg)
{
    led_update_pattern();
    hrt_call_after((struct hrt_call *)arg, LED_UPDATE_INTERVAL_US, led_update_callback, arg);
}

__EXPORT void led_init(void)
{
    if (!leds_initialized) {
        // Configure LED pin
        stm32_configgpio(GPIO_WS2812B_LED);
        
        // Initialize all LEDs to off
        memset(leds, 0, sizeof(leds));
        ws2812b_update();

        // Start update timer
        static struct hrt_call led_call;
        hrt_call_after(&led_call, LED_UPDATE_INTERVAL_US, led_update_callback, &led_call);

        leds_initialized = true;
        syslog(LOG_INFO, "WS2812B LEDs initialized on PE14\n");
    }
}

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

__EXPORT void led_off(int led)
{
    set_all_leds(0, 0, 0);
    ws2812b_update();
}

__EXPORT void led_toggle(int led)
{
    current_state = LED_STATE_INIT;
    led_update_pattern();
}

#ifdef CONFIG_ARCH_LEDS
void board_autoled_initialize(void)
{
    led_init();
}

void board_autoled_on(int led)
{
    if (!nuttx_owns_leds) {
        return;
    }
    led_on(led);
}

void board_autoled_off(int led)
{
    if (!nuttx_owns_leds) {
        return;
    }
    led_off(led);
}
#endif
