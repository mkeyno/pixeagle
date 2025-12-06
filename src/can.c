/****************************************************************************
 * boards/px4/pixeagle/src/can.c
 *
 * Pixeagle CAN bus initialization
 *
 ****************************************************************************/

#ifdef CONFIG_CAN

#include <errno.h>
#include <debug.h>

#include <nuttx/can/can.h>
#include <arch/board/board.h>

#include "chip.h"
#include "arm_internal.h"
#include "board_config.h"

// Forward declaration of FDCAN initialization function
struct can_dev_s;
extern struct can_dev_s *stm32_fdcaninitialize(int port);

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/* Configuration ********************************************************************/

/* Pixeagle has both FDCAN1 and FDCAN2 available:
 *   FDCAN1: PD0 (RX), PD1 (TX)
 *   FDCAN2: PB12 (RX), PB13 (TX)
 */

/************************************************************************************
 * Private Data
 ************************************************************************************/

static bool can1_initialized = false;
static bool can2_initialized = false;

/************************************************************************************
 * Public Functions
 ************************************************************************************/

/************************************************************************************
 * Name: can_devinit
 *
 * Description:
 *   All STM32 architectures must provide the following interface to work with
 *   examples/can.
 *
 ************************************************************************************/

int can_devinit(void)
{
	struct can_dev_s *can;
	int ret;

#ifdef CONFIG_STM32H7_FDCAN1
	/* Check if we have already initialized CAN1 */
	if (!can1_initialized) {
		/* Call stm32_fdcaninitialize() to get an instance of the FDCAN1 interface */
		can = stm32_fdcaninitialize(1);

		if (can == NULL) {
			canerr("ERROR: Failed to get FDCAN1 interface\n");
			return -ENODEV;
		}

		/* Register the CAN driver at "/dev/can0" */
		ret = can_register("/dev/can0", can);

		if (ret < 0) {
			canerr("ERROR: can_register FDCAN1 failed: %d\n", ret);
			return ret;
		}

		/* Now we are initialized */
		can1_initialized = true;
		caninfo("FDCAN1 initialized at /dev/can0\n");
	}
#endif

#ifdef CONFIG_STM32H7_FDCAN2
	/* Check if we have already initialized CAN2 */
	if (!can2_initialized) {
		/* Call stm32_fdcaninitialize() to get an instance of the FDCAN2 interface */
		can = stm32_fdcaninitialize(2);

		if (can == NULL) {
			canerr("ERROR: Failed to get FDCAN2 interface\n");
			return -ENODEV;
		}

		/* Register the CAN driver at "/dev/can1" */
		ret = can_register("/dev/can1", can);

		if (ret < 0) {
			canerr("ERROR: can_register FDCAN2 failed: %d\n", ret);
			return ret;
		}

		/* Now we are initialized */
		can2_initialized = true;
		caninfo("FDCAN2 initialized at /dev/can1\n");
	}
#endif

#if !defined(CONFIG_STM32H7_FDCAN1) && !defined(CONFIG_STM32H7_FDCAN2)
	canerr("ERROR: No CAN interfaces enabled in configuration\n");
	return -ENODEV;
#endif

	return OK;
}

#endif /* CONFIG_CAN */
