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
usbipd list
usbipd bind --busid 4-4
usbipd attach --wsl --busid <busid>
usbipd attach --wsl --busid   1-1  --auto-attach
usbipd detach --busid <busid>

lsusb

st-info --probe
st-flash --version
openocd -v
st-util -p 4242



sudo service udev start
sudo udevadm control --reload-rules
sudo udevadm trigger
st-info --probe


arm-none-eabi-addr2line  -e ~/PX4-Autopilot/build/px4_pixeagle_default/px4_pixeagle_default.elf 0x0801a4b0
arm-none-eabi-nm   -l    -S ~/PX4-Autopilot/build/px4_pixeagle_default/px4_pixeagle_default.elf | grep -i romfs_img
 arm-none-eabi-readelf   -S ~/PX4-Autopilot/build/px4_pixeagle_default/px4_pixeagle_default.elf
arm-none-eabi-readelf    -s ~/PX4-Autopilot/build/px4_pixeagle_default/px4_pixeagle_default.elf | grep romfs_img
arm-none-eabi-objdump    -h ~/PX4-Autopilot/build/px4_pixeagle_default/px4_pixeagle_default.elf | grep romfs
arm-none-eabi-size          ~PX4-Autopilot/build/px4_pixeagle_default/px4_pixeagle_default.elf




ELF=build/px4_pixeagle_default/px4_pixeagle_default.elf
arm-none-eabi-objcopy -O binary --only-section=.romfs "$ELF" romfs.bin
arm-none-eabi-readelf -S "$ELF" | grep -i romfs
arm-none-eabi-objcopy -O binary --only-section=.romfs "$ELF" romfs.bin
strings -a romfs.bin | sed -n '1,200p'

# 1. Check your current remotes (to see what's set up)
git remote -v
# 2. Add the official PX4 repository as 'upstream' (do this once)
git remote add upstream https://github.com/PX4/PX4-Autopilot.git

git fetch upstream
git checkout upstream/main -- platforms/nuttx/src/px4/common/tasks.cpp

grep "CONFIG_NSH_ROMFSETC" build/px4_pixeagle_default/NuttX/nuttx/.config


stm32h7/stm32_start.c ->  platforms/nuttx/NuttX/nuttx/sched/init/nx_start.c

# Start code-server
code-server --auth none --bind-addr 0.0.0.0:8080

# Access at: http://76.13.21.71:8080
# Set up password authentication:
code-server --bind-addr 0.0.0.0:8080
# Password will be in ~/.config/code-server/config.yaml


ssh hamid@76.13.21.71 -p 1666
srv1082842.hstgr.cloud:5901

cd ~
sudo chown -R $(whoami):$(whoami) PX4-Autopilot
rm -rf ./build/px4_pixeagle_default/  && make px4_pixeagle_default
 make distclean && make px4_pixeagle_default

 #if defined(CONFIG_NSH_ROMFSETC) && !defined(CONFIG_NSH_DISABLESCRIPT)
  Execute the system init script
  
  
  scp -P 1666 -r D:/pixeagle hamid@76.13.21.71:~/PX4-Autopilot/boards/px4
   
  Standard boards have CONFIG_NSH_ROMFSETC=y but no CONFIG_NSH_ARCHROMFS. 
  NuttX default image (NSHInitVol, 110-byte rcS) runs. For standard boards, 
  this is fine because the real init script system comes from the PX4 platform ROMFS (config_romfs_root = px4fmu_common), 
  which is mounted by board_app_initialize() separately via the romfs linker library (not via nsh_romfsetc).
  The NuttX default rcS just runs mkrd + sets up /tmp, which is benign.
  CONFIG_NSH_ARCHROMFS required header  @  .boards/px4/pixeagle/nuttx-config/include/nsh_romfsimg.h
  
  
  
  USB console (/dev/ttyACM0) does not exist until usb_console start executes inside rc.sysinit.
  Since NSH runs rc.sysinit directly, rcS should either be removed or merged into rc.sysinit. 
  CONFIG_DRIVERS_CDCACM_AUTOSTART=y in your px4board conflicts with usb_console start in rc.sysinit — two things trying to own the USB CDC-ACM device.
  usb_console is a PX4 module that wraps the NuttX CDC-ACM system interface. It may not
be included in your default board configuration. Check:
grep -r "usb_console" boards/px4/pixeagle/default.px4board
  
 CONFIG_FLASH_BASED_PARAMS(refer to internal MCU flash (STM32H743's 2MB on-chip flash), NOT external FRAM/EEPROM.) tells NuttX to compile the flash-backed parameter driver,
but PX4's MTD manifest (mtd.cpp) declares no devices. At runtime px4_mft_configure()
logs this:
[PX4_CONFIGURE]   mfts[0]: type=1, pmft=0x81d80ec   ← MTD entry, nconfigs=0
PX4 then silently falls back to in-RAM parameters. param select /fs/microsd/params in
rc.sysinit correctly redirects to SD card storage, so parameters survive reboots only
if the SD card is present.  
  
Storage					Location					Persistent				SpeedWear Limit							Setup
SD Card 		   		/fs/microsd/params			Yes						Slow~100K writes/sector					param select /fs/microsd/params in rcS
Internal Flash			MCU PROGMEM partition		Yes						Fast~10K erase cycles					CONFIG_STM32H7_PROGMEM=y + MTD entry
External FRAM			SPI/I2C FRAM chip			Yes						Fast	Unlimited						MTD driver + manifest entry
RAM (default)			SRAM						NO						Fastest	Unlimited						Nothing — PX4 fallback 
  
If   need in-flight bootloader updates,  must:
   CONFIG_STM32H7_PROGMEM=y
   CONFIG_MTD_PROGMEM=y
   CONFIG_FLASH_BASED_PARAMS=y
   CONFIG_FS_PROCFS_INCLUDE_PROGMEM=y  
Add PROGMEM MTD entry to mtd.cpp:

cpp   static const px4_mtd_device_t progmem_dev = {
       .bus_id = 0,
       .devid  = SPIDEV_FLASH(0),
       .type   = MTD_PROGMEM,
   };
   static const px4_mtd_manifest_t board_mtd_config = {
       .nconfigs = 1,
       .entries  = { &progmem_dev }
   };   
   
  
bl_update is a PX4 command that overwrites the bootloader in flash on flight:
nsh> bl_update /fs/microsd/bootloader.bin  
# Disable bl_update command  
CONFIG_SYSTEMCMDS_BL_UPDATE=n  
  
  Internal Flash (PROGMEM) Param partition (carved from end of firmware space): typically 16-32 KB
  
 1. board_app_initialize()
   └─> stm32_usbinitialize()   ← Initializes USB PHY, clocks, GPIO
       (No device driver bound yet — USB is electrically active but no endpoints)

2. NSH starts and runs /etc/init.d/rcS

3. rcS calls rc.sysinit

4. rc.sysinit runs: usb_console start
   └─> Calls NuttX cdcacm_initialize()
       └─> Creates /dev/ttyACM0 
       └─> Binds CDC-ACM class driver to USB core
       └─> Enumerates as USB device on host PC

5. Windows sees: VID=0x26AC PID=0x0039 "PX4 Pixeagle" 
  
  
When CONFIG_NSH_ROMFSETC=y, NSH executes /etc/init.d/rcS automatically  

#define CONFIG_NSH_ROMFSMOUNTPT "/etc"
#define CONFIG_NSH_SYSINITSCRIPT "init.d/rc.sysinit"
#define CONFIG_NSH_INITSCRIPT "init.d/rcS"

nx_start()
  └─ task_create("nsh_main", ...)
       └─ nsh_main()
            ├─ platforms/nuttx/NuttX/apps/system/nsh/nsh_main.c
            └─ nsh_consolemain()
                 ├─ platforms/nuttx/NuttX/apps/nshlib/nsh_consolemain.c
                 └─ nsh_initialize()
                      ├─ platforms/nuttx/NuttX/apps/nshlib/nsh_init.c
                      │
                      │  Step A: ARCHINIT
                      ├─ boardctl(BOARDIOC_INIT, 0)
                      │    └─ board_app_initialize()   ← your init.c
                      │         (during this call /dev/ram0 ALREADY EXISTS — see why below)
                      │
                      │  Step B: ROMFSETC ← THIS IS WHERE IT DIES
                      └─ nsh_romfsetc()
                           ├─ platforms/nuttx/NuttX/apps/nshlib/nsh_romfsetc.c
                           │
                           │  With CONFIG_BOARDCTL_ROMDISK=y (your defconfig line 155):
                           ├─ boardctl(BOARDIOC_ROMDISK, &desc)
                           │    └─ board_ioctl.c → romdisk_register(0,...) → -EEXIST ← FAIL
                           │    OR returns OK but then:
                           ├─ nx_mount("/dev/ram0", "/etc", "romfs") → -EBUSY ← FAIL
                           │
                           └─ if (ret < 0) return ERROR;  ← rcS NEVER RUNS  
  
board_app_initialize enables the USB PHY and Clocks, but not the CDC-ACM class driver.
The Class Driver is started by usb_console start inside rc.sysinit
 Because rcS (and thus rc.sysinit) never executes, the USB device never enumerates.
 Windows sees the electrical connection (PHY on) but gets no data, so it drops the device. 
 
 When the board boots, nsh_romfsetc() mounts your internal ROMFS image to the /etc directory. 
 Because defconfig has CONFIG_NSH_ROMFSMOUNTPT="/etc", NSH looks inside that mount point for its startup script.(/etc/init.d/rcS)
 
 The SD card must be mounted before the parameter system starts. The parameter system starts very early in the boot sequence.
 
 
 
 
 
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


#undef board_query_manifest
#undef board_get_manifest

#include <nuttx/spi/spi.h>
#include <px4_platform_common/px4_manifest.h>


/****************************************************************************************************
 * Definitions
 ****************************************************************************************************/
#define BOARD_I2C_LATEINIT

/* LEDs */
#define BOARD_HAS_CONTROL_STATUS_LEDS      	1
#define BOARD_OVERLOAD_LED     				LED_RED
#define BOARD_ARMED_STATE_LED  				LED_GREEN

/* I2C late init */
#define BOARD_I2C_LATINIT

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

//#define FLASH_BASED_PARAMS  //parameter storage on flash/FRAM (small, ~8-16KB partitions for params only)
#define BOARD_HAS_FRAM             0// 1
#define BOARD_VALUE_PARAM_SAVE_SIZE 8192
//#define BOARD_USE_EXTERNAL_FLASH //Configuration and firmware are in external flash


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



/* CAN */
#define PX4_CAN_BUS_1 1
#define PX4_CAN_BUS_2 2

/* These will now use the PD0/PD1 values from board.h */
#define GPIO_CAN1_RX_EXTERNAL GPIO_CAN1_RX
#define GPIO_CAN1_TX_EXTERNAL GPIO_CAN1_TX


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
