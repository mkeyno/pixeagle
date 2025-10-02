#include <px4_arch/i2c_hw_description.h>
#include <nuttx/compiler.h>  // For __attribute__((weak))

#include <lib/drivers/device/Device.hpp>
#include <px4_platform_common/i2c.h>

constexpr px4_i2c_bus_t px4_i2c_buses[I2C_BUS_MAX_BUS_ITEMS] = {
    initI2CBusExternal(1),  // I2C1: External (PB6 SCL, PB7 SDA)
    initI2CBusInternal(3),  // I2C3: Internal (PA8 SCL, PC9 SDA for IST8310/BMP388)
    initI2CBusInternal(4),  // I2C4: Internal (PB8 SCL, PB9 SDA for BMP390)
};

__attribute__((weak)) bool px4_i2c_device_external(const uint32_t device_id)
{
    {
        // Mark internal sensors as non-external
        // IST8310 (I2C3, 0x0E)
        device::Device::DeviceId device_id_ist8310{};
        device_id_ist8310.devid_s.bus_type = device::Device::DeviceBusType_I2C;
        device_id_ist8310.devid_s.bus = 3;
        device_id_ist8310.devid_s.address = 0x0E;
        device_id_ist8310.devid_s.devtype = DRV_MAG_DEVTYPE_IST8310;

        if (device_id_ist8310.devid == device_id) {
            return false;
        }

        // BMP388 (I2C3, 0x77)
        device::Device::DeviceId device_id_bmp388{};
        device_id_bmp388.devid_s.bus_type = device::Device::DeviceBusType_I2C;
        device_id_bmp388.devid_s.bus = 3;
        device_id_bmp388.devid_s.address = 0x77;
        device_id_bmp388.devid_s.devtype = DRV_BARO_DEVTYPE_BMP388;

        if (device_id_bmp388.devid == device_id) {
            return false;
        }

        // BMP390 (I2C4, 0x77)
        device::Device::DeviceId device_id_bmp390{};
        device_id_bmp390.devid_s.bus_type = device::Device::DeviceBusType_I2C;
        device_id_bmp390.devid_s.bus = 4;
        device_id_bmp390.devid_s.address = 0x77;
        device_id_bmp390.devid_s.devtype = DRV_BARO_DEVTYPE_BMP390;

        if (device_id_bmp390.devid == device_id) {
            return false;
        }
    }

    device::Device::DeviceId dev_id{};
    dev_id.devid = device_id;
    return px4_i2c_bus_external(dev_id.devid_s.bus);
}
