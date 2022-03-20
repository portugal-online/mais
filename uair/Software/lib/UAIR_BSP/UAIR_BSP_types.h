/**
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
 * @file UAIR_BSP_types.h
 * @brief Data types used by the uAir BSP
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_CORE
 *
 * Data types used by the uAir BSP.
 */

#ifndef UAIR_BSP_TYPES_H__
#define UAIR_BSP_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/** uAir Board version. The current board version can be obtained using BSP_get_board_version() */
typedef enum
{
    UAIR_UNKNOWN,     /*!< Unknown board version */
    UAIR_NUCLEO_REV1, /*!< Development board, revision 1 */
    UAIR_NUCLEO_REV2  /*!< Development board, revision 2 */
} BSP_board_version_t;


/** Temperature accuracy. Used by all temperature sensors.
 Actual accuracy depends on the sensor */
typedef enum
{
    TEMP_ACCURACY_LOW, /*!< Low (lowest) accuracy */
    TEMP_ACCURACY_MED, /*!< Medium accuracy */
    TEMP_ACCURACY_HIGH /*!< High (maximum) accuracy */
} BSP_temp_accuracy_t;


/** Humidity accuracy. Used by all humidity sensors.
 Actual accuracy depends on the sensor */
typedef enum
{
    HUM_ACCURACY_LOW, /*!< Low (lowest) accuracy */
    HUM_ACCURACY_MED, /*!< Medium accuracy */
    HUM_ACCURACY_HIGH /*!< High (maximum) accuracy */
} BSP_hum_accuracy_t;

/** Physical sensor state. */
typedef enum
{
    SENSOR_AVAILABLE, /*!< Sensor available */
    SENSOR_OFFLINE,   /*!< Sensor currently offline */
    SENSOR_FAULTY     /*!< Sensor faulty */
} BSP_sensor_state_t;

/** BSP configuration. This structure is used to configure the BSP using BSP_init(). The default configuration
 can be obtained using BSP_get_default_config() */
typedef struct {
    void (*bsp_error)(BSP_error_t error);  /*!< BSP error handler. This is a callback function that will be called whenever a
                                                BSP error is raised */
    bool skip_shield_init;                 /*!< Skip shield/board initialization. Should be set to false */
    bool high_performance;                 /*!< High-performance operation. Should be set to false */
    BSP_temp_accuracy_t temp_accuracy;     /*!< BSP-wide temperature accuracy */
    BSP_hum_accuracy_t hum_accuracy;       /*!< BSP-wide humidity accuracy */
} BSP_config_t;

#ifdef __cplusplus
}
#endif

#endif
