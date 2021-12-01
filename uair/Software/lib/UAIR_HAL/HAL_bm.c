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
 * @file HAL_bm.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., MAIS Project
 *
 */

#include "HAL_bm.h"

static uint16_t battery_mV = 0;

/**
  * @brief Initialises the hardware for reading the battery voltage level
  * @param none
  * @return BM_op_result_t
  */
BM_op_result_t HAL_BM_Init(void)
{
    if (UAIR_BSP_BM_Init() != BSP_ERROR_NONE)
    {
        return BM_OP_FAIL;
    }
    return BM_OP_SUCCESS;
}

/**
  * @brief Deinitialises the hardware for reading the battery voltage level
  * @param none
  * @return BM_op_result_t
  */
BM_op_result_t HAL_BM_DeInit(void)
{
    if (UAIR_BSP_BM_DeInit() != BSP_ERROR_NONE)
    {
        return BM_OP_FAIL;
    }
    return BM_OP_SUCCESS;
}

/**
 * @brief Gets the MCU internal refernce voltage
 *
 * @return uint16_t of the calibrated reference voltage in millivolt (mv)
 */
uint16_t HAL_BM_GetInternalRefVoltage(void)
{
    uint16_t internal_ref_mV = 0;
    uint32_t adc_measurement = 0;

    UAIR_BSP_BM_ConfChannel(VREF_ADC_CHANNEL);

    adc_measurement = UAIR_BSP_BM_ReadChannel();
    if (adc_measurement != 0)
    {
        /**
             * We can use multiple methonds if the SOC is calibrated in production
             * (uint32_t)*VREFINT_CAL_ADDR
             * __LL_ADC_CALC_VREFANALOG_VOLTAGE(measuredLevel, ADC_RESOLUTION_12B)
             */
        internal_ref_mV = HAL_BM_VREFINT_CAL / adc_measurement;
    }
    return internal_ref_mV;
}

/**
 * @brief Gets the battery voltage level
 *
 * @return uint16_t of the battery voltage in millivolt (mv)
 */
uint16_t HAL_BM_GetBatteryVoltage(void)
{
    uint16_t internal_ref_mV = 0;
    uint32_t adc_measurement = 0;
    internal_ref_mV = HAL_BM_GetInternalRefVoltage();
    UAIR_BSP_BM_ConfChannel(VBAT_ADC_CHANNEL);
    adc_measurement = UAIR_BSP_BM_ReadChannel();
    if (adc_measurement != 0)
    {
        battery_mV = __LL_ADC_CALC_DATA_TO_VOLTAGE(internal_ref_mV, adc_measurement, VBAT_ADC_RES);
        battery_mV = battery_mV * HAL_BM_OUTPUT_DIVISION_RATIO;
    }
    return battery_mV;
}

bool HAL_BM_OnBattery(void)
{
    return battery_mV < 3100;
}
