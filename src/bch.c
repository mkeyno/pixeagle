/****************************************************************************
 * boards/px4/pixeagle/src/bch.c
 *
 * Minimal stub for bchdev_register to satisfy linker when BCH is disabled.
 * This file intentionally does not enable or pull in the legacy BCH library.
 *
 * Place this file in: boards/px4/pixeagle/src/
 * Add it to the drivers_board target so it is linked into the board library.
 *
 ****************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <syslog.h>

/* Exported symbol expected by PX4/NuttX build:
 *
 *   int bchdev_register(const char *blockpath, const char *charpath, bool readonly);
 *
 * Provide a minimal implementation that returns -ENOSYS by default so the
 * linker is satisfied and no BCH objects are pulled in.  If you later want
 * to provide a small mapping from a character device name (e.g. "/dev/sdcard")
 * to the block device (e.g. "/dev/mmcsd0"), implement that logic here.
 */

int bchdev_register(const char *blockpath, const char *charpath, bool readonly)
{
    /* Silence unused parameter warnings */
    (void)blockpath;
    (void)charpath;
    (void)readonly;

    /* Log once to help debugging if this stub is ever invoked at runtime. */
    syslog(LOG_INFO, "bch_stub: bchdev_register called but BCH is disabled; returning -ENOSYS\n");

    /* Indicate "function not implemented" so callers can fall back. */
    return -ENOSYS;
}
