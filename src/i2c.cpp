/****************************************************************************
 * boards/pixeagle/pixeagle/src/i2c.cpp
 *
 * Pixeagle I2C bus configuration
 *
 ****************************************************************************/

#include <px4_arch/i2c_hw_description.h>

/**
 * Pixeagle I2C Bus Configuration:
 * 
 * I2C1 (External): PB6 (SCL), PB7 (SDA) - External devices
 * I2C3 (Internal): PA8 (SCL), PC9 (SDA) - BMP388 & IST8310 (shared)
 * I2C4 (Internal): PB8 (SCL), PB9 (SDA) - BMP390
 */

constexpr px4_i2c_bus_t px4_i2c_buses[I2C_BUS_MAX_BUS_ITEMS] = {
	initI2CBusExternal(1),  // I2C1: External on PB6/PB7
	initI2CBusInternal(3),  // I2C3: Internal - BMP388, IST8310
	initI2CBusInternal(4),  // I2C4: Internal - BMP390
};