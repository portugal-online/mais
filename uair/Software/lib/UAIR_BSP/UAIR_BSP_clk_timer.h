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
 * @file UAIR_BSP_clk_timer.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_CLK_TIMER_H__
#define UAIR_BSP_CLK_TIMER_H__

#include "stm32wlxx_hal.h"
#include "UAIR_BSP_error.h"
#include "UAIR_BSP_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t BSP_delay_us(unsigned us);


int32_t UAIR_BSP_IWDG_Init(uint32_t iwdg_reload);
void UAIR_BSP_IWDG_Refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* UAIR_BSP_CLK_TIMER_H */
