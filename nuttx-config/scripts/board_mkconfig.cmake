# board_mkconfig.cmake
#
# Build configuration for Pixeagle (STM32H743VIT6, 16MHz HSE, 480MHz core, 2MB flash).
# Hardware:
# - SPI1 (BMI088: PA4 CS_ACC, PB2 CS_GYR, PA5-7, 20 MHz, PC4/PC5 interrupts)
# - SPI2 (MicroSD: PB11 CS, FM25V01A-GTR: PD10 CS, PB10/14/15, 10/20 MHz)
# - SPI3 (External: PB3-5, PD7 CS, 10 MHz, 5V via TXS0108ERGYR)
# - SPI4 (ICM-42688-P: PE4 CS, PE2/5/6, PE3 DRDY, 20 MHz)
# - I2C1 (External: PB6/7, 400 kHz, 5V), I2C3 (IST8310: 0x0E, BMP388: 0x76, PC9/PA8)
# - I2C4 (BMP390: 0x76, PB8/9, 400 kHz)
# - UART3 (/dev/ttyS1, PD8/9, SBUS/PPM), UART4 (/dev/ttyS3, PC10/11, debug)
# - UART5 (/dev/ttyS2, PC12/PD2, sensors), USART2 (/dev/ttyS0, PD3-6, telemetry)
# - UART7 (/dev/ttyS4, PE7-10, CM4), UART8 (/dev/ttyS5, PE0/1, reserved)
# - CAN1 (PD0/1, AF9), CAN2 (PB12/13), both 5V via TCAN1044VDRQ1
# - TIM1 (PWM OUT 8: PA10, AUX6: PE11, WS2812B: PE14 via FastLED)
# - TIM3 (PWM OUT 5: PC6, AUX5: PB0, PPM: PB1), TIM4 (PWM OUT 1-4: PD12-15)
# - TIM5 (AUX 1-4: PA0-3), TIM8 (PWM OUT 6: PC7, OUT 7: PC8)
# - ADC (PC0-3, 11:1 divider for 5S LiPo)
# - Sensor power: PA15 (GPIO_VDD_5V_PERIPH_EN, active high)
# No PX4IO. FastLED linked in src/CMakeLists.txt.

set(PX4_BOARD pixeagle)
set(PX4_MCU STM32H743VI)
set(PX4_HSE_FREQ 16000000)
set(PX4_FLASH_SIZE 2048) # 2MB

# NuttX configurations
set(NUTTX_CONFIGS
  ${CMAKE_CURRENT_SOURCE_DIR}/nuttx-config/nsh/defconfig
  ${CMAKE_CURRENT_SOURCE_DIR}/nuttx-config/bootloader/defconfig
)

px4_nuttx_config(
  BOARD ${PX4_BOARD}
  MCU ${PX4_MCU}
  HSE_FREQ ${PX4_HSE_FREQ}
  FLASH_SIZE ${PX4_FLASH_SIZE}
  CONFIGS ${NUTTX_CONFIGS}
)