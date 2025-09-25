/**
 * @file can.c
 *
 * Pixeagle-specific CAN functions
 *
 * This file configures dual CAN FD interfaces for the Pixeagle board, based on
 * STM32H743VIT6, with BMI088 (SPI1), ICM-42688-P (SPI4), IST8310 (I2C3, 0x0E),
 * BMP388 (I2C3, 0x76), BMP390 (I2C4, 0x76), FM25V01A-GTR (SPI2), MicroSD (SPI2),
 * dual WS2812B LEDs (PE14), UART4 (PC10/PC11, debug, 5V), UART5 (PC12/PD2, sensor module, 5V),
 * USART2 (PD5 TX/PD6 RX, PD3 CTS/PD4 RTS, telemetry with flow control, 5V), UART7 (PE7/PE8, CM4/ESP32, 5V).
 * Supports dual CAN FD lines: CAN1 (PD0/PD1), CAN2 (PB12/PB13), both using  // UPDATED: CAN1 pins
 * TCAN1044VDRQ1 5V transceivers for external CAN-enabled modules and sensors.
 * Registers CAN1 as /dev/can0 and CAN2 as /dev/can1.
 */

#include <errno.h>
#include <debug.h>

#include <nuttx/can/can.h>
#include <arch/board/board.h>

#include "chip.h"
#include "arm_internal.h"
#include "stm32_can.h"
#include "board_config.h"

#ifdef CONFIG_CAN

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/* Configuration ********************************************************************/

#if !defined(CONFIG_STM32_CAN1) && !defined(CONFIG_STM32_CAN2)
#  warning "CAN FD support is enabled but not configured for Pixeagle. Define CONFIG_STM32_CAN1 (PD0/PD1) and/or CONFIG_STM32_CAN2 (PB12/PB13) in board_config.h."
#endif

/* Define CAN ports for Pixeagle */
#ifdef CONFIG_STM32_CAN1
#  define CAN1_PORT 1  // CAN1 on PD0 (CANRX), PD1 (CANTX), 5V via TCAN1044VDRQ1  // UPDATED: Pins
#endif
#ifdef CONFIG_STM32_CAN2
#  define CAN2_PORT 2  // CAN2 on PB12 (CANRX), PB13 (CANTX), 5V via TCAN1044VDRQ1
#endif

/************************************************************************************
 * Private Functions
 ************************************************************************************/

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/**
 * @brief Initialize CAN FD interfaces for Pixeagle
 *
 * Initializes dual CAN FD interfaces (CAN1 on PD0/PD1, CAN2 on PB12/PB13) using  // UPDATED: CAN1 pins
 * TCAN1044VDRQ1 5V transceivers. Registers CAN1 as /dev/can0 and CAN2 as /dev/can1.
 * Requires CONFIG_STM32_CAN1 and/or CONFIG_STM32_CAN2 defined in board_config.h
 * with appropriate pin assignments and TCAN1044VDRQ1 transceivers for 5V signaling.
 *
 * @return OK on success, or negative error code on failure.
 */
int can_devinit(void)
{
    static bool 	initialized 		  = false;
    struct 			can_dev_s 		*can1 = NULL;
    struct 			can_dev_s 		*can2 = NULL;
    int 			ret;

    /* Check if we have already initialized */
    if (initialized) {        return OK;    }

    /* Ensure at least one CAN port is configured */
#if !defined(CONFIG_STM32_CAN1) && !defined(CONFIG_STM32_CAN2)
    canerr("ERROR: No CAN FD ports enabled for Pixeagle. Define CONFIG_STM32_CAN1 and/or CONFIG_STM32_CAN2 in board_config.h.\n");
    return -ENODEV;
#endif

    /* Initialize CAN1 (PD0/PD1) if enabled */  // UPDATED: Pins
#ifdef CONFIG_STM32_CAN1
    can1 = stm32_caninitialize(CAN1_PORT);
    if (can1 == NULL) {
        canerr("ERROR: Failed to initialize CAN1 interface (PD0/PD1)\n");
        return -ENODEV;
    }
    /* Register CAN1 driver at /dev/can0 */
    ret = can_register("/dev/can0", can1);
    if (ret < 0) {
        canerr("ERROR: can_register failed for CAN1: %d\n", ret);
        return ret;
    }
#endif

    /* Initialize CAN2 (PB12/PB13) if enabled */
#ifdef CONFIG_STM32_CAN2
    can2 = stm32_caninitialize(CAN2_PORT);
    if (can2 == NULL) {
        canerr("ERROR: Failed to initialize CAN2 interface (PB12/PB13)\n");
        return -ENODEV;
    }
    /* Register CAN2 driver at /dev/can1 */
    ret = can_register("/dev/can1", can2);
    if (ret < 0) {
        canerr("ERROR: can_register failed for CAN2: %d\n", ret);
        return ret;
    }
#endif

    /* Mark as initialized */
    initialized = true;

    return OK;
}

#endif /* CONFIG_CAN */