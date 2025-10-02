/**
 * @file mtd.cpp
 *
 * Pixeagle MTD (Memory Technology Device) configuration
 *
 * Configures non-volatile storage for the Pixeagle board (STM32H743VIT6, version V1C00):
 * - FM25V01A-GTR (SPI2, PD10 CS, 128 KB, 4096 blocks of 32 bytes, /fs/mtd_params)  // FIXED: 128 KB capacity
 * - MicroSD (SPI2, PB11 CS, for logging, not MTD but included in manifest)
 * Hardware:
 * - BMI088 (SPI1, PA4 CS_ACC, PB2 CS_GYR, PA5–PA7, 20 MHz)  // UPDATED: GYR CS to PB2 per pin table
 * - ICM-42688-P (SPI4, PE4 CS, PE2/PE5/PE6, PE3 DRDY, 20 MHz)  // FIXED: CS on PE4, DRDY on PE3
 * - IST8310 (I2C3, PA8 SCL/PC9 SDA, 0x0E, 400 kHz), BMP388 (I2C3, 0x76), BMP390 (I2C4, PB8 SCL/PB9 SDA, 0x76)
 * - MicroSD (SPI2, PB11 CS, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz)
 * - FM25V01A-GTR (SPI2, PD10 CS, 20 MHz)
 * - External SPI (SPI3, PB3 SCK, PB4 MISO, PB5 MOSI, PD7 CS, 10 MHz, 5V via TXS0108ERGYR)  // FIXED: CS on PD7
 * - External I2C (I2C1, PB6 SCL, PB7 SDA, 400 kHz, 5V via TXS0108ERGYR)
 * - Dual WS2812B LEDs (PE14, TIM1)
 * - UART4 (PC10/PC11, debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5 (PC12/PD2, sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V, /dev/ttyS0, 57600 baud)  // UPDATED: Pins to PD3-PD6 per pin table
 * - UART7 (PE7/PE8, CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - UART3 (PD8/PD9, SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent: 0 normal, 1 inverted)
 * - CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1), CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1)  // UPDATED: CAN1 to PD0/PD1 per pin table
 * - USB OTG FS (PA9 VBUS, PA11 DM, PA12 DP, no power/overcurrent pins)
 * No PX4IO co-processor. I2C1 and SPI3 use TXS0108ERGYR for 5V level translation.
 */


#include <nuttx/spi/spi.h>
#include <px4_platform_common/px4_manifest.h>
#include <px4_platform_common/defines.h>  // For SPIDEV_FLASH

// FM25V01A-GTR on SPI2: 128 KB, emulated as 4096 blocks of 32 bytes
static const px4_mft_device_t spi2_fram = {
    .bus_type = px4_mft_device_t::SPI,
    .devid    = SPIDEV_FLASH(0)
};

static const px4_mtd_entry_t fmum_fram = {
    .device = &spi2_fram,
    .npart = 1,
    .partd = {
        {
            .type = MTD_PARAMETERS,
            .path = "/fs/mtd_params",
            .nblocks = (131072 / (1 << CONFIG_RAMTRON_EMULATE_SECTOR_SHIFT))  // 128 KB
        }
    },
};

static const px4_mtd_manifest_t board_mtd_config = {
    .nconfigs = 1,
    .entries = {
        &fmum_fram
    }
};

static const px4_mft_entry_s mtd_mft = {
    .type = MTD,
    .pmft = (void *)&board_mtd_config,
};

static const px4_mft_s mft = {
    .nmft = 1,
    .mfts = {
        &mtd_mft
    }
};

const px4_mft_s *board_get_manifest(void)
{
    return &mft;
}
