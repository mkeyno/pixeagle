/****************************************************************************
 * boards/pixeagle/pixeagle/src/tone_alarm_gpio.cpp
 *
 * Simple bit-banged GPIO tone alarm driver.
 ****************************************************************************/

#include <px4_platform_common/px4_config.h>
#include <px4_platform/gpio.h>
#include <drivers/drv_hrt.h>
#include <unistd.h>
#include <board_config.h> // For GPIO_TONE_ALARM_IDLE

/**
 * @brief Plays a simple tone by bit-banging the GPIO.
 * @param frequency The frequency of the tone in Hz.
 * @param duration_ms The duration of the tone in milliseconds.
 */
extern "C" void tone_alarm_gpio_beep(uint16_t frequency, uint16_t duration_ms)
{
    // The bootloader/main firmware share a macro for the tone pin (PE13)
    px4_arch_configgpio(GPIO_TONE_ALARM_IDLE);

    if (frequency == 0 || duration_ms == 0) {
        px4_arch_gpiowrite(GPIO_TONE_ALARM_IDLE, false);
        return;
    }

    // Calculate half period in microseconds
    // (1,000,000 us/s) / (frequency * 2 half cycles/cycle)
    int half_period_us = 1000000 / (2 * frequency);

    // Calculate end time
    uint64_t end_time = hrt_absolute_time() + (uint64_t)duration_ms * 1000;

    // Toggle the pin for the duration
    while (hrt_absolute_time() < end_time) {
        // Toggle HIGH (Assuming active-high for tone/buzzer, which might be opposite of an active-low LED)
        px4_arch_gpiowrite(GPIO_TONE_ALARM_IDLE, true);
        px4_usleep(half_period_us);

        // Toggle LOW
        px4_arch_gpiowrite(GPIO_TONE_ALARM_IDLE, false);
        px4_usleep(half_period_us);
    }

    // Ensure it ends LOW (off)
    px4_arch_gpiowrite(GPIO_TONE_ALARM_IDLE, false);
}

// Keep the main function for command line testing
extern "C" __EXPORT int tone_alarm_gpio_main(int argc, char *argv[])
{
    // Simple test beep
    tone_alarm_gpio_beep(2000, 200);
    return 0;
}