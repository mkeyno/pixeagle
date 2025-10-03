#include <px4_arch/io_timer_hw_description.h>

/* Timer allocation - 8 PWM outputs only
 *
 * TIM3_CH1: PWM OUT 5 (PC6)
 * TIM4_CH1: PWM OUT 1 (PD12)
 * TIM4_CH2: PWM OUT 2 (PD13)
 * TIM4_CH3: PWM OUT 3 (PD14)
 * TIM4_CH4: PWM OUT 4 (PD15)
 * TIM8_CH2: PWM OUT 6 (PC7)
 * TIM8_CH3: PWM OUT 7 (PC8)
 * TIM1_CH3: PWM OUT 8 (PA10)
 */

constexpr io_timers_t io_timers[MAX_IO_TIMERS] = {
    initIOTimer(Timer::Timer1, DMA{DMA::Index1}), // TIM1
    initIOTimer(Timer::Timer3, DMA{DMA::Index1}), // TIM3
    initIOTimer(Timer::Timer4, DMA{DMA::Index1}), // TIM4
    initIOTimer(Timer::Timer8, DMA{DMA::Index1}), // TIM8
};

constexpr timer_io_channels_t timer_io_channels[MAX_TIMER_IO_CHANNELS] = {
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel1}, {GPIO::PortD, GPIO::Pin12}), // PWM OUT 1
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel2}, {GPIO::PortD, GPIO::Pin13}), // PWM OUT 2
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel3}, {GPIO::PortD, GPIO::Pin14}), // PWM OUT 3
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel4}, {GPIO::PortD, GPIO::Pin15}), // PWM OUT 4
    initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel1}, {GPIO::PortC, GPIO::Pin6}),  // PWM OUT 5
    initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel2}, {GPIO::PortC, GPIO::Pin7}),  // PWM OUT 6
    initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel3}, {GPIO::PortC, GPIO::Pin8}),  // PWM OUT 7
    initIOTimerChannel(io_timers, {Timer::Timer1, Timer::Channel3}, {GPIO::PortA, GPIO::Pin10}), // PWM OUT 8
};

constexpr io_timers_channel_mapping_t io_timers_channel_mapping =
    initIOTimerChannelMapping(io_timers, timer_io_channels);