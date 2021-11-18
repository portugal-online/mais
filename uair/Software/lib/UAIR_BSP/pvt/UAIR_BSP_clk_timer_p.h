#ifndef UAIR_BSP_CLK_TIMER_P_H
#define UAIR_BSP_CLK_TIMER_P_H

#include "UAIR_BSP_error.h"

extern RTC_HandleTypeDef UAIR_BSP_rtc;
extern IWDG_HandleTypeDef UAIR_BSP_iwdg;

BSP_error_t UAIR_BSP_RTC_Init(void);
BSP_error_t UAIR_BSP_LPTIM_Init(void);

#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1 << RTC_N_PREDIV_S) - 1)
#define RTC_PREDIV_A ((1 << (15 - RTC_N_PREDIV_S)) - 1)

BSP_error_t UAIR_BSP_LPTIM_count(uint32_t period);
void UAIR_BSP_LPTIM_wait(void);
BSP_error_t UAIR_BSP_LPTIM_delay(unsigned us);

#endif
