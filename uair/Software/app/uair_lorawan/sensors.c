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
 * @file sensors.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include "app.h"
#include "UAIR_tracer.h"
#include "sensors.h"
#include "stm32_timer.h"
#include <stdlib.h>

#define TEMP_HUM_SAMPLING_INTERVAL_MS 10000

static UTIL_TIMER_Object_t TempSensTimer;

typedef enum {
    SENS_IDLE,
    SENS_ACQUIRE0,
    SENS_ACQUIRE1
} sensor_state_t;

static sensor_state_t sensor_state;

static int32_t ext_temp, ext_hum;
static int32_t int_temp, int_hum;

static enum {
    SENSOR_INTERNAL,
    SENSOR_EXTERNAL
} current_sensor;

static void OnTempSensTimerEvent(void __attribute__((unused)) *data)
{
    BSP_error_t err = BSP_ERROR_NO_INIT;

    unsigned acquisition_time_internal_ms = (BSP_internal_temp_hum_get_measure_delay_us() + 999)/1000;
    unsigned acquisition_time_external_ms = (BSP_external_temp_hum_get_measure_delay_us() + 999)/1000;
    unsigned acquisition_time;

    switch (sensor_state) {
    case SENS_IDLE:

        APP_PPRINTF("%s: acquisition times ms: internal %u, external %u\r\n",
                    __FUNCTION__,
                    acquisition_time_internal_ms,
                    acquisition_time_external_ms);

        /* Start both sensors at same time */
        err = BSP_internal_temp_hum_start_measure();
        if (err==BSP_ERROR_NONE) {
            err = BSP_external_temp_hum_start_measure();
        }
        if (BSP_ERROR_NONE==err) {
            // Compute first sensor to measure.
            if (acquisition_time_external_ms < acquisition_time_internal_ms) {
                acquisition_time = acquisition_time_external_ms;
                current_sensor = SENSOR_EXTERNAL;
            } else {
                acquisition_time = acquisition_time_internal_ms;
                current_sensor = SENSOR_INTERNAL;
            }

            sensor_state = SENS_ACQUIRE0;

            // TIMER IS BUGGY!
            UTIL_TIMER_SetPeriod(&TempSensTimer, 1+acquisition_time); // Timer seems to not take
            UTIL_TIMER_Start(&TempSensTimer);
        } else {
            APP_PPRINTF("%s: cannot start first temp measure (error %d sensor %d)\r\n", __FUNCTION__, err, current_sensor);
        }
        break;

    case SENS_ACQUIRE0: /* Fall-through */
    case SENS_ACQUIRE1:
        switch (current_sensor) {
        case SENSOR_INTERNAL:
            err = BSP_internal_temp_hum_read_measure(&int_temp, &int_hum);
            break;
        case SENSOR_EXTERNAL:
            err = BSP_external_temp_hum_read_measure(&ext_temp, &ext_hum);
            break;
        }

        if (err!=BSP_ERROR_NONE) {
            APP_PPRINTF("%s: cannot read first temp measure (error %d sensor %d)\r\n", __FUNCTION__, err, current_sensor);
        }
        if (sensor_state==SENS_ACQUIRE1) {
            sensor_state = SENS_IDLE;
            UTIL_TIMER_SetPeriod(&TempSensTimer, TEMP_HUM_SAMPLING_INTERVAL_MS);

            // Print out sensor data
            APP_PPRINTF("%s: Internal temp %f hum %f\r\n", __FUNCTION__, (float)int_temp/1000.0, (float)int_hum/1000.0);
            APP_PPRINTF("%s: External temp %f hum %f\r\n", __FUNCTION__, (float)ext_temp/1000.0, (float)ext_hum/1000.0);


        } else {
            // Move to next sensor.
            if (current_sensor==SENSOR_INTERNAL)
                current_sensor = SENSOR_EXTERNAL;
            else
                current_sensor = SENSOR_INTERNAL;
            sensor_state = SENS_ACQUIRE1;
           

            UTIL_TIMER_SetPeriod(&TempSensTimer, abs(acquisition_time_external_ms-acquisition_time_internal_ms));
        }
        UTIL_TIMER_Start(&TempSensTimer);
        break;
    }
}

sensors_op_result_t sensors_init(void)
{
    // Start reading timer. each 30 seconds.
    sensor_state = SENS_IDLE;

    UTIL_TIMER_Create(&TempSensTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTempSensTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TempSensTimer, TEMP_HUM_SAMPLING_INTERVAL_MS);
    UTIL_TIMER_Start(&TempSensTimer);

    APP_PPRINTF("\r\n Successfully intialized all sensors \r\n");
    return SENSORS_OP_SUCCESS;
}

sensors_op_result_t sensors_sample(sensors_t *sensor_data)
{
    int16_t status = 0;

    // TBD: check for valid data

    sensor_data->ext_temperature  = ext_temp;
    sensor_data->ext_humidity  = ext_hum;

    sensor_data->int_temperature  = int_temp;
    sensor_data->int_humidity  = int_hum;

    if (status != 0)
    {
        APP_PPRINTF("\r\n Failed to read sensor data Error status: %d \r\n", status);
        return SENSORS_OP_FAIL;
    }
    APP_PPRINTF("\r\n Successfully sampled sensors \r\n");
    return SENSORS_OP_SUCCESS;
}
