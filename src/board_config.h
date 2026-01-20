#pragma once

#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

/****************************************************************************************************
Application/Driver Level
Configures the PX4 Autopilot. It gives meaningful names to GPIOs so drivers can find them
To define Chip Selects (CS), Data Ready (DRDY) pins, Battery scaling, and Bus assignments
 ****************************************************************************************************/

#include <px4_platform_common/px4_config.h>
#include <nuttx/compiler.h>
#include <stdint.h>
#include <stm32_gpio.h>

extern   uint32_t board_hw_version;
extern   uint32_t board_hw_revision;

/* Hardware version definition */
#define HW_VER_REV(v,r) ((((uint32_t)(v)) << 16) | ((uint32_t)(r)))
#define VER00 HW_VER_REV(0x0,0x0)






/****************************************************************************************************
 * Definitions
 ****************************************************************************************************/


/* LEDs */
#define BOARD_HAS_CONTROL_STATUS_LEDS      	1
#define BOARD_OVERLOAD_LED     				LED_RED
#define BOARD_ARMED_STATE_LED  				LED_GREEN


/* ADC channels */
#define PX4_ADC_GPIO  \
	/* PC0 */  GPIO_ADC123_INP10, \
	/* PC1 */  GPIO_ADC123_INP11, \
	/* PC2 */  GPIO_ADC123_INP12, \
	/* PC3 */  GPIO_ADC12_INP13

#define ADC_SCALED_V5_CHANNEL               /* PC0 */  10
#define ADC_SCALED_VDD_3V3_SENSORS_CHANNEL  /* PC1 */  11
#define ADC_BATTERY_VOLTAGE_CHANNEL         /* PC2 */  12
#define ADC_BATTERY_CURRENT_CHANNEL         /* PC3 */  13

#define ADC_CHANNELS \
	((1 << ADC_SCALED_V5_CHANNEL)              | \
	 (1 << ADC_SCALED_VDD_3V3_SENSORS_CHANNEL) | \
	 (1 << ADC_BATTERY_VOLTAGE_CHANNEL)        | \
	 (1 << ADC_BATTERY_CURRENT_CHANNEL))

#define BOARD_ADC_OPEN_CIRCUIT_V     (5.6f)

//#define FLASH_BASED_PARAMS
#define BOARD_USE_EXTERNAL_FLASH //Configuration and firmware are in external flash


/* WS2812B RGB LED - using bitbang on PE14  in next version change to PE13*/
#define GPIO_LED_WS2812B       (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_100MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN14)

/* Safety switch and alarm
PE13 timer1 channel3  can not use because of confiict with PA10 pwm output8
PE14 timer1 channel4
*/
#define GPIO_SAFETY_SWITCH_IN   /* PE12 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_PORTE|GPIO_PIN12)
#define GPIO_SAFETY_SWITCH_LED  /* PE15 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN15)
#define GPIO_ALARM_OUTPUT       /* PE13 will switch to PE14 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN13)


/* Define the safety button GPIO for the safety_button driver */
#define GPIO_BTN_SAFETY GPIO_SAFETY_SWITCH_IN
#define GPIO_LED_SAFETY GPIO_SAFETY_SWITCH_LED


/* Tone alarm output */
#define TONE_ALARM_TIMER        		1
#define TONE_ALARM_CHANNEL      		3 /*4*/
#define BOARD_HAS_TONE_ALARM_GPIO      	0 /*was 1*/
#define GPIO_TONE_ALARM_IDLE    		GPIO_ALARM_OUTPUT
#define GPIO_TONE_ALARM					GPIO_ALARM_OUTPUT

/* LED Configuration for Bootloader Status */
#define BOARD_PIN_LED_ACTIVITY         GPIO_SAFETY_SWITCH_LED  /* PE15 */
#define BOARD_PIN_LED_BOOTLOADER       GPIO_SAFETY_SWITCH_LED  /* PE15 */
#define BOARD_LED_ON                   0
#define BOARD_LED_OFF                  1

/* PWM - 8 channels total */
#define DIRECT_PWM_OUTPUT_CHANNELS  8
#define BOARD_NUM_IO_TIMERS 4

/* Power supply control */
#define GPIO_VDD_3V3_SENSORS_EN     /* PA15 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN15)
#define VDD_3V3_SENSORS_EN(on_true)    px4_arch_gpiowrite(GPIO_VDD_3V3_SENSORS_EN, (on_true))

/* USB OTG FS - PA9 VBUS */
/* NOTE: PA9 hardware definition kept for reference, but NOT used for sensing below */
#define GPIO_OTGFS_VBUS         /* PA9 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_SPEED_100MHz|GPIO_PORTA|GPIO_PIN9)

/* USB Connection Detection
 * * FORCED TO TRUE:
 * The hardware VBUS sensing on PA9 is disabled/unused.
 * We force the firmware to assume USB is always connected and valid.
 */
#define BOARD_ADC_USB_CONNECTED   (1) /* #define BOARD_ADC_USB_CONNECTED (px4_arch_gpioread(GPIO_OTGFS_VBUS)) */
#define BOARD_ADC_USB_VALID       (1)

/* High-resolution timer */
#define HRT_TIMER               2
#define HRT_TIMER_CHANNEL       1

/* PWM input driver */
#define PWMIN_TIMER                       3
#define PWMIN_TIMER_CHANNEL    /* T3C4 */ 4
#define GPIO_PWM_IN            /* PB1 */  GPIO_TIM3_CH4IN

/* RC Input - SBUS on USART3 */
//#define RC_SERIAL_PORT                    "/dev/ttyS2"

/* SDIO */
#define SDIO_SLOTNO                    0
#define SDIO_MINOR                     0

#if defined(CONFIG_BOARD_INITIALIZE) && !defined(CONFIG_BOARDCTL) && \
   !defined(CONFIG_BOARD_INITTHREAD)
#  warning SDIO initialization cannot be perfomed on the IDLE thread
#endif

#define BOARD_ADC_SERVO_VALID     (1)
#define BOARD_ADC_BRICK_VALID     (1)

/* Raspberry Pi CM4 status pin */
#define GPIO_RPI_CM4_STATUS     /* PD11 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_PORTD|GPIO_PIN11)

/* DMA pool */
#define BOARD_DMA_ALLOC_POOL_SIZE 16*1024  // Was 5120

/* FRAM Chip Select (PD10) is defined and initialized to High (disabled)*/
#define GPIO_SPI2_CS_FRAM (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTD|GPIO_PIN10)
/* --- SPI2: Shared Bus (SD Card + FRAM) --- */
/* CS Pins: PB11 (SD Card), PD10 (FRAM - Already defined in your file) */
#define GPIO_SPI2_CS_SDCARD      (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN11)


/* --- SPI1: BMI088 (Accel + Gyro) --- */
/* CS Pins: PA4 (Acc), PB2 (Gyr) */
/* PA4 = GYRO (ID 0x0F) PB2 = ACCEL (ID 0x1E) */
/* Accel is actually on PB2, Gyro is on PA4 */
#define GPIO_SPI1_CS_BMI088_GYR   (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTA|GPIO_PIN4)
#define GPIO_SPI1_CS_BMI088_ACC   (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTB|GPIO_PIN2)

/* DRDY Pins: PC4 (Acc), PC5 (Gyr) /////////////////////////////////////////////////////////////////*/
#define GPIO_SPI1_DRDY_BMI088_ACC (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTC|GPIO_PIN4)
#define GPIO_SPI1_DRDY_BMI088_GYR (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTC|GPIO_PIN5)






/* --- SPI3: External Connector --- */
/* CS Pin: PD7 */
#define GPIO_SPI3_CS_EXTERNAL    (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTD|GPIO_PIN7)


/* --- SPI4: ICM-42688-P --- */
/* CS Pin: PE4 */
#define GPIO_SPI4_CS_ICM42688P   (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTE|GPIO_PIN4)

/* DRDY Pin: PE3 */
#define GPIO_SPI4_DRDY_ICM42688P (GPIO_INPUT|GPIO_FLOAT|GPIO_EXTI|GPIO_PORTE|GPIO_PIN3)

/* Board reset */
#define BOARD_HAS_ON_RESET 1
#define SDMMC_PIN_OFF(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_INPUT|GPIO_FLOAT|GPIO_SPEED_2MHz))

#define PX4_GPIO_INIT_LIST { \
				PX4_ADC_GPIO, \
				GPIO_CAN1_TX, GPIO_CAN1_RX, \
				GPIO_CAN2_TX, GPIO_CAN2_RX, \
				GPIO_VDD_3V3_SENSORS_EN, \
				GPIO_SAFETY_SWITCH_IN, \
				GPIO_SAFETY_SWITCH_LED, \
				GPIO_ALARM_OUTPUT, \
				GPIO_LED_WS2812B, \
				GPIO_RPI_CM4_STATUS, \
				/* SPI Chip Selects - CRITICAL */ \
				GPIO_SPI1_CS_BMI088_ACC, \
				GPIO_SPI1_CS_BMI088_GYR, \
				GPIO_SPI2_CS_FRAM, \
				GPIO_SPI2_CS_SDCARD, \
				GPIO_SPI3_CS_EXTERNAL, \
				GPIO_SPI4_CS_ICM42688P, \
				/* SPI Data Ready Pins - CRITICAL */ \
				GPIO_SPI1_DRDY_BMI088_ACC, \
				GPIO_SPI1_DRDY_BMI088_GYR, \
				GPIO_SPI4_DRDY_ICM42688P, \
				/* USB VBUS */ \
				GPIO_OTGFS_VBUS, \
				GPIO_TONE_ALARM_IDLE, \
			}

#define BOARD_ENABLE_CONSOLE_BUFFER

/****************************************************************************************************
 * Public Functions
 ****************************************************************************************************/

#ifndef __ASSEMBLY__

__BEGIN_DECLS

int stm32_sdio_initialize(void);
extern void stm32_spiinitialize(void);
extern void stm32_usbinitialize(void);
extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

__END_DECLS

#endif /* __ASSEMBLY__ */
