/** Copyright (c) 2021 MAIS Project
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
 * @file UAIR_BSP_bm.h
 *
 * @copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_BM__
#define UAIR_BSP_BM__

#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_adc.h"
#include "UAIR_BSP_error.h"
#include "UAIR_BSP_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ADC_HandleTypeDef UAIR_BSP_voltage_adc;

#define VBAT_ADC ADC
#define VBAT_ADC_RES                         ADC_RESOLUTION_12B
#define VBAT_ADC_CHANNEL                     ADC_CHANNEL_VBAT 
#define VREF_ADC_CHANNEL                     ADC_CHANNEL_VREFINT

BSP_error_t UAIR_BSP_BM_Init(void);

BSP_error_t UAIR_BSP_BM_EnableBatteryRead();
BSP_error_t UAIR_BSP_BM_DisableBatteryRead();

BSP_error_t UAIR_BSP_BM_PrepareAcquisition(void);
BSP_error_t UAIR_BSP_BM_EndAcquisition(void);

int32_t UAIR_BSP_BM_DeInit(void);

int32_t UAIR_BSP_BM_ConfChannel(uint32_t channel);
uint32_t UAIR_BSP_BM_ReadChannel(void);

#ifdef __cplusplus
}
#endif

#endif
