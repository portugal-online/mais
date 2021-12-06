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
 * @file UAIR_hal.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_HAL_H__
#define UAIR_HAL_H__

#include "stm32wlxx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "HAL_clk.h"
#include "HAL_gpio.h"
#include "HAL_bm.h"

typedef I2C_HandleTypeDef *HAL_I2C_bus_t;


/**
 * UAIR HAL return types
 */
typedef enum
{
    UAIR_HAL_OP_SUCCESS = 0,
    UAIR_HAL_OP_FAIL = 1,
} UAIR_HAL_op_result_t;

void UAIR_HAL_Init(void);
void UAIR_HAL_DeInit(void);

UAIR_HAL_op_result_t UAIR_HAL_SysClk_Init(bool lowpower);
void UAIR_HAL_Error_Handler(void);
void HAL_delay_us(unsigned);
bool UAIR_HAL_is_lowpower(void);
UAIR_HAL_op_result_t UAIR_HAL_request_high_performance(void);
void UAIR_HAL_release_high_performance(void);

void __attribute__((noreturn)) HAL_FATAL(void);

#ifdef __cplusplus
}
#endif

#endif /*UAIR_HAL_H*/
