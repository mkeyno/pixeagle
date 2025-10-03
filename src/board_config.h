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

// Change this from 5 to 4 (only using TIM1, TIM3, TIM4, TIM8)
#define BOARD_NUM_IO_TIMERS 4

// Keep this as 8 for your 8 PWM outputs
#define BOARD_NUM_TIMER_IO_CHANNELS 8

// These should match
#define DIRECT_PWM_OUTPUT_CHANNELS  8
#define DIRECT_INPUT_TIMER_CHANNELS 8

 
/************************************************************************************
 * Board Control Definitions
 ************************************************************************************/
#define BOARDIOC_INIT      0x0000
#define BOARDIOC_RESET     0x0001  /* Reset to bootloader if arg=1, normal reset if arg=0 */
#define BOARDIOC_POWEROFF  0x0002
/****************************************************************************************************
 * Definitions
 ****************************************************************************************************/

/* No PX4IO (FMU-only design) */
#define BOARD_USES_PX4IO_VERSION       0

/* Single hardware version for Pixeagle */
#define BOARD_NUM_SPI_CFG_HW_VERSIONS 1
#define V1C00   HW_VER_REV(0x1,0x0)

/* Standard PX4 SPI selector macro */
#define PX4_MK_SPI_SEL(bus, dev) (((bus) << 8) | (dev))

/* ADC - Reference from board.h (no redefs) */
#define PX4_ADC_GPIO  \
    /* PC0 */  GPIO_ADC1_IN10,  /* Analog sensor 1 */ \
    /* PC1 */  GPIO_ADC1_IN11,  /* Analog sensor 2 */ \
    /* PC2 */  GPIO_ADC1_IN12,  /* Battery voltage */ \
    /* PC3 */  GPIO_ADC1_IN13   /* Battery current */

/* SPI Bus definitions */
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

#define SPIDEV_FRAM 0

#define BOARD_NUMBER_OF_PWM_DRIVERS 8  // Standard FMU PWM groups for metadata
/* SPI clock frequencies */
#define PX4_SPI_BUS_FREQ_SENSORS      20000000
#define PX4_SPI_BUS_FREQ_RAMTRON      20000000
#define PX4_SPI_BUS_FREQ_EXT          10000000
#define PX4_SPI_BUS_FREQ_ICM42688P    20000000

/* Sensor DRDY interrupts - THESE ARE WHAT CONTROL INTERRUPT-DRIVEN DATA */
#define GPIO_BMI088_ACC_INT1    (GPIO_INPUT|GPIO_PULLUP|GPIO_EXTI|GPIO_PORTC|GPIO_PIN4)
#define GPIO_BMI088_GYR_INT1    (GPIO_INPUT|GPIO_PULLUP|GPIO_EXTI|GPIO_PORTC|GPIO_PIN5)
#define GPIO_ICM42688P_DRDY     (GPIO_INPUT|GPIO_PULLUP|GPIO_EXTI|GPIO_PORTE|GPIO_PIN3)

/* Number of SPI bus hardware versions */
#define BOARD_NUM_SPI_CFG_HW_VERSIONS 1



/* I2C Bus definitions - PX4 abstraction layer */
#define PX4_I2C_BUS_EXT                1
#define PX4_I2C_BUS_IST8310_BMP388     3
#define PX4_I2C_BUS_BMP390             4

/* I2C Device addresses - PX4 abstraction layer */
#define PX4_I2C_OBDEV_IST8310          0x0e
#define PX4_I2C_OBDEV_BMP388           0x77
#define PX4_I2C_OBDEV_BMP390           0x77

/* GPIO List - Consolidated PX4 init list (uses board.h names directly) */
#define PX4_GPIO_INIT_LIST { \
    PX4_ADC_GPIO, \
    GPIO_VDD_5V_PERIPH_EN, \
    GPIO_WS2812B_LED, \
    GPIO_SAFETY_LED, \
    GPIO_SAFETY_SWITCH_IN, \
    GPIO_BUZZER, \
    GPIO_CAN1_TX, \
    GPIO_CAN1_RX, \
    GPIO_CAN2_TX, \
    GPIO_CAN2_RX, \
    GPIO_PPM_IN, \
    GPIO_CM4_STATUS, \
    GPIO_BMI088_ACC_INT1, \
    GPIO_BMI088_GYR_INT1, \
    GPIO_ICM42688P_DRDY \
}

/* Board identification - Use board.h values directly */
#define BOARD_TYPE                     100      /* Must match hw_config.h and .prototype */
#define BOARD_FLASH_SECTORS            16
#define BOARD_FLASH_SIZE               (2 * 1024 * 1024)

/* Console and features */
#define BOARD_ENABLE_CONSOLE_BUFFER
#define BOARD_HAS_ON_RESET             1

/* HW Rev and Ver detection */
#define BOARD_HAS_HW_SPLIT_VERSIONING

/* Dummy HW Revision Sensing - use PC13, PC14 */
#define GPIO_HW_REV_DRIVE     (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN13)
#define GPIO_HW_REV_SENSE     (GPIO_INPUT|GPIO_FLOAT|GPIO_PORTC|GPIO_PIN14)
#define ADC_HW_REV_SENSE_CHANNEL    ADC1_CH(10)  /* Use existing PC0 */

/* Dummy HW Version Sensing - use the same pins (it's just dummy detection) */
#define GPIO_HW_VER_DRIVE     GPIO_HW_REV_DRIVE   /* Same as revision drive */
#define GPIO_HW_VER_SENSE     GPIO_HW_REV_SENSE   /* Same as revision sense */
#define ADC_HW_VER_SENSE_CHANNEL    ADC_HW_REV_SENSE_CHANNEL  /* Same ADC channel */


#define HW_INFO_INIT_PREFIX           "PX4_"

#define HW_INFO_INIT \
        {HW_INFO_INIT_PREFIX"FMUV6C", "PIXEAGLE", "V1.0"},

__BEGIN_DECLS

/* Function declarations */
extern void stm32_spiinitialize(void);
extern void stm32_usbinitialize(void);
extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

__END_DECLS