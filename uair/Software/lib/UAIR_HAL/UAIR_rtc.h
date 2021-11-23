/** Copyright © 2021 The Things Industries B.V.
 *  Copyright © 2021 MAIS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file UAIR_rtc.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_RTC_H__
#define UAIR_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_timer.h"
#include "stm32_systime.h"

UTIL_TIMER_Status_t UAIR_RTC_Init(void);
UTIL_TIMER_Status_t UAIR_RTC_StartTimer(uint32_t timeout);
UTIL_TIMER_Status_t UAIR_RTC_StopTimer(void);
uint32_t UAIR_RTC_SetTimerContext(void);
uint32_t UAIR_RTC_GetTimerContext(void);
uint32_t UAIR_RTC_GetTimerElapsedTime(void);
uint32_t UAIR_RTC_GetTimerValue(void);
uint32_t UAIR_RTC_GetMinimumTimeout(void);
void UAIR_RTC_DelayMs(uint32_t delay);
uint32_t UAIR_RTC_Convert_ms2Tick(uint32_t timeMilliSec);
uint32_t UAIR_RTC_Convert_Tick2ms(uint32_t tick);
uint32_t UAIR_RTC_GetTime(uint16_t *subSeconds);
void UAIR_RTC_BkUp_Write_Seconds(uint32_t Seconds);
uint32_t UAIR_RTC_BkUp_Read_Seconds(void);
void UAIR_RTC_BkUp_Write_SubSeconds(uint32_t SubSeconds);
uint32_t UAIR_RTC_BkUp_Read_SubSeconds(void);

#ifdef __cplusplus
}
#endif

#endif /*UAIR_RTC_H*/
