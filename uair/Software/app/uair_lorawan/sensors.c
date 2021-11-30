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
#include "UAIR_rtc.h"
#include "sensors.h"
#include "stm32_timer.h"
#include <stdlib.h>
#include <limits.h>

#define TEMP_HUM_SAMPLING_INTERVAL_MS 1998 /* As per ZMOD OAQ2 */

static UTIL_TIMER_Object_t TempSensTimer;

typedef enum {
    SENS_IDLE,
    SENS_ACQUIRE
} sensor_fsm_state_t;

static sensor_fsm_state_t sensor_fsm_state;

enum sensor_id_e {
    SENSOR_INTERNAL = 0,
    SENSOR_EXTERNAL = 1,
    SENSOR_AQI = 2,
    SENSOR_NONE = -1
};

static enum sensor_id_e current_sensor;

struct sensor_interface {
    unsigned (*get_measure_delay_us)(void);
    BSP_sensor_state_t (*get_state)(void);
    BSP_error_t (*start_measure)(void);
    BSP_error_t (*read_measure)(void);
    void (*set_validity)(sensor_validity_t v);
    sensor_validity_t (*get_validity)(void);
};

static sensors_t sensor_data;

static BSP_error_t internal_temp_hum_read_measure(void);
static BSP_error_t external_temp_hum_read_measure(void);
static BSP_error_t air_quality_read_measure(void);

static void internal_temp_hum_set_validity(sensor_validity_t v) { sensor_data.th_internal.validity = v; }
static void external_temp_hum_set_validity(sensor_validity_t v) { sensor_data.th_external.validity = v; }
static void air_quality_set_validity(sensor_validity_t v) { sensor_data.aqi_validity = v; }
static sensor_validity_t internal_temp_hum_get_validity(void) { return sensor_data.th_internal.validity; }
static sensor_validity_t external_temp_hum_get_validity(void) { return sensor_data.th_external.validity; }
static sensor_validity_t air_quality_get_validity(void) { return sensor_data.aqi_validity; }

static inline sensor_validity_t next_validity_on_error(sensor_validity_t old)
{
    if (old < VALIDITY_INVALID)
        return old+1;
    return old;
}


static struct sensor_interface sensor_interface[] =
{
    {
        .get_measure_delay_us = BSP_internal_temp_hum_get_measure_delay_us,
        .get_state = BSP_internal_temp_get_sensor_state,
        .start_measure = BSP_internal_temp_hum_start_measure,
        .read_measure = internal_temp_hum_read_measure,
        .set_validity = internal_temp_hum_set_validity,
        .get_validity = internal_temp_hum_get_validity
    },
    {
        .get_measure_delay_us = BSP_external_temp_hum_get_measure_delay_us,
        .get_state = BSP_external_temp_get_sensor_state,
        .start_measure = BSP_external_temp_hum_start_measure,
        .read_measure = external_temp_hum_read_measure,
        .set_validity = external_temp_hum_set_validity,
        .get_validity = external_temp_hum_get_validity
    },
    {
        .get_measure_delay_us = BSP_air_quality_get_measure_delay_us,
        .get_state = BSP_air_quality_get_sensor_state,
        .start_measure = BSP_air_quality_start_measurement,
        .read_measure = air_quality_read_measure,
        .set_validity = air_quality_set_validity,
        .get_validity = air_quality_get_validity
    },
};

#define NUM_SENSORS (sizeof(sensor_interface)/sizeof(sensor_interface[0]))
#define NUM_TH_SENSORS 2

static enum { SENSOR_IDLE, SENSOR_MEASURING } sensor_status[NUM_SENSORS];

const char *val_names[] = {
    "VALID",
    "STALE",
    "OLDAGE",
    "INVALID"
};

static const char *validity_name(sensor_validity_t val)
{
    if (val<VALIDITY_VALID || val>VALIDITY_INVALID)
        val = VALIDITY_INVALID;

    return val_names[(int)val];
}

static BSP_error_t internal_temp_hum_read_measure(void)
{
    BSP_error_t err;

    err = BSP_internal_temp_hum_read_measure(&sensor_data.th_internal.temp,
                                             &sensor_data.th_internal.hum);

    if (err==BSP_ERROR_NONE) {
        APP_PPRINTF("%s: Internal temp %f hum %f\r\n", __FUNCTION__,
                    (float)sensor_data.th_internal.temp/1000.0,
                    (float)sensor_data.th_internal.hum/1000.0);
        sensor_data.th_internal.validity = VALIDITY_VALID;
    } else {
        APP_PPRINTF("%s: cannot sample internal temperature\r\n", __FUNCTION__);
        sensor_data.th_internal.validity = next_validity_on_error(sensor_data.th_internal.validity);
    }
    return err;

}


static BSP_error_t external_temp_hum_read_measure(void)
{
    BSP_error_t err;
    err = BSP_external_temp_hum_read_measure(&sensor_data.th_external.temp,
                                              &sensor_data.th_external.hum);
    if (err==BSP_ERROR_NONE) {
        APP_PPRINTF("%s: External temp %f hum %f\r\n", __FUNCTION__,
                    (float)sensor_data.th_external.temp/1000.0,
                    (float)sensor_data.th_external.hum/1000.0);
        sensor_data.th_external.validity = VALIDITY_VALID;
    } else {
        APP_PPRINTF("%s: cannot sample external temperature\r\n", __FUNCTION__);
        sensor_data.th_external.validity = next_validity_on_error(sensor_data.th_external.validity);
    }
    return err;
}

static BSP_error_t air_quality_read_measure(void)
{
    int32_t temp_i, hum_i;

    BSP_error_t err = BSP_air_quality_measurement_completed();
    APP_PPRINTF("%s: BSP_air_quality_measurement_completed %d\r\n", __FUNCTION__, err);

    if (BSP_ERROR_NONE==err) {
        // If we have recent external data, use external (even if stale)
        if (sensor_data.th_external.validity==VALIDITY_VALID ||
            sensor_data.th_external.validity==VALIDITY_STALE) {
            temp_i = sensor_data.th_external.temp;
            hum_i = sensor_data.th_external.hum;
            BSP_TRACE("Using external TH (%s)", validity_name(sensor_data.th_external.validity));
        } else {
            // If internal temp available, use it
            if (sensor_data.th_internal.validity==VALIDITY_VALID ||
                sensor_data.th_internal.validity==VALIDITY_STALE) {
                temp_i = sensor_data.th_external.temp;
                hum_i = sensor_data.th_external.hum;
                BSP_TRACE("Using internal TH (%s)", validity_name(sensor_data.th_internal.validity));
            } else {
                BSP_TRACE("Using dummy TH");
                // Both sensors have invalid data.
                temp_i = 20000;
                hum_i = 50000;
            }
        }



        err = BSP_air_quality_calculate((float)temp_i/1000.0,
                                        (float)hum_i/1000.0,
                                        &sensor_data.aqi);
        APP_PPRINTF("%s: BSP_air_quality_calculate %d\r\n", __FUNCTION__, err);
        if (err==BSP_ERROR_NONE) {
            APP_PPRINTF("%s: O3 concentration (ppb): %f \r\n", __FUNCTION__, sensor_data.aqi.O3_conc_ppb );
            APP_PPRINTF("%s: fast AQI : %d\r\n", __FUNCTION__, sensor_data.aqi.FAST_AQI);
            APP_PPRINTF("%s: EPA AQI  : %d\r\n", __FUNCTION__, sensor_data.aqi.EPA_AQI);
        } else {
        }
    }
    return err;
}

/* returns delay or -1 if no delay is applicable */
static int sensor_start_measuring(enum sensor_id_e sensor)
{
    int delay = -1;
    BSP_error_t err;
    sensor_validity_t validity;

    sensor_status[sensor] = SENSOR_IDLE;

    struct sensor_interface *intf = &sensor_interface[sensor];

    BSP_sensor_state_t internal_sensor_state = intf->get_state();

    if (internal_sensor_state == SENSOR_AVAILABLE) {
        //APP_PPRINTF("%s start measure on sensor %d\r\n", __FUNCTION__, sensor);
        err = intf->start_measure();
        if (err==BSP_ERROR_NONE) {
            sensor_status[sensor] = SENSOR_MEASURING;
            delay = (int) (intf->get_measure_delay_us() + 999)/1000;

            // Mark old information as STALE if was valid
            if (intf->get_validity()==VALIDITY_VALID)
                intf->set_validity(VALIDITY_STALE);

        } else {
            APP_PPRINTF("%s cannot start measure, err %d\r\n", __FUNCTION__, err);
            validity = intf->get_validity();
            intf->set_validity(next_validity_on_error(validity));
        }
    } else {
        // TBD: invalidate sensor data.
        intf->set_validity(VALIDITY_INVALID);
        APP_PPRINTF("%s: sensor %d is not available\r\n", __FUNCTION__, sensor);
    }
    return delay;
}

static int sensor_measuring_times[NUM_SENSORS]; // Will hold delays between start measure and readout

static enum sensor_id_e next_sensor_to_read(int *time_required, int elapsed)
{
    int i;
    int delay = INT_MAX;
    enum sensor_id_e sensor = SENSOR_NONE;

    //APP_PPRINTF("%s evaluate next sensor\r\n", __FUNCTION__);

    for (i=0;i<NUM_SENSORS;i++) {
        //  APP_PPRINTF("%s sensor %d time is %d delay %d\r\n", __FUNCTION__, i, sensor_measuring_times[i], delay);
        if (sensor_measuring_times[i]>=0) {
            if (sensor_measuring_times[i] < delay) {
                delay = sensor_measuring_times[i];
                sensor = i;
            }
        }
    }

    if (sensor!=SENSOR_NONE)
        *time_required = delay - elapsed;

    /*
    APP_PPRINTF("%s chose sensor %d time required %d (delay %d elapsed %d)\r\n", __FUNCTION__,
                sensor, *time_required,
                delay, elapsed);
                */
    return sensor;
}

static int time_elapsed;

static void sensor_read_and_process(enum sensor_id_e sensor)
{
    struct sensor_interface *intf = &sensor_interface[sensor];
    intf->read_measure();
    sensor_measuring_times[sensor] = -1;
    sensor_status[sensor] = SENSOR_IDLE;
}

static void sensors_done()
{
    uint16_t mseconds;
    uint32_t seconds = UAIR_RTC_GetTime(&mseconds);

    APP_PPRINTF("SENSOR_DATA"
                ":%d.%d"
                ":%f"
                ":%f"
                ":%f"
                ":%f"
                ":%f"
                ":%d"
                ":%d"
                ":%d"
                "\r\n",
                seconds,
                mseconds,
                (float)sensor_data.th_internal.temp/1000.0,
                (float)sensor_data.th_internal.hum/1000.0,
                (float)sensor_data.th_external.temp/1000.0,
                (float)sensor_data.th_external.hum/1000.0,
                sensor_data.aqi.O3_conc_ppb,
                sensor_data.aqi.FAST_AQI,
                sensor_data.aqi.EPA_AQI,
                sensor_data.mic_gain);

}

static void OnTempSensTimerEvent(void __attribute__((unused)) *data)
{
    int time_required;
    uint8_t gain;
    uint32_t t = HAL_GetTick();
    APP_PPRINTF("Ticks: %d (%08x)\r\n", t, t);

    switch (sensor_fsm_state) {
    case SENS_IDLE:
        APP_PPRINTF("Measure internal\r\n");
        /* Start all sensors at same time */
//        APP_PPRINTF("Starting measurements\r\n");
        sensor_measuring_times[SENSOR_INTERNAL] = sensor_start_measuring(SENSOR_INTERNAL);
        APP_PPRINTF("Measure external\r\n");
        sensor_measuring_times[SENSOR_EXTERNAL] = sensor_start_measuring(SENSOR_EXTERNAL);
        APP_PPRINTF("Measure AQI\r\n");

        sensor_measuring_times[SENSOR_AQI] = sensor_start_measuring(SENSOR_AQI);
        APP_PPRINTF("Measure microphone\r\n");

        if( BSP_microphone_read_gain(&gain)==BSP_ERROR_NONE) {
            sensor_data.mic_gain = gain;
        } else {
            sensor_data.mic_gain = -1;
        }

        time_elapsed = 0;

        enum sensor_id_e next_sensor = next_sensor_to_read(&time_required, time_elapsed);

        if (next_sensor==SENSOR_NONE) {
            APP_PPRINTF("%s: no sensors available!!!\r\n", __FUNCTION__);
            UTIL_TIMER_SetPeriod(&TempSensTimer, TEMP_HUM_SAMPLING_INTERVAL_MS);
            UTIL_TIMER_Start(&TempSensTimer);
            break;
        }

        current_sensor = next_sensor;

        APP_PPRINTF("%s: next sensor measure in %d ms\r\n", __FUNCTION__, time_required);

        UTIL_TIMER_SetPeriod(&TempSensTimer, 1+time_required); // Timer seems to not take
        UTIL_TIMER_Start(&TempSensTimer);

        sensor_fsm_state = SENS_ACQUIRE;
        time_elapsed += time_required; // Take note on time used

        break;

    case SENS_ACQUIRE:
        APP_PPRINTF("Starting reads sensor %d\r\n", current_sensor);
        // Assumption is we have current_sensor.
        sensor_read_and_process(current_sensor);

        next_sensor = next_sensor_to_read(&time_required, time_elapsed);

        if (next_sensor==SENSOR_NONE) {
            // All sensors done.
            sensors_done();
            sensor_fsm_state = SENS_IDLE;
            APP_PPRINTF("%s: delay measure in %d ms\r\n", __FUNCTION__, TEMP_HUM_SAMPLING_INTERVAL_MS - time_elapsed);
            UTIL_TIMER_SetPeriod(&TempSensTimer, TEMP_HUM_SAMPLING_INTERVAL_MS - time_elapsed);
        } else {
            APP_PPRINTF("%s: next sensor measure in %d ms\r\n", __FUNCTION__, time_required);
            sensor_fsm_state = SENS_ACQUIRE;
            time_elapsed += time_required; // Take note on time used
            current_sensor = next_sensor;
            UTIL_TIMER_SetPeriod(&TempSensTimer, 1+time_required);
        }

        UTIL_TIMER_Start(&TempSensTimer);
        break;
    }
}

sensors_op_result_t sensors_init(void)
{
    sensor_fsm_state = SENS_IDLE;

    sensor_data.th_external.validity = VALIDITY_INVALID;
    sensor_data.th_internal.validity = VALIDITY_INVALID;
    sensor_data.aqi_validity = VALIDITY_INVALID;

    UTIL_TIMER_Create(&TempSensTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTempSensTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TempSensTimer, TEMP_HUM_SAMPLING_INTERVAL_MS);
    UTIL_TIMER_Start(&TempSensTimer);

    APP_PPRINTF("\r\n Successfully intialized all sensors \r\n");
    return SENSORS_OP_SUCCESS;
}

sensors_op_result_t sensors_sample(sensors_t *sensor_data)
{
    return 0;
}
