#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void tone_alarm_gpio_beep(uint16_t frequency, uint16_t duration_ms);

#ifdef __cplusplus
}
#endif
