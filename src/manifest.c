/****************************************************************************
 * boards/px4/pixeagle/src/manifest.c
 *
 * Pixeagle hardware manifest - defines onboard components
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <board_config.h>

#include <inttypes.h>
#include <stdbool.h>
#include <syslog.h>

#include "systemlib/px4_macros.h"

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/

typedef struct {
	uint32_t                hw_ver_rev; /* the version and revision */
	const px4_hw_mft_item_t *mft;       /* The first entry */
	uint32_t                entries;    /* the length of the list */
} px4_hw_mft_list_entry_t;

typedef px4_hw_mft_list_entry_t *px4_hw_mft_list_entry;
#define px4_hw_mft_list_uninitialized (px4_hw_mft_list_entry) -1

static const px4_hw_mft_item_t device_unsupported = {0, 0, 0};

/****************************************************************************
 * Pixeagle Hardware Manifest
 * 
 * Lists all onboard sensors and peripherals
 ****************************************************************************/

// List of components on the Pixeagle board configuration
// The index of those components is given by the enum (px4_hw_mft_item_id_t)
// declared in board_common.h

static const px4_hw_mft_item_t hw_mft_list_pixeagle[] = {
	{                                      // IMU 0: ICM-42688-P on SPI4
		.present     = 1,
		.mandatory   = 1,
		.connection  = px4_hw_con_onboard,
	},
	{                                      // IMU 1: BMI088 Accel on SPI1
		.present     = 1,
		.mandatory   = 1,
		.connection  = px4_hw_con_onboard,
	},
	{                                      // IMU 2: BMI088 Gyro on SPI1
		.present     = 1,
		.mandatory   = 1,
		.connection  = px4_hw_con_onboard,
	},
	{                                      // Barometer 0: BMP388 on I2C3
		.present     = 1,
		.mandatory   = 1,
		.connection  = px4_hw_con_onboard,
	},
	{                                      // Barometer 1: BMP390 on I2C4
		.present     = 1,
		.mandatory   = 1,
		.connection  = px4_hw_con_onboard,
	},
	{                                      // Magnetometer 0: IST8310 on I2C3
		.present     = 1,
		.mandatory   = 1,
		.connection  = px4_hw_con_onboard,
	},
};

static px4_hw_mft_list_entry_t mft_lists[] = {
//  ver_rev                  manifest                        entries
	{VER00, hw_mft_list_pixeagle, arraySize(hw_mft_list_pixeagle)},
};

/************************************************************************************
 * Name: board_query_manifest
 *
 * Description:
 *   Optional returns manifest item.
 *
 * Input Parameters:
 *   manifest_id - the ID for the manifest item to retrieve
 *
 * Returned Value:
 *   0 - item is not in manifest => assume legacy operations
 *   pointer to a manifest item
 *
 ************************************************************************************/

// Undefine the macro version to allow our function implementation
#undef board_query_manifest

__EXPORT px4_hw_mft_item board_query_manifest(px4_hw_mft_item_id_t id)
{
	static px4_hw_mft_list_entry boards_manifest = px4_hw_mft_list_uninitialized;

	if (boards_manifest == px4_hw_mft_list_uninitialized) {
		uint32_t ver_rev = ((uint32_t)board_get_hw_version() & 0xFFFF) << 16;
		ver_rev |= ((uint32_t)board_get_hw_revision() & 0xFFFF);

		for (unsigned i = 0; i < arraySize(mft_lists); i++) {
			if (mft_lists[i].hw_ver_rev == ver_rev) {
				boards_manifest = &mft_lists[i];
				syslog(LOG_INFO, "[boot] Pixeagle hardware manifest loaded (ver_rev: %08" PRIx32 ")\n", ver_rev);
				break;
			}
		}

		if (boards_manifest == px4_hw_mft_list_uninitialized) {
			syslog(LOG_ERR, "[boot] Pixeagle board %08" PRIx32 " is not supported!\n", ver_rev);
		}
	}

	px4_hw_mft_item rv = &device_unsupported;

	if (boards_manifest != px4_hw_mft_list_uninitialized &&
	    id < boards_manifest->entries) {
		rv = &boards_manifest->mft[id];
	}

	return rv;
}