/** Copyright © 2021 The Things Industries B.V.
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
 * @file main.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "BSP.h"
#include "stm32_seq.h"
#include "sys_app.h"
#include "lora_app.h"

static BSP_config_t bsp_config;

static void MX_LoRaWAN_Init(void)
{
    SystemApp_Init();
    LoRaWAN_Init();
}

static void MX_LoRaWAN_Process(void)
{
    UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
}

int main(void)
{
    BSP_get_default_config(&bsp_config);

    // No shield connected
    bsp_config.skip_shield_init = true;
    bsp_config.high_performance = true;
    bsp_config.disable_watchdog = true;

    if (BSP_init(&bsp_config)!=BSP_ERROR_NONE) {
        while (1) {
            __WFI();
        }
    }

    UAIR_LPM_SetStopMode((1 << UAIR_LPM_APP), UAIR_LPM_DISABLE);

    MX_LoRaWAN_Init();
    while (1)
    {
        MX_LoRaWAN_Process();
    }
}
