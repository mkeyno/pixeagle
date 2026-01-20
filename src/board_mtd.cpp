/****************************************************************************
 * boards/px4/pixeagle/src/board_mtd.cpp
 *
 * Pixeagle FRAM MTD Configuration
 * Configures Ramtron FRAM on SPI2 for parameter and calibration storage
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <board_config.h>
#include <nuttx/spi/spi.h>
#include <px4_platform_common/px4_manifest.h>

/* SPI2 FRAM device - Ramtron FM25V01A (128 Kbit / 16 KByte)
 * Device ID: 7F 7F 7F 7F 7F 7F C2 21 08
 */
static const px4_mft_device_t spi2_fram = {
	.bus_type = px4_mft_device_t::SPI,
	.devid    = SPIDEV_FLASH(0)  /* SPI2, CS 0 - FRAM chip select */
};

/* FRAM partition configuration
 * Total Size: 128 Kbits = 16 KBytes (16,384 bytes)
 * Sector size: 32 bytes (CONFIG_RAMTRON_EMULATE_SECTOR_SHIFT=5)
 * Total blocks: 16,384 / 32 = 512 blocks
 *
 * Partitioning (50/50 split):
 * - Partition 0 (/fs/mtd_params):  256 blocks (8 KB)
 * - Partition 1 (/fs/mtd_caldata): 256 blocks (8 KB)
 */
static const px4_mtd_entry_t fram_entry = {
	.device = &spi2_fram,
	.npart = 2,
	.partd = {
		{
			.type = MTD_PARAMETERS,
			.path = "/fs/mtd_params",
			.nblocks = 256  /* 8KB / 32B = 256 blocks */
		},
		{
			.type = MTD_CALDATA,
			.path = "/fs/mtd_caldata",
			.nblocks = 256  /* 8KB / 32B = 256 blocks */
		}
	},
};

/* MTD manifest - FRAM Only (SD Card is a Block Device, not MTD) */
static const px4_mtd_manifest_t board_mtd_config = {
    .nconfigs   = 1,
    .entries = {
        &fram_entry
    }
};

/* MTD manifest entry */
static const px4_mft_entry_s mtd_mft = {
	.type = MTD,
	.pmft = (void *) &board_mtd_config,
};

/* Board manifest */
static const px4_mft_s mft = {
	.nmft = 1,
	.mfts = {
		&mtd_mft,
	}
};

/* Export the manifest - called by PX4 framework */
const px4_mft_s *board_get_manifest(void)
{
	return &mft;
}
