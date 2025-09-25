/**
 * @file board_config.h
 *
 * Pixeagle internal definitions - CORRECTED VERSION
 *
 * Hardware configuration for Pixeagle board (STM32H743VIT6 MCU, version V1C00)
 */

#pragma once

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>
#include <stm32_gpio.h>

/****************************************************************************************************
 * Definitions
 ****************************************************************************************************/

/* No PX4IO (FMU-only design) */
#define BOARD_USES_PX4IO_VERSION       0

/* Single hardware version for Pixeagle */
#define BOARD_NUM_SPI_CFG_HW_VERSIONS 1
#define PIX00   HW_VER_REV(0x1,0x0)

/* Power control - CORRECTED: PA15 active high for sensor power enable */
#define GPIO_VDD_5V_PERIPH_EN  (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN15)
#define GPIO_VDD_5V_PERIPH_nEN  GPIO_VDD_5V_PERIPH_EN  /* Alias for compatibility */

/* LEDs and Safety */
#define GPIO_WS2812B_LED     (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN14)
#define GPIO_SAFETY_LED      (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN15)
#define GPIO_BTN_SAFETY      (GPIO_INPUT|GPIO_PULLUP|GPIO_PORTE|GPIO_PIN12)
#define GPIO_TONE_ALARM_IDLE (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN13)

/* Status indicators */
#define BOARD_HAS_CONTROL_STATUS_LEDS  1
#define BOARD_OVERLOAD_LED             LED_RED
#define BOARD_ARMED_STATE_LED          LED_BLUE

/* ADC - CORRECTED: Use STM32H7 naming convention */
#define PX4_ADC_GPIO  \
    /* PC0 */  GPIO_ADC1_IN10,  /* Analog sensor 1 */ \
    /* PC1 */  GPIO_ADC1_IN11,  /* Analog sensor 2 */ \
    /* PC2 */  GPIO_ADC1_IN12,  /* Battery voltage */ \
    /* PC3 */  GPIO_ADC1_IN13   /* Battery current */

/* ADC Channel definitions - must match board.h */
#define ADC_ANALOG_SENSOR1_CHANNEL     10
#define ADC_ANALOG_SENSOR2_CHANNEL     11  
#define ADC_BATTERY_VOLTAGE_CHANNEL    12
#define ADC_BATTERY_CURRENT_CHANNEL    13

/* SPI Bus definitions - must match board.h */
#define PX4_SPI_BUS_SENSORS      1
#define PX4_SPI_BUS_RAMTRON      2  
#define PX4_SPI_BUS_EXT          3
#define PX4_SPI_BUS_ICM42688P    4

/* SPI Device definitions */
#define PX4_SPIDEV_BMI088_ACC          PX4_MK_SPI_SEL(PX4_SPI_BUS_SENSORS,0)
#define PX4_SPIDEV_BMI088_GYR          PX4_MK_SPI_SEL(PX4_SPI_BUS_SENSORS,1)
#define PX4_SPIDEV_FRAM                PX4_MK_SPI_SEL(PX4_SPI_BUS_RAMTRON,0)
#define PX4_SPIDEV_MICROSD             PX4_MK_SPI_SEL(PX4_SPI_BUS_RAMTRON,1)
#define PX4_SPIDEV_EXT0                PX4_MK_SPI_SEL(PX4_SPI_BUS_EXT,0)
#define PX4_SPIDEV_ICM42688P           PX4_MK_SPI_SEL(PX4_SPI_BUS_ICM42688P,0)

/* I2C Bus definitions - must match board.h */
#define PX4_I2C_BUS_EXT                1
#define PX4_I2C_BUS_IST8310_BMP388     3
#define PX4_I2C_BUS_BMP390             4

/* I2C Device addresses */
#define PX4_I2C_OBDEV_IST8310          0x0e
#define PX4_I2C_OBDEV_BMP388           0x77
#define PX4_I2C_OBDEV_BMP390           0x77

/* PWM Configuration */
#define DIRECT_PWM_OUTPUT_CHANNELS     8
#define DIRECT_INPUT_TIMER_CHANNELS    8

/* GPIO List - SIMPLIFIED to avoid conflicts with board.h */
#define PX4_GPIO_INIT_LIST { \
    PX4_ADC_GPIO, \
    GPIO_VDD_5V_PERIPH_EN, \
    GPIO_WS2812B_LED, \
    GPIO_SAFETY_LED, \
    GPIO_BTN_SAFETY, \
    GPIO_TONE_ALARM_IDLE \
}

/* Power Control Macros */
#define VDD_5V_PERIPH_EN(on_true)      px4_arch_gpiowrite(GPIO_VDD_5V_PERIPH_EN, (on_true))
#define VDD_5V_HIPOWER_EN(on_true)     VDD_5V_PERIPH_EN(on_true)
#define SPEKTRUM_POWER(on_true)        VDD_5V_PERIPH_EN(on_true)

/* Safety LED Control */
#define BOARD_SAFETY_LED(on_true)      px4_arch_gpiowrite(GPIO_SAFETY_LED, !(on_true))

/* Battery Configuration */
#define BOARD_BATTERY1_V_DIV           (11.0f)  /* 11:1 voltage divider */
#define BOARD_BATTERY1_A_PER_V         (17.0f)  /* Current sensor scaling */

/* Board identification */
#define BOARD_TYPE                     100      /* Must match hw_config.h and .prototype */
#define BOARD_FLASH_SECTORS            16
#define BOARD_FLASH_SIZE               (2 * 1024 * 1024)

/* Console and features */
#define BOARD_ENABLE_CONSOLE_BUFFER
#define BOARD_HAS_ON_RESET             1

__BEGIN_DECLS

/* Function declarations */
extern void stm32_spiinitialize(void);
extern void stm32_usbinitialize(void);
extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

__END_DECLS