/* boards/px4/pixeagle/src/oled_debug.c
 * Simple shim used by board init to avoid link errors.
 */

#include <syslog.h>
#include <stdio.h>

/* Keep C linkage and simple API used by init.c */
void oled_debug_init(void)
{
    /* Optionally initialize a GPIO or mark that debug is available */
    syslog(LOG_INFO, "[OLED DEBUG] init\n");
}

void oled_debug_step(int step, const char *msg)
{
    if (msg) {
        syslog(LOG_INFO, "[OLED DEBUG] step %d: %s\n", step, msg);
    } else {
        syslog(LOG_INFO, "[OLED DEBUG] step %d\n", step);
    }
}

void oled_debug_val(const char *label, int val)
{
    if (label) {
        syslog(LOG_INFO, "[OLED DEBUG] %s: %d\n", label, val);
    } else {
        syslog(LOG_INFO, "[OLED DEBUG] val: %d\n", val);
    }
}
