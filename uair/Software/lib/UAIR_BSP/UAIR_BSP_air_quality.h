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
 * @defgroup UAIR_BSP_SENSOR_AIR_QUALITY uAir Air Quality Sensor interface
 * @ingroup UAIR_BSP_SENSORS
 *
 * uAir interfacing to air quality sensor.
 *
 * Usage of the sensor should be as follows:
 *
 * - Ensure sensor is ready with \ref BSP_air_quality_get_sensor_state()
 * - Call \ref BSP_air_quality_start_measurement()
 * - Wait for \ref BSP_air_quality_get_measure_delay_us()
 * - Calculate OAQ values with \ref BSP_air_quality_calculate() after measurement completes \note
 *   \ref BSP_air_quality_measurement_completed() is internally used to confirm measurement was complete.
 * - Repeat the measurement after the vendor-supplied measurement-to-measurement delay (1998ms)
 *
 * It takes 901 samples for the initial OAQ values to be reported since the sensor needs stabilization. During this
 * period \ref BSP_air_quality_calculate() will return \ref BSP_ERROR_CALIBRATING.
 */

/**
 * @file UAIR_BSP_air_quality.h
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 * uAir interfacing to air quality sensor header
 *
 * @{
 */

#ifndef UAIR_BSP_AIR_QUALITY_H__
#define UAIR_BSP_AIR_QUALITY_H__

#include "BSP.h"
#include "HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 @brief Air quality results
 */
typedef struct {
    float O3_conc_ppb;  /*!< O3 (Ozone) concentration in PPB (parts per billion) */
    uint16_t FAST_AQI;  /*!< Fast Air Quality Index */
    uint16_t EPA_AQI;   /*!< EPA (averaged) Air Quality Index */
} BSP_air_quality_results_t;

BSP_error_t BSP_air_quality_start_measurement(void);
BSP_error_t BSP_air_quality_measurement_completed(void);
BSP_error_t BSP_air_quality_calculate(const float temp_c,
                                      const float hum_pct,
                                      BSP_air_quality_results_t *results);
unsigned int BSP_air_quality_get_measure_delay_us(void);
BSP_sensor_state_t BSP_air_quality_get_sensor_state(void);

#ifdef __cplusplus
}
#endif

/* @} */

#endif
