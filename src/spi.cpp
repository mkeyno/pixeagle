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