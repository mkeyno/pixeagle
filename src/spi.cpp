/**
 * @file spi.cpp
 *
 * Pixeagle SPI bus configuration
 *
 * Configures SPI buses for the Pixeagle board (STM32H743VIT6, version V1C00):
 * - SPI1: BMI088 (accel CS: PA4, gyro CS: PB2, PA5 SCK, PA6 MISO, PA7 MOSI, 20 MHz)
 * - SPI2: MicroSD (CS: PB11, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz), FM25V01A-GTR (CS: PD10, 20 MHz)
 * - SPI4: ICM-42688-P (CS: PE4, PE2 SCK, PE5 MISO, PE6 MOSI, DRDY: PE3, 20 MHz)
 * - SPI3: External interface (PB3 SCK, PB4 MISO, PB5 MOSI, PD7 CS, 10 MHz, 5V via TXS0108ERGYR)
 * Other hardware:
 * - IST8310 (I2C3, PC9/PA8, 0x0E), BMP388 (I2C3, 0x76), BMP390 (I2C4, PB8/PB9, 0x76)
 * - External I2C (I2C1, PB6 SCL, PB7 SDA, 400 kHz, 5V via TXS0108ERGYR)
 * - Dual WS2812B LEDs (PE14, TIM1)
 * - UART4 (PC10/PC11, debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5 (PC12/PD2, sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V, /dev/ttyS0, 57600 baud) // UPDATED: USART2 pins to PD3-PD6
 * - UART7 (PE7/PE8, CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - UART3 (PD8/PD9, SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent)
 * - CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1) // UPDATED: CAN1 pins to PD0/PD1
 * - USB OTG FS (PA9 VBUS, PA11 DM, PA12 DP, no power/overcurrent pins)
 * No PX4IO co-processor. I2C1 and SPI3 use TXS0108ERGYR for 5V level translation.
 * Called by init.c (stm32_spisd_initialize for MicroSD) and aligns with manifest.c.
 */

#include <px4_arch/spi_hw_description.h>
#include <drivers/drv_sensor.h>
#include <nuttx/spi/spi.h>

constexpr px4_spi_bus_t px4_spi_buses[] = {
	// SPI1: BMI088 sensors
	initSPIBus(SPI::Bus::SPI1, {
		initSPIDevice(DRV_ACC_DEVTYPE_BMI088, SPI::CS{GPIO::PortA, GPIO::Pin4}),
		initSPIDevice(DRV_GYR_DEVTYPE_BMI088, SPI::CS{GPIO::PortB, GPIO::Pin2}),
	}),
	// SPI2: FRAM and MicroSD  
	initSPIBus(SPI::Bus::SPI2, {
		initSPIDevice(DRV_DEVTYPE_UNUSED, SPI::CS{GPIO::PortD, GPIO::Pin10}),  // FRAM
		initSPIDevice(DRV_DEVTYPE_UNUSED, SPI::CS{GPIO::PortB, GPIO::Pin11}),  // MicroSD
	}),
	// SPI3: External
	initSPIBus(SPI::Bus::SPI3, {
		initSPIDevice(DRV_DEVTYPE_UNUSED, SPI::CS{GPIO::PortD, GPIO::Pin7}),  // External
	}),
	// SPI4: ICM42688P
	initSPIBus(SPI::Bus::SPI4, {
		initSPIDevice(DRV_IMU_DEVTYPE_ICM42688P, SPI::CS{GPIO::PortE, GPIO::Pin4}),
	}),
};

//static constexpr bool unused = validateSPIConfig(px4_spi_buses);
