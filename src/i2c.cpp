/**
 * @file i2c.cpp
 *
 * Pixeagle I2C bus configuration
 *
 * Configures I2C buses for the Pixeagle board (STM32H743VIT6, version V1C00):
 * - I2C1: External interface (PB6 SCL, PB7 SDA, 5V via TXS0108ERGYR, 400 kHz)
 * - I2C3: IST8310 (0x0E), BMP388 (0x76) (PC9 SCL, PA8 SDA, internal, 400 kHz)
 * - I2C4: BMP390 (0x76) (PB8 SCL, PB9 SDA, internal, 400 kHz)
 * Other hardware:
 * - BMI088 (SPI1, PA4 CS_ACC, PB2 CS_GYR, PA5–PA7, 20 MHz)
 * - ICM-42688-P (SPI4, PE4 CS, PE2/PE5/PE6, PE3 DRDY, 20 MHz)
 * - MicroSD (SPI2, PB11 CS, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz)
 * - FM25V01A-GTR (SPI2, PD10 CS, 20 MHz)
 * - External SPI (SPI3, PB3 SCK, PB4 MISO, PB5 MOSI, PD7 CS, 10 MHz, 5V via TXS0108ERGYR)
 * - Dual WS2812B LEDs (PE14, TIM1)
 * - UART4 (PC10/PC11, debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5 (PC12/PD2, sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V, /dev/ttyS0, 57600 baud) // UPDATED: USART2 pins to PD3-PD6
 * - UART7 (PE7/PE8, CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - UART3 (PD8/PD9, SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent: 0 normal, 1 inverted)
 * - CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1) // UPDATED: CAN1 pins to PD0/PD1
 * - USB OTG FS (PA9 VBUS, PA11 DM, PA12 DP, no power/overcurrent pins)
 * No PX4IO co-processor. I2C1 and SPI3 use TXS0108ERGYR for 5V level translation.
 */

#include <px4_arch/i2c_hw_description.h>
#include <lib/drivers/device/Device.hpp>
#include <px4_platform_common/i2c.h>

constexpr px4_i2c_bus_t px4_i2c_buses[I2C_BUS_MAX_BUS_ITEMS] = {
    initI2CBusExternal(1, 400000), // I2C1: External, PB6 SCL, PB7 SDA, 400 kHz, 5V via TXS0108ERGYR
    initI2CBus(3, 400000),        // I2C3: IST8310 (0x0E), BMP388 (0x76), PC9 SCL, PA8 SDA, internal
    initI2CBus(4, 400000),        // I2C4: BMP390 (0x76), PB8 SCL, PB9 SDA, internal
};

bool px4_i2c_device_external(const uint32_t device_id)
{
    // Mark IST8310 (I2C3, 0x0E), BMP388 (I2C3, 0x76), and BMP390 (I2C4, 0x76) as internal
    device::Device::DeviceId 	device_id_ist8310{};
								device_id_ist8310.devid_s.bus_type = device::Device::DeviceBusType_I2C;
								device_id_ist8310.devid_s.bus = 3;
								device_id_ist8310.devid_s.address = 0x0E;
								device_id_ist8310.devid_s.devtype = DRV_MAG_DEVTYPE_IST8310;
							if (device_id_ist8310.devid == device_id)         return false;
    

    device::Device::DeviceId 	device_id_bmp388{};
								device_id_bmp388.devid_s.bus_type = device::Device::DeviceBusType_I2C;
								device_id_bmp388.devid_s.bus = 3;
								device_id_bmp388.devid_s.address = 0x76;
								device_id_bmp388.devid_s.devtype = DRV_BARO_DEVTYPE_BMP388;
							if (device_id_bmp388.devid == device_id)         return false;    

    device::Device::DeviceId 	device_id_bmp390{};
								device_id_bmp390.devid_s.bus_type = device::Device::DeviceBusType_I2C;
								device_id_bmp390.devid_s.bus = 4;
								device_id_bmp390.devid_s.address = 0x76;
								device_id_bmp390.devid_s.devtype = DRV_BARO_DEVTYPE_BMP390;
							if (device_id_bmp390.devid == device_id)       return false;    



    device::Device::DeviceId 	dev_id{};
								dev_id.devid = device_id;
    return px4_i2c_bus_external(dev_id.devid_s.bus);
}