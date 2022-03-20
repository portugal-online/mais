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
 * @defgroup UAIR_BSP_SENSOR_EXTERNAL_TEMP uAir External Temperature/Humidity Sensor interface
 * @ingroup UAIR_BSP_SENSORS
 *
 * uAir interfacing to external temperature/humidity sensor.
 *
 * Usage of the sensor should be as follows:
 *
 * - Ensure sensor is ready with \ref BSP_external_temp_hum_get_sensor_state()
 * - Call \ref BSP_external_temp_hum_start_measure()
 * - Wait for \ref BSP_external_temp_hum_get_measure_delay_us()
 * - Extract values with \ref BSP_external_temp_hum_read_measure() after measurement completes 
 *
 */

/**
 * @file UAIR_BSP_externaltemp.h
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_EXTERNAL_TEMP
 *
 * uAir interfacing to external temperature/humidity sensor header
 *
 */

#ifndef UAIR_BSP_EXTERNALTEMP_H__
#define UAIR_BSP_EXTERNALTEMP_H__

#include "BSP.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned int BSP_external_temp_hum_get_measure_delay_us(void);
BSP_error_t BSP_external_temp_hum_start_measure(void);
BSP_error_t BSP_external_temp_hum_read_measure(int32_t *temp, int32_t *hum);
BSP_sensor_state_t BSP_external_temp_hum_get_sensor_state(void);

#ifdef __cplusplus
}
#endif

#endif
