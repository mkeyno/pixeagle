#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

/****************************************************************************
 * Bootloader Only
 * tells the bootloader where to flash the firmware, which LED to blink during boot, and which USB/UART port to listen to for uploads.
 * Bootloader USB identification
 ****************************************************************************/
#define USBDEVICESTRING      "PX4 BL Pixeagle"
#define USBPRODUCTID         0x0039
#define USBMFGSTRING         "PixEagle"

#define BOARD_LED_ON                   0
#define BOARD_LED_OFF                  1

/* LED/Pin Mapping */
#define LED_STARTED        0
#define LED_HEAPALLOCATE   1
#define LED_IRQSENABLED    2
#define LED_STACKCREATED   3
#define LED_INIRQ          4
#define LED_SIGNAL         5
#define LED_ASSERTION      6
#define LED_PANIC          7
#define LED_IDLE           8


/* Pixeagle Bootloader Configuration */

/* Boot device selection list*/
#define USB0_DEV       0x01
#define SERIAL0_DEV    0x02
#define SERIAL1_DEV    0x04

/* Memory Configuration - STM32H743VIT6 */
#define APP_LOAD_ADDRESS               0x08020000
#define BOOTLOADER_DELAY               3000

/* USB Interface Configuration */
#define INTERFACE_USB                  1
#define INTERFACE_USB_CONFIG           "/dev/ttyACM0"
/* VBUS sensing disabled per user request */
// #define BOARD_VBUS                     MK_GPIO_INPUT(GPIO_OTGFS_VBUS)

/* UART Interface Configuration */
#define INTERFACE_USART                1
#define INTERFACE_USART_CONFIG         "/dev/ttyS0,115200"  /* UART8 PE0/PE1 for bootloader serial */

/* Board Identification */
#define BOOT_DELAY_ADDRESS             0x000001a0
#define BOARD_TYPE                     1666

/* Flash Configuration - STM32H743VIT6 has 2MB flash */
#define _FLASH_KBYTES                  (*(uint32_t *)0x1FF1E880)
#define BOARD_FLASH_SECTORS            (15)
#define BOARD_FLASH_SIZE               (_FLASH_KBYTES * 1024)

/* Oscillator Configuration
 * Updated to 16 MHz External Crystal (HSE) to match IOC
 */
#define OSC_FREQ                       16

/* Serial Break Detection */
#define SERIAL_BREAK_DETECT_DISABLED   1

#if !defined(ARCH_SN_MAX_LENGTH)
# define ARCH_SN_MAX_LENGTH 12
#endif

#if !defined(APP_RESERVATION_SIZE)
#  define APP_RESERVATION_SIZE 0
#endif

#if !defined(BOARD_FIRST_FLASH_SECTOR_TO_ERASE)
#  define BOARD_FIRST_FLASH_SECTOR_TO_ERASE 1
#endif

#if !defined(USB_DATA_ALIGN)
# define USB_DATA_ALIGN
#endif

#ifndef BOOT_DEVICES_SELECTION
#  define BOOT_DEVICES_SELECTION USB0_DEV|SERIAL0_DEV|SERIAL1_DEV
#endif

#ifndef BOOT_DEVICES_FILTER_ONUSB
#  define BOOT_DEVICES_FILTER_ONUSB USB0_DEV|SERIAL0_DEV|SERIAL1_DEV
#endif

#endif /* HW_CONFIG_H_ */
