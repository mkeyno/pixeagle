/****************************************************************************
 * boards/px4/pixeagle/src/spi.cpp
 *
 * Pixeagle SPI bus configuration
 *
 ****************************************************************************/

#include <px4_arch/spi_hw_description.h>
#include <drivers/drv_sensor.h>
#include <nuttx/spi/spi.h>

/**
 * Pixeagle SPI Bus Configuration:
 * 
 * SPI1: BMI088 IMU
 * SPI2: FRAM & SD Card (shared bus)
 * SPI3: External (reserved)
 * SPI4: ICM-42688-P IMU
 */

constexpr px4_spi_bus_t px4_spi_buses[SPI_BUS_MAX_BUS_ITEMS] = {
	initSPIBus(SPI::Bus::SPI1, {
		initSPIDevice(DRV_ACC_DEVTYPE_BMI088, SPI::CS{GPIO::PortA, GPIO::Pin4}, SPI::DRDY{GPIO::PortC, GPIO::Pin4}),
		initSPIDevice(DRV_GYR_DEVTYPE_BMI088, SPI::CS{GPIO::PortB, GPIO::Pin2}, SPI::DRDY{GPIO::PortC, GPIO::Pin5}),
	}, {GPIO::PortA, GPIO::Pin15}),

	initSPIBus(SPI::Bus::SPI2, {
		initSPIDevice(SPIDEV_FLASH(0), SPI::CS{GPIO::PortD, GPIO::Pin10}), // FRAM
	}),

	initSPIBus(SPI::Bus::SPI3, {
		initSPIDevice(DRV_DEVTYPE_UNUSED, SPI::CS{GPIO::PortD, GPIO::Pin7}), // External
	}),

	initSPIBus(SPI::Bus::SPI4, {
		initSPIDevice(DRV_IMU_DEVTYPE_ICM42688P, SPI::CS{GPIO::PortE, GPIO::Pin4}, SPI::DRDY{GPIO::PortE, GPIO::Pin3}),
	}, {GPIO::PortA, GPIO::Pin15}),
};

static constexpr bool unused = validateSPIConfig(px4_spi_buses);