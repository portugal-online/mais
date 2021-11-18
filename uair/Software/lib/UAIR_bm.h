/** Copyright © 2021 The Things Industries B.V.
 * Copyright © 2021 MAIS Project
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
 * @file UAIR_bm.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef UAIR_BM_H
#define UAIR_BM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <inttypes.h>

/**
 * Battery Monitor return types
 */
typedef enum
{
    BM_OP_SUCCESS = 0,
    BM_OP_FAIL = 1,
} BM_op_result_t;


/*
 * Reference calibration value if we don't use VREFINT_CAL_ADDR production calibration value
 * Calibration value reference temperature is 30'c
 */
#define UAIR_BM_VREFINT_CAL (VREFINT_CAL_VREF * 1510UL)

/*
 * RP605Z283B Battery Monitor division ratio of the battery voltage
 * The converted ADC value represents the battery voltage divided by the ratio
 */
#define UAIR_BM_OUTPUT_DIVISION_RATIO 3U

uint16_t UAIR_BM_GetInternalRefVoltage(void);
uint16_t UAIR_BM_GetBatteryVoltage(void);
BM_op_result_t UAIR_BM_Init(void);
BM_op_result_t UAIR_BM_DeInit(void);
bool UAIR_BM_OnBattery(void);

#ifdef __cplusplus
}
#endif

#endif /*UAIR_BM_H*/
