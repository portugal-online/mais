#ifndef LL_RTC_H__
#define LL_RTC_H__

//#define RTC_FLAG_ALRAF (1<<0)
#define RTC_SCR_CALRBF (1<<0)

#define RTC_SR_ALRAF_Pos (0)

uint32_t LL_RTC_TIME_GetSubSecond(RTC_TypeDef*);

#endif
