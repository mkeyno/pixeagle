/****************************************************************************
 * boards/pixeagle/pixeagle/src/timer_config.cpp
 *
 * Pixeagle PWM timer configuration
 *
 ****************************************************************************/

#include <px4_arch/io_timer_hw_description.h>

/**
 * Pixeagle PWM Output Configuration (8 channels):
 * 
 * PWM 1: PD12 - TIM4_CH1
 * PWM 2: PD13 - TIM4_CH2
 * PWM 3: PD14 - TIM4_CH3
 * PWM 4: PD15 - TIM4_CH4
 * PWM 5: PC6  - TIM3_CH1 (also TIM8_CH1)
 * PWM 6: PC7  - TIM8_CH2
 * PWM 7: PC8  - TIM8_CH3
 * PWM 8: PA10 - TIM1_CH3
 * 
 * AUX GPIO (can be used as PWM if needed):
 * AUX 1: PA0  - TIM5_CH1
 * AUX 2: PA1  - TIM5_CH2
 * AUX 3: PA2  - TIM5_CH3
 * AUX 4: PA3  - TIM5_CH4
 * AUX 5: PB0  - TIM3_CH3
 * AUX 6: PE11 - TIM1_CH2
 * 
 * Note: PE14 is used for WS2812B LED (TIM1_CH4), not available for PWM
 */

constexpr io_timers_t io_timers[MAX_IO_TIMERS] = {
	initIOTimer(Timer::Timer1, DMA{DMA::Index1}),  // TIM1: PWM 8, (AUX 6)
	initIOTimer(Timer::Timer3, DMA{DMA::Index1}),  // TIM3: PWM 5, (AUX 5)
	initIOTimer(Timer::Timer4, DMA{DMA::Index1}),  // TIM4: PWM 1-4
	initIOTimer(Timer::Timer8, DMA{DMA::Index2}),  // TIM8: PWM 6-7
};

constexpr timer_io_channels_t timer_io_channels[MAX_TIMER_IO_CHANNELS] = {
	// Main PWM outputs 1-4 on TIM4
	initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel1}, {GPIO::PortD, GPIO::Pin12}), // PWM 1
	initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel2}, {GPIO::PortD, GPIO::Pin13}), // PWM 2
	initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel3}, {GPIO::PortD, GPIO::Pin14}), // PWM 3
	initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel4}, {GPIO::PortD, GPIO::Pin15}), // PWM 4
	
	// PWM 5 on TIM3_CH1
	initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel1}, {GPIO::PortC, GPIO::Pin6}),  // PWM 5
	
	// PWM 6-7 on TIM8
	initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel2}, {GPIO::PortC, GPIO::Pin7}),  // PWM 6
	initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel3}, {GPIO::PortC, GPIO::Pin8}),  // PWM 7
	
	// PWM 8 on TIM1_CH3
	initIOTimerChannel(io_timers, {Timer::Timer1, Timer::Channel3}, {GPIO::PortA, GPIO::Pin10}), // PWM 8
};

constexpr io_timers_channel_mapping_t io_timers_channel_mapping =
	initIOTimerChannelMapping(io_timers, timer_io_channels);