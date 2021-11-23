/** Copyright © 2021 MAIS Project
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
 * @file UAIR_BSP_externaltemp.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 * @ingroup BSPExternalTemp
 */

#ifndef UAIR_BSP_EXTERNALTEMP_H__
#define UAIR_BSP_EXTERNALTEMP_H__

#include "BSP.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A delay is required between starting a sensor measure amd performing the read.
 * This delay varies according to the resolution used.
 * This method allows the application to understand the minimum time interval
 * required between the start of measure
 *
 * @return The required delay (in microseconds) between start of measure and sensor readout
 */
unsigned int BSP_external_temp_hum_get_measure_delay_us(void);

/**
 * Start temperature/humidity measurement
 *
 * @return BSP_ERROR_NONE if successful
 * @return BSP_ERROR_NO_INIT if sensor was not successfully initialised
 * @return BSP_ERROR_BUSY if sensor is still pending a read from a previous start measure
 * @return BSP_ERROR_COMPONENT_FAILURE if any communication error occured
 */
BSP_error_t BSP_external_temp_hum_start_measure(void);

/**
 * Read temperature/humidity measurement (previously started)
 *
 * @return BSP_ERROR_NONE if successful
 * @return BSP_ERROR_NO_INIT if sensor was not successfully initialised
 * @return BSP_ERROR_BUSY if sensor is not currently measuring, if sensor reported stale data or
 * if the time interval between start of measure and the readout has not been observed
 * @return BSP_ERROR_COMPONENT_FAILURE if any communication error occured
 */
BSP_error_t BSP_external_temp_hum_read_measure(int32_t *temp, int32_t *hum);

BSP_sensor_state_t BSP_external_temp_get_sensor_state(void);

#ifdef __cplusplus
}
#endif

#endif
