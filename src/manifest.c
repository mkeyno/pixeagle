/****************************************************************************
 * boards/px4/pixeagle/src/manifest.c
 *
 * Pixeagle sensor hardware manifest
 *
 * Hardware revision encoding:
 *   board_get_hw_version()  returns 1  → stored in bits [31:16]
 *   board_get_hw_revision() returns 2  → stored in bits [15:0]
 *   ver_rev = (1 << 16) | 2 = 0x00010002
 *
 *   VER00 in board_config.h must be defined as:
 *     #define VER00  HW_VER_REV(1,2)
 *   or equivalently:
 *     #define VER00  0x00010002u
 *
 * Sensor manifest (indexed by px4_hw_mft_item_id_t):
 *   [0]  IMU 0   — ICM-42688-P  on SPI4
 *   [1]  IMU 1   — BMI088 Accel on SPI1
 *   [2]  IMU 2   — BMI088 Gyro  on SPI1
 *   [3]  BARO 0  — BMP388       on I2C3
 *   [4]  BARO 1  — BMP390       on I2C4
 *   [5]  MAG  0  — IST8310      on I2C3
 *
 * Reference:
 *   boards/px4/fmu-v5x/src/manifest.c  (same pattern, different sensor set)
 ****************************************************************************/

#include <nuttx/config.h>
#include <board_config.h>

#include <inttypes.h>
#include <stdbool.h>
#include <syslog.h>

#include "systemlib/px4_macros.h"

/****************************************************************************
 * Private type definitions (mirrors fmu-v5x pattern)
 ****************************************************************************/

typedef struct {
    uint32_t                 hw_ver_rev;
    const px4_hw_mft_item_t *mft;
    uint32_t                 entries;
} px4_hw_mft_list_entry_t;

typedef px4_hw_mft_list_entry_t *px4_hw_mft_list_entry;

#define px4_hw_mft_list_uninitialized ((px4_hw_mft_list_entry)-1)

/****************************************************************************
 * Fallback — returned for any device ID that is not in the manifest
 ****************************************************************************/

static const px4_hw_mft_item_t device_unsupported = {
    .present    = 0,
    .mandatory  = 0,
    .connection = 0,
};

/****************************************************************************
 * Pixeagle sensor manifest  (hardware revision VER00 = 0x00010002)
 *
 * IMPORTANT: The order of entries defines the device index (id) passed by
 * px4_platform_configure() when it calls board_query_manifest(id).
 * Do NOT reorder entries without updating all driver start scripts.
 ****************************************************************************/

static const px4_hw_mft_item_t hw_mft_list_pixeagle[] = {

    [0] = { /* IMU 0: ICM-42688-P on SPI4, CS=PE4, DRDY=PE3 */
        .present    = 1,
        .mandatory  = 1,
        .connection = px4_hw_con_onboard,
    },

    [1] = { /* IMU 1: BMI088 Accelerometer on SPI1, CS=PA4, DRDY=PC4 */
        .present    = 1,
        .mandatory  = 1,
        .connection = px4_hw_con_onboard,
    },

    [2] = { /* IMU 2: BMI088 Gyroscope on SPI1, CS=PB2, DRDY=PC5 */
        .present    = 1,
        .mandatory  = 1,
        .connection = px4_hw_con_onboard,
    },

    [3] = { /* BARO 0: BMP388 on I2C3 */
        .present    = 1,
        .mandatory  = 1,
        .connection = px4_hw_con_onboard,
    },

    [4] = { /* BARO 1: BMP390 on I2C4 */
        .present    = 1,
        .mandatory  = 1,
        .connection = px4_hw_con_onboard,
    },

    [5] = { /* MAG 0: IST8310 on I2C3 */
        .present    = 1,
        .mandatory  = 1,
        .connection = px4_hw_con_onboard,
    },
};

/****************************************************************************
 * Version → manifest mapping table
 *
 * Add additional entries here if you create new board revisions.
 * VER00 must equal (board_get_hw_version() << 16) | board_get_hw_revision()
 * = (1 << 16) | 2 = 0x00010002
 ****************************************************************************/

static px4_hw_mft_list_entry_t hw_mft_lists[] = {
    {
        .hw_ver_rev = VER00,
        .mft        = hw_mft_list_pixeagle,
        .entries    = arraySize(hw_mft_list_pixeagle),
    },
};

/****************************************************************************
 * board_query_manifest()
 *
 * Called by px4_platform_configure() to check whether a sensor at a given
 * manifest index is present and mandatory on this hardware revision.
 *
 * The function caches the matched manifest table on first call.
 ****************************************************************************/

/* Suppress the default macro before declaring the exported function */
#undef board_query_manifest

__EXPORT px4_hw_mft_item board_query_manifest(px4_hw_mft_item_id_t id)
{
    static px4_hw_mft_list_entry boards_manifest = px4_hw_mft_list_uninitialized;

    /* --- First-call initialisation: match board revision to manifest --- */
    if (boards_manifest == px4_hw_mft_list_uninitialized) {

        uint32_t ver_rev = ((uint32_t)board_get_hw_version()  & 0xFFFFu) << 16u
                         | ((uint32_t)board_get_hw_revision() & 0xFFFFu);

        syslog(LOG_INFO, "[MANIFEST] HW ver_rev=0x%08" PRIx32 " — searching manifest table\n",
               ver_rev);

        /* Safety: if hardware detection is broken, fall back to VER00 */
        if (ver_rev == 0xFFFFFFFFu) {
            syslog(LOG_WARNING, "[MANIFEST] HW detection failed — forcing VER00\n");
            ver_rev = VER00;
        }

        boards_manifest = NULL; /* not found yet */

        for (unsigned i = 0; i < arraySize(hw_mft_lists); i++) {
            if (hw_mft_lists[i].hw_ver_rev == ver_rev) {
                boards_manifest = &hw_mft_lists[i];
                syslog(LOG_INFO,
                       "[MANIFEST] Loaded manifest for ver_rev=0x%08" PRIx32
                       " (%lu sensors)\n",
                       ver_rev, boards_manifest->entries);
                break;
            }
        }

        if (boards_manifest == NULL) {
            syslog(LOG_ERR,
                   "[MANIFEST] No manifest match for ver_rev=0x%08" PRIx32
                   " — all sensors unsupported!\n", ver_rev);
            /* Leave boards_manifest = NULL so queries return device_unsupported */
        }
    }

    /* --- Return the item for the requested device id --- */
    if (boards_manifest != NULL && id < boards_manifest->entries) {
        const px4_hw_mft_item_t *item = &boards_manifest->mft[id];
        syslog(LOG_INFO,
               "[MANIFEST] id=%u  present=%d  mandatory=%d\n",
               id, item->present, item->mandatory);
        return item;
    }

    /* Device not in manifest — return unsupported stub */
    syslog(LOG_WARNING, "[MANIFEST] id=%u not in manifest — returning unsupported\n", id);
    return &device_unsupported;
}
