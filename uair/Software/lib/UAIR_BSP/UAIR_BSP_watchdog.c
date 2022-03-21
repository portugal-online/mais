/*
 * Copyright (C) 2021, 2022 MAIS Project
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
 * @file UAIR_BSP_watchdog.c
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_CORE
 *
 * uAir watchdog interface header
 *
 */

#include "UAIR_BSP_watchdog.h"
#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_iwdg.h"

static IWDG_HandleTypeDef iwdg;

#define WDG_MAX_RELOAD_SECONDS (int32_t)((4U*4095U)/250U)

BSP_error_t UAIR_BSP_watchdog_init(uint32_t seconds)
{
    BSP_error_t err;
    HAL_StatusTypeDef status;

    /*
     * IWDG is clocked by LSI divided by 128. This gives a frequency of 250Hz
     *
     * The divider used is 4, so IWDG frequency is 62.5Hz.
     *
     * Max reload is 4095
     */

    if (seconds <= WDG_MAX_RELOAD_SECONDS) {

        iwdg.Instance  = IWDG;
        iwdg.Init.Prescaler = IWDG_PRESCALER_4;
        iwdg.Init.Window = IWDG_WINDOW_DISABLE;
        iwdg.Init.Reload = (seconds * 250U)>>2;

        status = HAL_IWDG_Init(&iwdg);

        if (status != HAL_OK)
        {
            BSP_error_set(ERROR_ZONE_IWDG, BSP_ERROR_TYPE_IWDG_INIT, 0, status);
            err = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
            err = BSP_ERROR_NONE;
        }

    } else
    {
        err = BSP_ERROR_WRONG_PARAM;
    }

    return err;
}

/**
 * @brief kick watchdog
 * @ingroup UAIR_BSP_CORE
 *
 *
 * Kick the internal watchdog.
 *
 */
void UAIR_BSP_watchdog_kick(void)
{
    HAL_StatusTypeDef status;

    status = HAL_IWDG_Refresh(&iwdg);

    if (status != HAL_OK) {
        BSP_error_set(ERROR_ZONE_IWDG, BSP_ERROR_TYPE_IWDG_KICK, 0, status);
        BSP_FATAL();
    }
}

