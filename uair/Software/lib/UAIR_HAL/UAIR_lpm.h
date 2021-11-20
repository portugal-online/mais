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
 * @file UAIR_lpm.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_LPM_H
#define UAIR_LPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/**
 * @brief macro used to enter the critical section
 */
#define UAIR_LPM_ENTER_CRITICAL_SECTION( )    uint32_t primask_bit= __get_PRIMASK();\
  __disable_irq()

/**
 * @brief macro used to exit the critical section
 */
#define UAIR_LPM_EXIT_CRITICAL_SECTION( )     __set_PRIMASK(primask_bit)

/**
 * @brief value used to reset the LPM mode
 */
#define UAIR_LPM_NO_BIT_SET   (0UL)

/**
 * @brief type definition to represent the bit mask of an LPM mode
 */
typedef uint32_t UAIR_LPM_bm_t;

/**
 * Supported requester to the UAIR Low Power Manager - can be increased up to 32
 * It lists a bit mapping of all user of the Low Power Manager
 */
typedef enum
{
  UAIR_LPM_UART_TRACER,
  UAIR_LPM_I2C_SENSORS,
  UAIR_LPM_TIM_BUZZER,
  UAIR_LPM_ADC_BM,
  UAIR_LPM_LIB,
  UAIR_LPM_LTIM,
  UAIR_LPM_APP,
} UAIR_LPM_Id_t;

/**
 * @brief type definition to represent value of an LPM mode
 */
typedef enum
{
  UAIR_LPM_DISABLE=0,
  UAIR_LPM_ENABLE,
} UAIR_LPM_State_t;

/**
 * @brief type definition to represent the different type of LPM mode
 */

typedef enum
{
  UAIR_LPM_SLEEP_ONLY_MODE,
  UAIR_LPM_SLEEP_DEBUG_MODE,
  UAIR_LPM_SLEEP_STOP_MODE,
  UAIR_LPM_SLEEP_STOP_DEBUG_MODE,
} UAIR_LPM_Mode_t;

void UAIR_LPM_Init(UAIR_LPM_Mode_t init_mode);
void UAIR_LPM_DeInit(void);

void UAIR_LPM_Debugger_Enable(void);
void UAIR_LPM_Debugger_Disable(void);

void UAIR_LPM_PreStopModeHook(void);
void UAIR_LPM_PostStopModeHook(void);
void UAIR_LPM_PreSleepModeHook(void);
void UAIR_LPM_PostSleepModeHook(void);

void UAIR_LPM_SetStopMode(UAIR_LPM_bm_t lpm_id_bm, UAIR_LPM_State_t state);
UAIR_LPM_bm_t UAIR_LPM_GetStopMode(void);

void UAIR_LPM_EnterLowPower(void);

#ifdef __cplusplus
}
#endif

#endif /*UAIR_LPM_H*/
