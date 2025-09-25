/**
 * @file timer_config.cpp
 *
 * Pixeagle timer configuration
 *
 * Configures timers for PWM outputs and AUX GPIOs on the Pixeagle board (STM32H743VIT6, version V1C00):
 * - PWM OUT 1: PD12 (TIM4_CH1)
 * - PWM OUT 2: PD13 (TIM4_CH2)
 * - PWM OUT 3: PD14 (TIM4_CH3)
 * - PWM OUT 4: PD15 (TIM4_CH4)
 * - PWM OUT 5: PC6 (TIM3_CH1)
 * - PWM OUT 6: PC7 (TIM8_CH2)
 * - PWM OUT 7: PC8 (TIM8_CH3)  // UPDATED: Timer to TIM8_CH3
 * - PWM OUT 8: PA10 (TIM1_CH3)
 * - AUX GPIO 6: PE11 (TIM1_CH2)
 * - AUX GPIO 5: PB0 (TIM3_CH3)
 * - AUX GPIO 1: PA0 (TIM5_CH1)
 * - AUX GPIO 2: PA1 (TIM5_CH2)
 * - AUX GPIO 3: PA2 (TIM5_CH3)
 * - AUX GPIO 4: PA3 (TIM5_CH4)
 * - WS2812B LEDs: PE14 (TIM1)
 * - PPM Input: PB1 (TIM3_CH4)  // UPDATED: Added for PPM capture
 * Other hardware:
 * - BMI088 (SPI1, PA4 CS_ACC, PB2 CS_GYR, PA5–PA7, 20 MHz)  // UPDATED: GYR CS to PB2
 * - ICM-42688-P (SPI4, PE4 CS, PE2/PE5/PE6, 20 MHz)
 * - IST8310 (I2C3, PA8 SCL/PC9 SDA, 0x0E, 400 kHz), BMP388 (I2C3, 0x76), BMP390 (I2C4, PB8 SCL/PB9 SDA, 0x76)
 * - MicroSD (SPI2, PB11 CS, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz)
 * - FM25V01A-GTR (SPI2, PD10 CS, 20 MHz)
 * - External SPI (SPI3, PB3 SCK, PB4 MISO, PB5 MOSI, PD7 CS, 10 MHz, 5V via TXS0108ERGYR)
 * - External I2C (I2C1, PB6 SCL, PB7 SDA, 400 kHz, 5V via TXS0108ERGYR, user-selected via QGroundControl)
 * - UART4 (PC10/PC11, debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5 (PC12/PD2, sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V, /dev/ttyS0, 57600 baud)
 * - UART7 (PE7/PE8, CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - UART3 (PD8/PD9, SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent)
 * - CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1)
 * - USB OTG FS (PA9 VBUS, PA11 DM, PA12 DP, no power/overcurrent)
 * No PX4IO co-processor. I2C1 and SPI3 use TXS0108ERGYR for 5V level translation.
 */

#include <px4_arch/io_timer_hw_description.h>

/* Timer allocation
 * TIM1_CH2: AUX GPIO 6 (PE11)
 * TIM1_CH3: PWM OUT 8 (PA10)
 * TIM1_CH4: WS2812B LEDs (PE14)
 * TIM3_CH1: PWM OUT 5 (PC6)
 * TIM3_CH3: AUX GPIO 5 (PB0)
 * TIM3_CH4: PPM Input (PB1)  // UPDATED
 * TIM4_CH1: PWM OUT 1 (PD12)
 * TIM4_CH2: PWM OUT 2 (PD13)
 * TIM4_CH3: PWM OUT 3 (PD14)
 * TIM4_CH4: PWM OUT 4 (PD15)
 * TIM5_CH1: AUX GPIO 1 (PA0)
 * TIM5_CH2: AUX GPIO 2 (PA1)
 * TIM5_CH3: AUX GPIO 3 (PA2)
 * TIM5_CH4: AUX GPIO 4 (PA3)
 * TIM8_CH2: PWM OUT 6 (PC7)
 * TIM8_CH3: PWM OUT 7 (PC8)  // UPDATED
 */

constexpr io_timers_t 		io_timers[MAX_IO_TIMERS] = {
    initIOTimer(Timer::Timer1, DMA{DMA::Index1}), // TIM1 for PWM OUT 8, AUX GPIO 6, LEDs
    initIOTimer(Timer::Timer3, DMA{DMA::Index1}), // TIM3 for PWM OUT 5, AUX GPIO 5, PPM
    initIOTimer(Timer::Timer4, DMA{DMA::Index1}), // TIM4 for PWM OUT 1–4
    initIOTimer(Timer::Timer5),                   // TIM5 for AUX GPIO 1–4
    initIOTimer(Timer::Timer8, DMA{DMA::Index1}), // TIM8 for PWM OUT 6–7  // UPDATED: Added CH3 support
};

constexpr timer_io_channels_t 			timer_io_channels[MAX_TIMER_IO_CHANNELS] = {
    initIOTimerChannel(io_timers, {Timer::Timer1, Timer::Channel3}, {GPIO::PortA, GPIO::Pin10}), // PWM OUT 8
    initIOTimerChannel(io_timers, {Timer::Timer1, Timer::Channel2}, {GPIO::PortE, GPIO::Pin11}), // AUX GPIO 6
    initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel1}, {GPIO::PortC, GPIO::Pin6}),  // PWM OUT 5
    initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel3}, {GPIO::PortB, GPIO::Pin0}),  // AUX GPIO 5
    initIOTimerChannel(io_timers, {Timer::Timer3, Timer::Channel4}, {GPIO::PortB, GPIO::Pin1}),  // PPM Input (UPDATED: Added as input channel)
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel1}, {GPIO::PortD, GPIO::Pin12}), // PWM OUT 1
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel2}, {GPIO::PortD, GPIO::Pin13}), // PWM OUT 2
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel3}, {GPIO::PortD, GPIO::Pin14}), // PWM OUT 3
    initIOTimerChannel(io_timers, {Timer::Timer4, Timer::Channel4}, {GPIO::PortD, GPIO::Pin15}), // PWM OUT 4
    initIOTimerChannel(io_timers, {Timer::Timer5, Timer::Channel1}, {GPIO::PortA, GPIO::Pin0}),  // AUX GPIO 1
    initIOTimerChannel(io_timers, {Timer::Timer5, Timer::Channel2}, {GPIO::PortA, GPIO::Pin1}),  // AUX GPIO 2
    initIOTimerChannel(io_timers, {Timer::Timer5, Timer::Channel3}, {GPIO::PortA, GPIO::Pin2}),  // AUX GPIO 3
    initIOTimerChannel(io_timers, {Timer::Timer5, Timer::Channel4}, {GPIO::PortA, GPIO::Pin3}),  // AUX GPIO 4
    initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel2}, {GPIO::PortC, GPIO::Pin7}),  // PWM OUT 6
    initIOTimerChannel(io_timers, {Timer::Timer8, Timer::Channel3}, {GPIO::PortC, GPIO::Pin8}),  // PWM OUT 7 (UPDATED: Timer/channel)
};

constexpr io_timers_channel_mapping_t 		io_timers_channel_mapping = initIOTimerChannelMapping(io_timers, timer_io_channels);