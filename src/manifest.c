/**
 * @file manifest.c
 *
 * Pixeagle hardware manifest
 *
 * Defines the hardware manifest for the Pixeagle board (STM32H743VIT6, version V1C00):
 
 
 * - BMI088 (SPI1, PA4 CS_ACC, PB2 CS_GYR, PA5–PA7, 20 MHz)  // UPDATED: GYR CS to PB2
 * - ICM-42688-P (SPI4, PE4 CS, PE2/PE5/PE6, PE3 DRDY, 20 MHz)  // FIXED: CS on PE4, DRDY on PE3
 * - IST8310 (I2C3, PA8 SCL/PC9 SDA, 0x0E, 400 kHz)
 * - BMP388 (I2C3, PA8 SCL/PC9 SDA, 0x76, 400 kHz)
 * - BMP390 (I2C4, PB8 SCL/PB9 SDA, 0x76, 400 kHz)
 * - MicroSD (SPI2, PB11 CS, PB10 SCK, PB14 MISO, PB15 MOSI, 10 MHz)
 * - FM25V01A-GTR (SPI2, PD10 CS, 20 MHz)
 * - External SPI (SPI3, PB3 SCK, PB4 MISO, PB5 MOSI, PD7 CS, 10 MHz, 5V via TXS0108ERGYR)  // FIXED: CS on PD7
 * - External I2C (I2C1, PB6 SCL, PB7 SDA, 400 kHz, 5V via TXS0108ERGYR)
 * - Dual WS2812B LEDs (PE14, TIM1)
 * - UART4 (PC10/PC11, debug, 5V, /dev/ttyS3, 115200 baud)
 * - UART5 (PC12/PD2, sensor module, 5V, /dev/ttyS2, 115200 baud)
 * - USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry, 5V, /dev/ttyS0, 57600 baud)  // UPDATED: Pins to PD3-PD6
 * - UART7 (PE7/PE8, CM4/ESP32, 5V, /dev/ttyS4, 921600 baud)
 * - UART3 (PD8/PD9, SBUS/PPM, auto-detect, RC_SBUS_INV device-dependent: 0 normal, 1 inverted)
 * - CAN1 (PD0/PD1, 5V via TCAN1044VDRQ1)  // UPDATED: Pins to PD0/PD1
 * - CAN2 (PB12/PB13, 5V via TCAN1044VDRQ1)
 * - USB OTG FS (PA9 VBUS, PA11 DM, PA12 DP, no power/overcurrent pins)
 * No PX4IO co-processor. I2C1 and SPI3 use TXS0108ERGYR for 5V level translation.
 * All components are onboard, present, and mandatory.
 */

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
    uint32_t                hw_ver_rev; /* Version and revision ID */
    const px4_hw_mft_item_t *mft;       /* Pointer to manifest items */
    uint32_t                entries;    /* Number of manifest items */
} 
px4_hw_mft_list_entry_t;

typedef px4_hw_mft_list_entry_t *px4_hw_mft_list_entry;
#define px4_hw_mft_list_uninitialized (px4_hw_mft_list_entry) -1

/* Define component IDs for Pixeagle */
typedef enum {
    HW_MFT_BMI088,         /* BMI088 IMU on SPI1 */
    HW_MFT_ICM42688P,      /* ICM-42688-P IMU on SPI4 */
    HW_MFT_IST8310,        /* IST8310 magnetometer on I2C3 */
    HW_MFT_BMP388,         /* BMP388 barometer on I2C3 */
    HW_MFT_BMP390,         /* BMP390 barometer on I2C4 */
    HW_MFT_FM25V01A,       /* FM25V01A-GTR FRAM on SPI2 */
    HW_MFT_MICROSD,        /* MicroSD on SPI2 */
    HW_MFT_WS2812B,        /* Dual WS2812B LEDs on PE14 (TIM1) */
    HW_MFT_CAN1,           /* CAN1 on PD0/PD1 (TCAN1044VDRQ1) */  // UPDATED: Pins
    HW_MFT_CAN2,           /* CAN2 on PB12/PB13 (TCAN1044VDRQ1) */
    HW_MFT_COUNT           /* Total number of components */
} 
px4_hw_mft_item_id_t;

/* Define Pixeagle hardware version/revision */
#define V1C00 0x10000  /* Version 1, Revision 0 */

/* Pixeagle hardware manifest (all components onboard and mandatory) */
static const px4_hw_mft_item_t 				hw_mft_list_v1c00[] = {
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* BMI088 */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* ICM-42688-P */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* IST8310 */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* BMP388 */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* BMP390 */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* FM25V01A-GTR */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* MicroSD */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* WS2812B LEDs */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* CAN1 */
    { .present = 1, .mandatory = 1, .connection = px4_hw_con_onboard }, /* CAN2 */
};

static px4_hw_mft_list_entry_t 				mft_lists[] = {
    { V1C00, hw_mft_list_v1c00, arraySize(hw_mft_list_v1c00) }, /* Pixeagle Rev 0 */
};

/* Default to unsupported device if no manifest matches */
static const px4_hw_mft_item_t 				device_unsupported = { 0, 0, 0 };

/************************************************************************************
 * Name: board_query_manifest
 *
 * Description:
 *   Returns the manifest item for a given component ID.
 *
 * Input Parameters:
 *   id - The component ID (px4_hw_mft_item_id_t) to retrieve.
 *
 * Returned Value:
 *   Pointer to the manifest item, or device_unsupported if not found.
 *
 ************************************************************************************/

__EXPORT px4_hw_mft_item 			board_query_manifest(px4_hw_mft_item_id_t id)
{
    static px4_hw_mft_list_entry 			boards_manifest = px4_hw_mft_list_uninitialized;

    /* Initialize manifest on first call */
    if (boards_manifest == px4_hw_mft_list_uninitialized) {
        uint32_t ver_rev = board_get_hw_version() << 16;
        ver_rev |= board_get_hw_revision();

        for (unsigned i = 0; i < arraySize(mft_lists); i++) {
            if (mft_lists[i].hw_ver_rev == ver_rev) {
                boards_manifest = &mft_lists[i];
                break;
            }
        }

        if (boards_manifest == px4_hw_mft_list_uninitialized) {
            syslog(LOG_ERR, "[boot] Pixeagle board %08" PRIx32 " is not supported!\n", ver_rev);
        }
    }

    /* Return manifest item or unsupported */
    px4_hw_mft_item rv = &device_unsupported;

    if (boards_manifest != px4_hw_mft_list_uninitialized && id < boards_manifest->entries) {
        rv = &boards_manifest->mft[id];
    }

    return rv;
}