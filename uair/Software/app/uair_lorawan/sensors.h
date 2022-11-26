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
 * @file sensors.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef SENSORS_H__
#define SENSORS_H__

#include "UAIR_BSP_air_quality.h"

typedef enum
{
    SENSOR_ID_AIR_QLT = 0,
    SENSOR_ID_AIR_QLT_MAX,
    SENSOR_ID_TEMP_AVG_EXTERNAL,
    SENSOR_ID_TEMP_MAX_INTERNAL,
    SENSOR_ID_HUM_AVG_EXTERNAL,
    SENSOR_ID_HUM_MAX_INTERNAL,
    SENSOR_ID_SOUND_LVL_AVG,
    SENSOR_ID_SOUND_LVL_MAX,
    SENSOR_ID_BATTERY,

	SENSOR_ID_RESERVED
} sensor_id_t;

typedef enum
{
    SENSORS_OP_SUCCESS = 0,
    SENSORS_OP_FAIL = 1,
} sensors_op_result_t;

typedef enum io_context_keys config_key_t;

typedef void (*audit_event_cb_t)(void *userdata, uint8_t audit_type);

/**
 * @brief Initializes all the sensors that will be sampled
 *
 * @return sensors_op_result_t
 */
sensors_op_result_t UAIR_sensors_init(void);

/**
 * @brief Clears all the collected data for all sensors
 */
void UAIR_sensors_clear_measures(void);

/**
 * @brief Clears all the collected data for a specific sensor
 *
 * @param id sensor identifier
 */
void UAIR_sensors_clear_measures_id(sensor_id_t id);

/**
 * @brief Reads the current measure (based on collected data) of a specific sensor
 *
 * @param id sensor identifier
 * @param value read measure
 */
sensors_op_result_t UAIR_sensors_read_measure(sensor_id_t id, uint16_t* value);

/**
 * @brief Set config parameter
 *
 * @param key key for the affected parameter
 * @param value new value for the affected parameter
 */
void UAIR_sensors_set_config_param_uint8(config_key_t key, uint8_t value);

/**
 * @brief Set config parameter
 *
 * @param key key for the affected parameter
 * @param value new value for the affected parameter
 */
void UAIR_sensors_set_config_param_uint16(config_key_t key, uint16_t value);

/**
 * @brief Subscribe for audit events of all sensors
 *
 * @param cb callback to be invoked whenever an audit event is emmitted
 * @param value new value for the affected parameter
 */
void UAIR_sensors_audit_register_listener(void* userdata, audit_event_cb_t cb);

/**
 * @brief Subscribe for audit events of a specific sensor
 *
 * @param cb callback to be invoked whenever an audit event is emmitted
 * @param id sensor identifier
 */
void UAIR_sensors_audit_register_listener_id(void* userdata, audit_event_cb_t cb, sensor_id_t id);

/**
 * @brief Unsubscribe for audit events of all sensors
 */
void UAIR_sensors_audit_unregister_listener();

/**
 * @brief Unsubscribe for audit events of a specific sensor
 *
 * @param id sensor identifier
 */
void UAIR_sensors_audit_unregister_listener_id(sensor_id_t id);


void sensor_start_measure_callback();
void sensor_end_measure_callback();

#endif /* __SENSORS_H__ */
