#ifndef HW_RTC_H__
#define HW_RTC_H__

#include <inttypes.h>

#ifdef __cplusplus
#include "csignal.hpp"

CSignal<uint32_t, uint32_t> &rtc_timer_signal();
#endif

#ifdef __cplusplus
extern "C" {
#endif

void rtc_engine_init(void);
void rtc_engine_deinit(void);
uint32_t rtc_engine_get_counter(void);
uint32_t rtc_engine_get_ticks();
void rtc_engine_set_alarm_a(uint32_t counter);
void rtc_engine_set_alarm_a_enable(int enabled);
void rtc_engine_enable();
uint32_t rtc_engine_get_second_counter();
void rtc_enable_progress();

#ifdef __cplusplus
}
#endif

#endif
