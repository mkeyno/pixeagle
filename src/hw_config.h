/*
 * hw_config.h
 *
 * Pixeagle hardware configuration for STM32H743VIT6 MCU (version V1C00)
 * Created on: May 17, 2015
 * Author: david_s5
 * Adapted for Pixeagle by [Your Name/Handle]
 *
 * Hardware:
 * - SPI1: BMI088 (PA4 CS_ACC, PB2 CS_GYR, PA5 SCK, PA6 MISO, PA7 MOSI, 20 MHz)  // UPDATED: GYR CS to PB2
 * - SPI2: MicroSD (PB11 CS, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz), FM25V01A-GTR (PD10 CS, 20 MHz)
 * - SPI3: External (PD7 CS, PB3 SCK, PB4 MISO, PB5 MOSI, 10 MHz, 5V via TXS0108ERGYR)
 * - SPI4: ICM-42688-P (PE4 CS, PE2 SCK, PE5 MISO, PE6 MOSI, PE3 DRDY, 20 MHz)  // FIXED: CS on PE4, DRDY on PE3
 * - I2C1: External (PB6 SCL, PB7 SDA, 400 kHz, 5V via TXS0108ERGYR, user-selected via QGroundControl)
 * - I2C3: IST8310 (0x0E), BMP388 (0x76) (PA8 SCL, PC9 SDA, 400 kHz)
 * - I2C4: BMP390 (0x76) (PB8 SCL, PB9 SDA, 400 kHz)
 * - UART3: PD8 TX, PD9 RX (SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent)
 * - USART2: PD5 TX, PD6 RX, PD3 CTS, PD4 RTS (telemetry, 5V, /dev/ttyS0, 57600 baud)  // UPDATED: Pins to PD3-PD6
 * - UART4: PC10 TX, PC11 RX (debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5: PC12 TX, PD2 RX (sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - UART7: PE7 TX, PE8 RX (CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - CAN1: PD0 RX, PD1 TX (5V via TCAN1044VDRQ1)  // UPDATED: Pins to PD0/PD1
 * - CAN2: PB12 RX, PB13 TX (5V via TCAN1044VDRQ1)
 * - USB OTG FS: PA9 VBUS, PA11 DM, PA12 DP (no power/overcurrent)
 * - ADC: PC0 (analog sensor), PC1 (analog sensor 2), PC2 (battery voltage, 11:1 divider), PC3 (battery current)
 * - PWM OUT: PD12 (TIM4_CH1), PD13 (TIM4_CH2), PD14 (TIM4_CH3), PD15 (TIM4_CH4), PC6 (TIM3_CH1), PC7 (TIM8_CH2), PC8 (TIM8_CH3), PA10 (TIM1_CH3)
 * - AUX GPIO: PA0 (TIM5_CH1), PA1 (TIM5_CH2), PA2 (TIM5_CH3), PA3 (TIM5_CH4), PB0 (TIM3_CH3), PE11 (TIM1_CH2)
 * - Dual WS2812B LEDs: PE14 (TIM1_CH4, FastLED)
 * - Sensor power: PA15 (active low)
 * - Tone alarm: PE13 (TIM1_CH3)
 * - PWM input: PB1 (TIM3_CH4, PPM)  // UPDATED: Added PPM on PB1
 * - External digital pins (I2C1, SPI3): 5V via TXS0108ERGYR
 * No PX4IO co-processor.
 */

#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

/****************************************************************************
 * Bootloader USB device string must contain "PX4 BL " prefix for /dev/serial/by-id/<asterisk>PX4<asterisk>
 ****************************************************************************/

#include <stm32_gpio.h>

/* Boot device selection list */
#define USB0_DEV       0x01
#define SERIAL0_DEV    0x02
#define SERIAL1_DEV    0x04

/* Bootloader configuration for STM32H743VIT6 (2MB Flash, 8x128KB sectors per bank) */
#define APP_LOAD_ADDRESS               0x08040000  /* Start after bootloader (256KB reserved) */
#define BOOTLOADER_DELAY               5000        /* 5s delay for USB/bootloader */
#define INTERFACE_USB                  1
#define INTERFACE_USB_CONFIG           "/dev/ttyACM0"
#define BOARD_VBUS                     MK_GPIO_INPUT(GPIO_OTGFS_VBUS)  /* PA9 */
#define INTERFACE_USART                1
#define INTERFACE_USART_CONFIG         "/dev/ttyS0,57600"  /* USART2 for telemetry */
#define BOOT_DELAY_ADDRESS             0x000001a0
#define BOARD_TYPE                     100          /* Unique ID for Pixeagle, update in .prototype */
#define _FLASH_KBYTES                  (*(uint32_t *)0x1FF1E880)  /* STM32H743 flash size register */
#define BOARD_FLASH_SECTORS            16          /* 2MB Flash: 2 banks x 8 sectors (128KB each) */
#define BOARD_FLASH_SIZE               (_FLASH_KBYTES * 1024)  /* 2048 KB */
#define BOARD_FIRST_FLASH_SECTOR_TO_ERASE 2  /* Start erasing after bootloader sector */

#define OSC_FREQ                       16           /* 8 MHz HSE crystal for STM32H743 */

/* WS2812B LEDs on PE14 (FastLED, no traditional red/blue LEDs) */
#define BOARD_PIN_LED_ACTIVITY         GPIO_WS2812B_LED  /* PE14 */
#define BOARD_PIN_LED_BOOTLOADER       GPIO_WS2812B_LED  /* PE14 */
#define BOARD_LED_ON                   1  /* FastLED sets WS2812B color */
#define BOARD_LED_OFF                  0  /* FastLED clears WS2812B */

#define SERIAL_BREAK_DETECT_DISABLED   1




/*
 * Force bootloader pin (disabled to avoid ESC risks)
 */
#undef BOARD_FORCE_BL_PIN_OUT
#undef BOARD_FORCE_BL_PIN_IN
#undef BOARD_POWER_PIN_OUT
#undef BOARD_POWER_ON
#undef BOARD_POWER_OFF
#undef BOARD_POWER_PIN_RELEASE

#if !defined(ARCH_SN_MAX_LENGTH)
# define ARCH_SN_MAX_LENGTH 12
#endif

#if !defined(APP_RESERVATION_SIZE)
# define APP_RESERVATION_SIZE (256 * 1024)  /* 256KB for bootloader */
#endif

#if !defined(USB_DATA_ALIGN)
# define USB_DATA_ALIGN
#endif

#ifndef BOOT_DEVICES_SELECTION
# define BOOT_DEVICES_SELECTION USB0_DEV|SERIAL0_DEV|SERIAL1_DEV
#endif

#ifndef BOOT_DEVICES_FILTER_ONUSB
# define BOOT_DEVICES_FILTER_ONUSB USB0_DEV|SERIAL0_DEV|SERIAL1_DEV
#endif

#endif /* HW_CONFIG_H_ */