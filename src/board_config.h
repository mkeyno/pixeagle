#pragma once

/****************************************************************************************************
 * Included Files
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

/* PX4IO connection configuration */

#define BOARD_USES_PX4IO_VERSION       2
#define PX4IO_SERIAL_DEVICE            "/dev/ttyS5"
#define PX4IO_SERIAL_TX_GPIO           GPIO_UART5_TX
#define PX4IO_SERIAL_RX_GPIO           GPIO_UART5_RX
#define PX4IO_SERIAL_BASE              STM32_UART5_BASE
#define PX4IO_SERIAL_VECTOR            STM32_IRQ_UART5
#define PX4IO_SERIAL_TX_DMAMAP         DMAMAP_UART5_TX
#define PX4IO_SERIAL_RX_DMAMAP         DMAMAP_UART5_RX
#define PX4IO_SERIAL_RCC_REG           STM32_RCC_APB1LENR
#define PX4IO_SERIAL_RCC_EN            RCC_APB1LENR_UART5EN
#define PX4IO_SERIAL_CLOCK             STM32_PCLK1_FREQUENCY
#define PX4IO_SERIAL_BITRATE           1500000               /* 1.5Mbps -> max rate for IO */

/* LEDs are driven with push open drain to support Anode to 5V or 3.3V */
/* Note: Using WS2812B on PE14 for RGB LED (bitbang implementation) */
/* PE13 and PE15 available for debugging/status */

#define GPIO_nLED_RED        /* PE13 */  (GPIO_OUTPUT|GPIO_OPENDRAIN|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTE|GPIO_PIN13)
#define GPIO_nLED_GREEN      /* PE15 */  (GPIO_OUTPUT|GPIO_OPENDRAIN|GPIO_SPEED_50MHz|GPIO_OUTPUT_SET|GPIO_PORTE|GPIO_PIN15)

#define BOARD_HAS_CONTROL_STATUS_LEDS      1
#define BOARD_OVERLOAD_LED     LED_RED
#define BOARD_ARMED_STATE_LED  LED_GREEN

/* WS2812B RGB LED - using bitbang on PE14 */
#define GPIO_LED_WS2812B     /* PE14 */  (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_100MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN14)

/* ADC channels */

/* Define GPIO pins used as ADC N.B. Channel numbers must match below */

#define PX4_ADC_GPIO  \
	/* PC0 */  GPIO_ADC123_INP10, \
	/* PC1 */  GPIO_ADC123_INP11, \
	/* PC2 */  GPIO_ADC123_INP12, \
	/* PC3 */  GPIO_ADC12_INP13

/* Define Channel numbers must match above GPIO pin IN(n)*/

#define ADC_SCALED_V5_CHANNEL               /* PC0 */  10
#define ADC_SCALED_VDD_3V3_SENSORS_CHANNEL  /* PC1 */  11
#define ADC_BATTERY_VOLTAGE_CHANNEL         /* PC2 */  12
#define ADC_BATTERY_CURRENT_CHANNEL         /* PC3 */  13

#define ADC_CHANNELS \
	((1 << ADC_SCALED_V5_CHANNEL)              | \
	 (1 << ADC_SCALED_VDD_3V3_SENSORS_CHANNEL) | \
	 (1 << ADC_BATTERY_VOLTAGE_CHANNEL)        | \
	 (1 << ADC_BATTERY_CURRENT_CHANNEL))

/* HW has to large of R termination on ADC todo:change when HW value is chosen */

#define BOARD_ADC_OPEN_CIRCUIT_V     (5.6f)

/* Safety switch and alarm */

#define GPIO_SAFETY_SWITCH_IN   /* PE12 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_PORTE|GPIO_PIN12)
#define GPIO_SAFETY_SWITCH_LED  /* PE15 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN15)
#define GPIO_ALARM_OUTPUT       /* PE13 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN13)



/* PWM - 8 channels total */

#define DIRECT_PWM_OUTPUT_CHANNELS  8
#define BOARD_NUM_IO_TIMERS 4

/* Power supply control and monitoring GPIOs */
/* PA15 - Reset peripheral power rails (sensors via PA15, active high) */

#define GPIO_VDD_3V3_SENSORS_EN     /* PA15 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTA|GPIO_PIN15)

/* Define True logic Power Control in arch agnostic form */

#define VDD_3V3_SENSORS_EN(on_true)    px4_arch_gpiowrite(GPIO_VDD_3V3_SENSORS_EN, (on_true))

/* Tone alarm output - PE13 can be used for alarm */

#define TONE_ALARM_TIMER        1   /* timer 1 */
#define TONE_ALARM_CHANNEL      3   /* PE14 TIM1_CH3   */

#define GPIO_BUZZER_1           /* PE13 */ (GPIO_OUTPUT|GPIO_PUSHPULL|GPIO_SPEED_2MHz|GPIO_OUTPUT_CLEAR|GPIO_PORTE|GPIO_PIN13)

#define GPIO_TONE_ALARM_IDLE    GPIO_BUZZER_1
#define GPIO_TONE_ALARM         GPIO_BUZZER_1

/* USB OTG FS
 *
 * PA9  OTG_FS_VBUS VBUS sensing
 */
#define GPIO_OTGFS_VBUS         /* PA9 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_SPEED_100MHz|GPIO_PORTA|GPIO_PIN9)

/* High-resolution timer */
#define HRT_TIMER               2  /* use timer1 for the HRT */
#define HRT_TIMER_CHANNEL       1  /* use capture/compare channel 3 PA10 */

/* PWM input driver - using PB1 (PPM input) attached to timer3 */

#define PWMIN_TIMER                       3
#define PWMIN_TIMER_CHANNEL    /* T3C4 */ 4
#define GPIO_PWM_IN            /* PB1 */  GPIO_TIM3_CH4IN

/* RC Input - SBUS on USART3 */
#define RC_SERIAL_PORT                    "/dev/ttyS2"

/* SDIO */
#define SDIO_SLOTNO                    0  /* Only one slot */
#define SDIO_MINOR                     0

/* SD card bringup does not work if performed on the IDLE thread because it
 * will cause waiting.  Use either:
 *
 *  CONFIG_BOARDCTL=y, OR
 *  CONFIG_BOARD_INITIALIZE=y && CONFIG_BOARD_INITTHREAD=y
 */

#if defined(CONFIG_BOARD_INITIALIZE) && !defined(CONFIG_BOARDCTL) && \
   !defined(CONFIG_BOARD_INITTHREAD)
#  warning SDIO initialization cannot be perfomed on the IDLE thread
#endif

/* By Providing BOARD_ADC_USB_CONNECTED (using the px4_arch abstraction)
 * this board support the ADC system_power interface, and therefore
 * provides the true logic GPIO BOARD_ADC_xxxx macros.
 */
#define BOARD_ADC_USB_CONNECTED (px4_arch_gpioread(GPIO_OTGFS_VBUS))
#define BOARD_ADC_USB_VALID     (px4_arch_gpioread(GPIO_OTGFS_VBUS))

/* Board never powers off the Servo rail */

#define BOARD_ADC_SERVO_VALID     (1)

#define BOARD_ADC_BRICK_VALID     (1)

/* Raspberry Pi CM4 status pin */
#define GPIO_RPI_CM4_STATUS     /* PD11 */ (GPIO_INPUT|GPIO_PULLDOWN|GPIO_PORTD|GPIO_PIN11)

/* This board provides a DMA pool and APIs */
#define BOARD_DMA_ALLOC_POOL_SIZE 5120

/* This board provides the board_on_reset interface */

#define BOARD_HAS_ON_RESET 1
#define SDMMC_PIN_OFF(def) (((def) & (GPIO_PORT_MASK | GPIO_PIN_MASK)) | (GPIO_INPUT|GPIO_FLOAT|GPIO_SPEED_2MHz))

#define PX4_GPIO_INIT_LIST { \
		PX4_ADC_GPIO,                     \
		GPIO_CAN1_TX,                     \
		GPIO_CAN1_RX,                     \
		GPIO_CAN2_TX,                     \
		GPIO_CAN2_RX,                     \
		GPIO_VDD_3V3_SENSORS_EN,          \
		GPIO_SAFETY_SWITCH_IN,            \
		GPIO_SAFETY_SWITCH_LED,           \
		GPIO_ALARM_OUTPUT,                \
		GPIO_LED_WS2812B,                 \
		GPIO_RPI_CM4_STATUS,              \
		SDMMC_PIN_OFF(GPIO_SDMMC1_D0),    \
		SDMMC_PIN_OFF(GPIO_SDMMC1_D1),    \
		SDMMC_PIN_OFF(GPIO_SDMMC1_D2),    \
		SDMMC_PIN_OFF(GPIO_SDMMC1_D3),    \
		SDMMC_PIN_OFF(GPIO_SDMMC1_CMD),   \
		GPIO_TONE_ALARM_IDLE,             \
	}

#define BOARD_ENABLE_CONSOLE_BUFFER

__BEGIN_DECLS

/****************************************************************************************************
 * Public Types
 ****************************************************************************************************/

/****************************************************************************************************
 * Public data
 ****************************************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************************************
 * Public Functions
 ****************************************************************************************************/

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   Initialize SDIO-based MMC/SD card support
 *
 ****************************************************************************/

int stm32_sdio_initialize(void);

/****************************************************************************************************
 * Name: stm32_spiinitialize
 *
 * Description:
 *   Called to configure SPI chip select GPIO pins for the board.
 *
 ****************************************************************************************************/

extern void stm32_spiinitialize(void);

extern void stm32_usbinitialize(void);

extern void board_peripheral_reset(int ms);

#include <px4_platform_common/board_common.h>

#endif /* __ASSEMBLY__ */

__END_DECLS
