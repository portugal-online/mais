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
 * @file UAIR_BSP_clk_timer.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#include "BSP.h"
#include "UAIR_BSP_clk_timer.h"
#include "pvt/UAIR_BSP_clk_timer_p.h"
#include <stdbool.h>

TIM_HandleTypeDef UAIR_BSP_buzzer_timer;
RTC_HandleTypeDef UAIR_BSP_rtc;
IWDG_HandleTypeDef UAIR_BSP_iwdg;
LPTIM_HandleTypeDef UAIR_BSP_lptim = {0};
static volatile bool lptim_running = false;

BSP_error_t UAIR_BSP_LPTIM_Init(void)
{
    //LPTIM_InitTypeDef init = {0};
    UAIR_BSP_lptim.Instance = LPTIM1;
    UAIR_BSP_lptim.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
    UAIR_BSP_lptim.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32; // 1.333333us per tick
    UAIR_BSP_lptim.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
    UAIR_BSP_lptim.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
    UAIR_BSP_lptim.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
    UAIR_BSP_lptim.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
    UAIR_BSP_lptim.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
    UAIR_BSP_lptim.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
    UAIR_BSP_lptim.Init.RepetitionCounter = 0;

    HAL_StatusTypeDef r = HAL_LPTIM_Init(&UAIR_BSP_lptim);
    if (r!=HAL_OK) {
        BSP_TRACE("LPTIM_Init: HAL error %d", r);
        return BSP_ERROR_NO_INIT;
    }
    lptim_running = false;
    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_LPTIM_count(uint32_t period)
{
    lptim_running = true;

    //BSP_TRACE("LPTIM: pre-start state %d", HAL_LPTIM_GetState(&UAIR_BSP_lptim));
    HAL_StatusTypeDef r = HAL_LPTIM_OnePulse_Start_IT(&UAIR_BSP_lptim, period, period/2);
    //BSP_TRACE("LPTIM: start state %d", HAL_LPTIM_GetState(&UAIR_BSP_lptim));
    if (r==HAL_OK) {
        return BSP_ERROR_NONE;
    }
    lptim_running = false;
    //BSP_TRACE("LPTIM: HAL error %d %d", r, HAL_LPTIM_GetState(&UAIR_BSP_lptim));
    return BSP_ERROR_PERIPH_FAILURE;
}

void UAIR_BSP_LPTIM_wait(void)
{
    do {
        __disable_irq();
        if (!lptim_running) {
            __enable_irq();
            break;
        } else {
            //__NOP();
            __WFI();
        }
        __enable_irq();
    } while (1);
    HAL_LPTIM_OnePulse_Stop_IT(&UAIR_BSP_lptim);
}

void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *h)
{
    lptim_running = false;
}

void  HAL_LPTIM_UpdateEventCallback(LPTIM_HandleTypeDef *h)
{
    //BSP_TRACE("Update");
}

BSP_error_t BSP_delay_us(unsigned us)
{
    return UAIR_BSP_LPTIM_delay(us);
}


// Minimum 100us

BSP_error_t UAIR_BSP_LPTIM_delay(unsigned us)
{
    BSP_error_t err;

    if (us<100) {
        us = 100;
    }
    us -= 52;
    us<<=8;
    us/=341;

    err = UAIR_BSP_LPTIM_count(us);
    if (err==BSP_ERROR_NONE) {
        UAIR_BSP_LPTIM_wait();
    }
    return err;
};



BSP_error_t UAIR_BSP_RTC_Init(void)
{
  RTC_AlarmTypeDef sAlarm = {0};

  UAIR_BSP_rtc.Instance = RTC;
  UAIR_BSP_rtc.Init.AsynchPrediv = RTC_PREDIV_A;
  UAIR_BSP_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  UAIR_BSP_rtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  UAIR_BSP_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  UAIR_BSP_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  UAIR_BSP_rtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  UAIR_BSP_rtc.Init.BinMode = RTC_BINARY_ONLY;
  if (HAL_RTC_Init(&UAIR_BSP_rtc) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }

  // Initialize RTC and set the Time and Date
  if (HAL_RTCEx_SetSSRU_IT(&UAIR_BSP_rtc) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }
  // Enable the Alarm A
  sAlarm.BinaryAutoClr = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDBINMASK_NONE;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&UAIR_BSP_rtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Configures independent watchdog timer
  * @note   The UAIR_BSP_IWDG_Refresh should be called every time before the timer expires
  * @arg    iwdg_reload: Timeout (in ticks of the IWDG timer) for the IWDG (max is 0xFFF)
  * @return UAIR_BSP status
  */
int32_t UAIR_BSP_IWDG_Init(uint32_t iwdg_reload)
{
  /* Set IWDG settings to maximum reload value */
  UAIR_BSP_iwdg.Instance = IWDG;
  UAIR_BSP_iwdg.Init.Prescaler = IWDG_PRESCALER_256;
  UAIR_BSP_iwdg.Init.Window = IWDG_WINR_WIN;
  UAIR_BSP_iwdg.Init.Reload = iwdg_reload;

  if (HAL_IWDG_Init(&UAIR_BSP_iwdg) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  Refreshes the watchdog timer counter
  * @return None
  */
void UAIR_BSP_IWDG_Refresh(void)
{
  HAL_IWDG_Refresh(&UAIR_BSP_iwdg);
}


