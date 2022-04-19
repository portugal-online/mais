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

#include "sensors.h"

#include "app.h"
#include "stm32_timer.h"

#include "UAIR_rtc.h"
#include "UAIR_tracer.h"
#include "UAIR_BSP_watchdog.h"

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#ifndef VERBOSE
#define LOG_VERBOSE(...)	do { if(false) APP_PPRINTF(__VA_ARGS__); } while(0)
#else
#define LOG_VERBOSE 		APP_PPRINTF
#endif

#define LOG					APP_PPRINTF

#define TEMP_HUM_SAMPLING_INTERVAL_MS 1998 /* As per ZMOD OAQ2 */

#define SAMPLE_AVG_ROTATION_THRESHOLD 100   /* number of previous samples to be
                                               accounted the for average calculation */
#define SAMPLE_AVG_MIN_THRESHOLD 50         /* minimum number of required valid samples
                                               to start returning a valid average measure */

#define SAMPLE_VALID_MIN_THRESHOLD 10       /* minimum number of required valid samples to consider
                                               the overall current data of the sensor as valid */

#define INVALID_SAMPLE INT_MAX

/* api definitions */
#define NUM_API_SENSORS SENSOR_ID_RESERVED

enum io_context_keys {
    dummy = 0
};

/* internal definitions */
typedef enum
{
    HWD_SENSOR_UNIT_NONE = -1,
    HWD_SENSOR_UNIT_TEMP_HUM_INTERNAL,
    HWD_SENSOR_UNIT_TEMP_HUM_EXTERNAL,
    HWD_SENSOR_UNIT_AQI,
} hwd_sensor_unit_t;

typedef enum
{
    SENSOR_MEASUREMENT_HUM_INTERNAL,
    SENSOR_MEASUREMENT_TEMP_INTERNAL,
    SENSOR_MEASUREMENT_HUM_EXTERNAL,
    SENSOR_MEASUREMENT_TEMP_EXTERNAL,
    SENSOR_MEASUREMENT_AQI,
    SENSOR_MEASUREMENT_SOUND,

    SENSOR_MEASUREMENT_SIZE
} sensor_measurement_t;

typedef enum
{
    VALIDITY_VALID,
    VALIDITY_STALE,
    VALIDITY_OLDAGE,
    VALIDITY_INVALID
} sensor_validity_t;

typedef struct
{
    unsigned (*get_measure_delay_us)(void);
    BSP_sensor_state_t (*get_state)(void);
    BSP_error_t (*start_measure)(void);
    BSP_error_t (*read_measure)(void);
} sensor_interface_t;

typedef struct
{
    int32_t temp;
    int32_t hum;
    sensor_validity_t validity;
} temp_hum_t;

/* global vars */
static struct
{
    int32_t rotation_index;
    int32_t previous_values[SAMPLE_AVG_ROTATION_THRESHOLD];
    int32_t value_avg;
    int32_t value_max;
    int32_t value_current;
} s_sensor_data[SENSOR_MEASUREMENT_SIZE];

static hwd_sensor_unit_t s_current_sensor;
static UTIL_TIMER_Object_t s_measure_timer;
static int s_time_elapsed;

static BSP_error_t internal_temp_hum_read_measure();
static BSP_error_t external_temp_hum_read_measure();
static BSP_error_t air_quality_read_measure();

static sensor_interface_t s_sensor_interfaces[] =
{
    {
        .get_measure_delay_us = BSP_internal_temp_hum_get_measure_delay_us,
        .get_state = BSP_internal_temp_hum_get_sensor_state,
        .start_measure = BSP_internal_temp_hum_start_measure,
        .read_measure = internal_temp_hum_read_measure,
    },
    {
        .get_measure_delay_us = BSP_external_temp_hum_get_measure_delay_us,
        .get_state = BSP_external_temp_hum_get_sensor_state,
        .start_measure = BSP_external_temp_hum_start_measure,
        .read_measure = external_temp_hum_read_measure,
    },
    {
        .get_measure_delay_us = BSP_air_quality_get_measure_delay_us,
        .get_state = BSP_air_quality_get_sensor_state,
        .start_measure = BSP_air_quality_start_measurement,
        .read_measure = air_quality_read_measure,
    },
};

static struct
{
    void* userdata;
    audit_event_cb_t cb;
} s_audit_listeners[NUM_API_SENSORS] = {0};

#define NUM_HWD_SENSORS (sizeof(s_sensor_interfaces)/sizeof(s_sensor_interfaces[0]))

static enum
{
    SENSOR_MEASURE_STATE_IDLE = 0,
    SENSOR_MEASURE_STATE_ACQUIRE
} s_sensor_measure_state;

static enum
{
    SENSOR_IDLE = 0,
    SENSOR_MEASURING
} s_sensor_status[NUM_HWD_SENSORS];

static int s_sensor_measuring_times[NUM_HWD_SENSORS]; // will hold delays between start measure and readout

/* */
static const char* sensor_measurement_name(sensor_measurement_t id)
{
    static const char *sensor_measurement_names[] = {
        "SENSOR_MEASUREMENT_HUM_INTERNAL",
        "SENSOR_MEASUREMENT_TEMP_INTERNAL",
        "SENSOR_MEASUREMENT_HUM_EXTERNAL",
        "SENSOR_MEASUREMENT_TEMP_EXTERNAL",
        "SENSOR_MEASUREMENT_AQI",
        "SENSOR_MEASUREMENT_SOUND"
    };

    return sensor_measurement_names[id];
}

static const char* sensor_hwd_unit_name(hwd_sensor_unit_t id)
{
    static const char *sensor_names[] = {
        "HWD_SENSOR_UNIT_TEMP_HUM_INTERNAL",
        "HWD_SENSOR_UNIT_TEMP_HUM_EXTERNAL",
        "HWD_SENSOR_UNIT_AQI",
    };

    return sensor_names[id];
}

static int average_calculation(sensor_measurement_t measurement, int32_t* avg)
{
    int i, valid_samples_count = 0;
    int32_t sum = 0;

    for (i = 0; i < SAMPLE_AVG_ROTATION_THRESHOLD; i++) {
        if (s_sensor_data[measurement].previous_values[i] == INVALID_SAMPLE)
            continue;

        valid_samples_count++;
        sum += s_sensor_data[measurement].previous_values[i];
    }

    if (valid_samples_count < SAMPLE_AVG_MIN_THRESHOLD) {
        LOG_VERBOSE("not enough valid samples");
        return -1;
    }

    *avg = sum / valid_samples_count;

    LOG_VERBOSE("(average calculation)\r\n%s: #samples=%d sum=%d avg=%f\r\n",
                sensor_measurement_name(measurement),
                valid_samples_count,
                sum,
                (float)*avg);
    return 0;
}

static int get_valid_sample(sensor_measurement_t measurement, int32_t* value) {

    int i, j;

    if (s_sensor_data[measurement].value_current != INVALID_SAMPLE) {
        *value = s_sensor_data[measurement].value_current;
        return 0;
    }

    // return error if we didn't find a valid value in the last SAMPLE_AVG_MIN_THRESHOLD samples
    for (i = s_sensor_data[measurement].rotation_index, j = 0; j < SAMPLE_VALID_MIN_THRESHOLD; ++j) {

        if (s_sensor_data[measurement].previous_values[i] != INVALID_SAMPLE) {
            *value = s_sensor_data[measurement].previous_values[i];
            return 0;
        }

        if (++i == SAMPLE_AVG_ROTATION_THRESHOLD)
            i = 0;
    }

    return -1;
}

static int has_valid_sample(sensor_measurement_t measurement) {
    int32_t dummy;
    return get_valid_sample(measurement, &dummy);
}

static int validate_sample(sensor_measurement_t measurement, int32_t new_value)
{
    /*
     * Temporary simplistic validations
     * We should consider more elaborated heuristics for the future
     */
    switch (measurement)
    {
    case SENSOR_MEASUREMENT_HUM_INTERNAL:
        if (new_value > 10 * 1000 && new_value < 90 * 1000)
            return 0;
    case SENSOR_MEASUREMENT_TEMP_INTERNAL:
        if (new_value > -5 * 1000 || new_value < 100 * 1000)
            return 0;

    case SENSOR_MEASUREMENT_HUM_EXTERNAL:
        if (new_value > 10 * 1000 || new_value < 90 * 1000)
            return 0;

    case SENSOR_MEASUREMENT_TEMP_EXTERNAL:
        if (new_value > -15 * 1000 || new_value < 60 * 1000)
            return 0;

    case SENSOR_MEASUREMENT_SOUND:
        if (new_value > -100 * 1000 || new_value < 100 * 1000)
            return 0;

    default:
        return -1;
    }

    return -1;
}

static void process_new_value(sensor_measurement_t measurement, int32_t new_value)
{
    uint8_t rotation_index;
    int32_t avg;

    rotation_index = s_sensor_data[measurement].rotation_index;
    if (rotation_index >= SAMPLE_AVG_ROTATION_THRESHOLD)
        rotation_index = 0;

    if (-1 == validate_sample(measurement, new_value))
        new_value = INVALID_SAMPLE;

    s_sensor_data[measurement].value_current = new_value;
    s_sensor_data[measurement].previous_values[rotation_index++] = new_value;
    s_sensor_data[measurement].rotation_index = rotation_index;

    if (new_value == INVALID_SAMPLE)
        return;

    if (s_sensor_data[measurement].value_max < new_value ||
        s_sensor_data[measurement].value_max == INVALID_SAMPLE)
        s_sensor_data[measurement].value_max = new_value;

    if (measurement == SENSOR_MEASUREMENT_AQI) {
        // avg already available from the sensor
        return;
    }

    if (-1 != average_calculation(measurement, &avg))
        s_sensor_data[measurement].value_avg = avg;
}

static BSP_error_t internal_temp_hum_read_measure()
{
    BSP_error_t err;
    int32_t temp, hum;

    err = BSP_internal_temp_hum_read_measure(&temp, &hum);
    if (err == BSP_ERROR_NONE) {
        LOG("internal temp=%f hum=%f\r\n", (float)temp / 1000.0, (float)hum / 1000.0);

        process_new_value(SENSOR_MEASUREMENT_HUM_INTERNAL, hum);
        process_new_value(SENSOR_MEASUREMENT_TEMP_INTERNAL, temp);
    } else
        LOG("error: cannot sample internal temperature\r\n");

    return err;
}

static BSP_error_t external_temp_hum_read_measure(void)
{
    BSP_error_t err;
    int32_t temp, hum;

    err = BSP_external_temp_hum_read_measure(&temp, &hum);
    if (err == BSP_ERROR_NONE) {
        LOG("external temp=%f hum=%f\r\n", (float)temp / 1000.0, (float)hum / 1000.0);

        process_new_value(SENSOR_MEASUREMENT_HUM_EXTERNAL, hum);
        process_new_value(SENSOR_MEASUREMENT_TEMP_EXTERNAL, temp);
    } else
        LOG("error: cannot sample external temperature\r\n");

    return err;
}

static BSP_error_t air_quality_read_measure()
{
    int32_t temp, hum;
    BSP_air_quality_results_t aqi;

    BSP_error_t err = BSP_air_quality_measurement_completed();
    if (err != BSP_ERROR_NONE) {
        LOG("error: air quality measurement failed (err=%d)\r\n", err);
        return err;
    }

    LOG_VERBOSE("air quality measurement completed\r\n");

    if (0 == get_valid_sample(SENSOR_MEASUREMENT_TEMP_EXTERNAL, &temp))
        LOG_VERBOSE("using external temp %d\r\n", temp);
    else if (0 == get_valid_sample(SENSOR_MEASUREMENT_TEMP_INTERNAL, &temp)) 
        LOG_VERBOSE("using internal temp %d\r\n, temp");
    else {
        // both sensors have invalid data.
        LOG("warn: using default temp\r\n");
        temp = 20000;
    }

    if (0 == get_valid_sample(SENSOR_MEASUREMENT_HUM_EXTERNAL, &hum)) 
        LOG_VERBOSE("using external hum %d\r\n", hum);
    else if (0 == get_valid_sample(SENSOR_MEASUREMENT_HUM_INTERNAL, &hum))
        LOG_VERBOSE("using internal hum %d\r\n", hum);
    else {
        // both sensors have invalid data.
        LOG("warn: using default hum\r\n");
        hum = 50000;
    }

    err = BSP_air_quality_calculate((float)temp / 1000.0,
                                    (float)hum / 1000.0,
                                    &aqi);

    LOG("BSP_air_quality_calculate %d\r\n", err);

    if (err == BSP_ERROR_NONE) {
        LOG("O3 concentration (ppb): %f \r\n", aqi.O3_conc_ppb );
        LOG("fast AQI : %d\r\n", aqi.FAST_AQI);
        LOG("EPA AQI  : %d\r\n", aqi.EPA_AQI);

        process_new_value(SENSOR_MEASUREMENT_AQI, aqi.FAST_AQI);
        s_sensor_data[SENSOR_MEASUREMENT_AQI].value_avg = aqi.EPA_AQI;
    }

    return err;
}

/**
 * @return delay or -1 if no delay is applicable
 */
static int sensor_start_measuring(hwd_sensor_unit_t sensor)
{
    int delay = -1;
    BSP_error_t err;

    s_sensor_status[sensor] = SENSOR_IDLE;

    sensor_interface_t *intf = &s_sensor_interfaces[sensor];

    BSP_sensor_state_t internal_sensor_state = intf->get_state();

    if (internal_sensor_state == SENSOR_AVAILABLE) {
        LOG_VERBOSE("start measure on sensor %d\r\n", sensor);

        err = intf->start_measure();
        if (BSP_ERROR_NONE == err) {
            s_sensor_status[sensor] = SENSOR_MEASURING;
            delay = (int) (intf->get_measure_delay_us() + 999) / 1000;
            return delay;
        } else
            LOG("cannot start measure, err %d\r\n", err);
    } else {
        // TBD: invalidate sensor data.
        LOG("sensor %d is not available\r\n", sensor);
    }

    switch (sensor)
    {
    case HWD_SENSOR_UNIT_TEMP_HUM_INTERNAL:
        process_new_value(SENSOR_MEASUREMENT_HUM_INTERNAL, INVALID_SAMPLE);
        process_new_value(SENSOR_MEASUREMENT_TEMP_INTERNAL, INVALID_SAMPLE);
        break;

    case HWD_SENSOR_UNIT_TEMP_HUM_EXTERNAL:
        process_new_value(SENSOR_MEASUREMENT_HUM_EXTERNAL, INVALID_SAMPLE);
        process_new_value(SENSOR_MEASUREMENT_TEMP_EXTERNAL, INVALID_SAMPLE);
        break;

    case HWD_SENSOR_UNIT_AQI:
        process_new_value(SENSOR_MEASUREMENT_AQI, INVALID_SAMPLE);
        break;

    default:
        break;
    }

    return delay;
}

static hwd_sensor_unit_t next_sensor_to_read(int elapsed, int *time_required)
{
    int i;
    int delay = INT_MAX;
    hwd_sensor_unit_t sensor = HWD_SENSOR_UNIT_NONE;

    LOG_VERBOSE("evaluate next sensor\r\n");

    for (i = 0; i < NUM_HWD_SENSORS; i++) {
        LOG_VERBOSE("sensor %d: time=%d delay=%d\r\n", sensor_hwd_unit_name(i), s_sensor_measuring_times[i], delay);
        if (s_sensor_measuring_times[i] >= 0) {
            if (s_sensor_measuring_times[i] < delay) {
                delay = s_sensor_measuring_times[i];
                sensor = i;
            }
        }
    }

    if (sensor != HWD_SENSOR_UNIT_NONE)
        *time_required = delay - elapsed;

    LOG_VERBOSE("selected sensor: %d\r\ntime required:%d\r\ndelay: %d\r\nelapsed: %d)\r\n",
                sensor,
                *time_required,
                delay,
                elapsed);

    return sensor;
}

static void sensor_read_and_process(hwd_sensor_unit_t sensor)
{
    sensor_interface_t *intf = &s_sensor_interfaces[sensor];
    intf->read_measure();
    s_sensor_measuring_times[sensor] = -1;
    s_sensor_status[sensor] = SENSOR_IDLE;
}

static void print_sensors()
{
    int i, j;
    uint16_t mseconds;
    uint32_t seconds = UAIR_RTC_GetTime(&mseconds);

    char bufs[3 * SENSOR_MEASUREMENT_SIZE][32];
    for (i = 0; i < SENSOR_MEASUREMENT_SIZE; i++) {

        j = 3 * i;
        if (s_sensor_data[i].value_current == INVALID_SAMPLE)
            strcpy((char*)(bufs + j), "NaN");
        else {
            if (i == SENSOR_MEASUREMENT_AQI)
                snprintf((char*)(bufs + j), 32, "%ld", (long)s_sensor_data[i].value_current);
            else
                snprintf((char*)(bufs + j), 32, "%f", (float)s_sensor_data[i].value_current / 1000.0);
        }

        j++;
        if (s_sensor_data[i].value_avg == INVALID_SAMPLE)
            strcpy((char*)(bufs + j), "NaN");
        else {
            if (i == SENSOR_MEASUREMENT_AQI)
                snprintf((char*)(bufs + j), 32, "%ld", (long)s_sensor_data[i].value_avg);
            else
                snprintf((char*)(bufs + j), 32, "%f", (float)s_sensor_data[i].value_avg / 1000.0);
        }

        j++;
        if (s_sensor_data[i].value_max == INVALID_SAMPLE)
            strcpy((char*)(bufs + j), "NaN");
        else {
            if (i == SENSOR_MEASUREMENT_AQI)
                snprintf((char*)(bufs + j), 32, "%ld", (long)s_sensor_data[i].value_max);
            else
                snprintf((char*)(bufs + j), 32, "%f", (float)s_sensor_data[i].value_max / 1000.0);
        }
    }

    LOG("\r\nSummary:\r\n"
        "time (s): %d.%d\r\n"
        "%s: current=%s avg=%s max=%s\r\n"
        "%s: current=%s avg=%s max=%s\r\n"
        "%s: current=%s avg=%s max=%s\r\n"
        "%s: current=%s avg=%s max=%s\r\n"
        "%s: current=%s avg=%s max=%s\r\n"
        "%s: current=%s avg=%s max=%s\r\n",
        seconds,
        mseconds,
        sensor_measurement_name(SENSOR_MEASUREMENT_TEMP_INTERNAL),
        bufs[3 * SENSOR_MEASUREMENT_TEMP_INTERNAL],
        bufs[3 * SENSOR_MEASUREMENT_TEMP_INTERNAL + 1],
        bufs[3 * SENSOR_MEASUREMENT_TEMP_INTERNAL + 2],
        sensor_measurement_name(SENSOR_MEASUREMENT_HUM_INTERNAL),
        bufs[3 * SENSOR_MEASUREMENT_HUM_INTERNAL],
        bufs[3 * SENSOR_MEASUREMENT_HUM_INTERNAL + 1],
        bufs[3 * SENSOR_MEASUREMENT_HUM_INTERNAL + 2],
        sensor_measurement_name(SENSOR_MEASUREMENT_TEMP_EXTERNAL),
        bufs[3 * SENSOR_MEASUREMENT_TEMP_EXTERNAL],
        bufs[3 * SENSOR_MEASUREMENT_TEMP_EXTERNAL + 1],
        bufs[3 * SENSOR_MEASUREMENT_TEMP_EXTERNAL + 2],
        sensor_measurement_name(SENSOR_MEASUREMENT_HUM_EXTERNAL),
        bufs[3 * SENSOR_MEASUREMENT_HUM_EXTERNAL],
        bufs[3 * SENSOR_MEASUREMENT_HUM_EXTERNAL + 1],
        bufs[3 * SENSOR_MEASUREMENT_HUM_EXTERNAL + 2],
        sensor_measurement_name(SENSOR_MEASUREMENT_AQI),
        bufs[3 * SENSOR_MEASUREMENT_AQI],
        bufs[3 * SENSOR_MEASUREMENT_AQI + 1],
        bufs[3 * SENSOR_MEASUREMENT_AQI + 2],
        sensor_measurement_name(SENSOR_MEASUREMENT_SOUND),
        bufs[3 * SENSOR_MEASUREMENT_SOUND],
        bufs[3 * SENSOR_MEASUREMENT_SOUND + 1],
        bufs[3 * SENSOR_MEASUREMENT_SOUND + 2]);
}

static void on_measure_timer_event(void __attribute__((unused)) *data)
{
    int time_required;
    uint8_t gain;
    //bool read_battery;


    uint32_t ticks = HAL_GetTick();
    LOG_VERBOSE("ticks: %d (%08x)\r\n", ticks, ticks);

    switch (s_sensor_measure_state) {
    case SENSOR_MEASURE_STATE_IDLE:

        UAIR_BSP_watchdog_kick();

        /* Start all sensors at same time */
        LOG("measure internal\r\n");
        s_sensor_measuring_times[HWD_SENSOR_UNIT_TEMP_HUM_INTERNAL] = sensor_start_measuring(HWD_SENSOR_UNIT_TEMP_HUM_INTERNAL);

        LOG("measure external\r\n");
        s_sensor_measuring_times[HWD_SENSOR_UNIT_TEMP_HUM_EXTERNAL] = sensor_start_measuring(HWD_SENSOR_UNIT_TEMP_HUM_EXTERNAL);

        LOG("measure AQI\r\n");
        s_sensor_measuring_times[HWD_SENSOR_UNIT_AQI] = sensor_start_measuring(HWD_SENSOR_UNIT_AQI);

        LOG("measure microphone\r\n");
        if( BSP_microphone_read_gain(&gain) == BSP_ERROR_NONE)
        	process_new_value(SENSOR_MEASUREMENT_SOUND, gain * 1000);

        s_time_elapsed = 0;
        hwd_sensor_unit_t next_sensor = next_sensor_to_read(s_time_elapsed, &time_required);

        if (next_sensor == HWD_SENSOR_UNIT_NONE) {
            LOG("warn: no sensors available!\r\n");
            UTIL_TIMER_SetPeriod(&s_measure_timer, TEMP_HUM_SAMPLING_INTERVAL_MS);
            UTIL_TIMER_Start(&s_measure_timer);
            break;
        }

        s_current_sensor = next_sensor;

        LOG("next sensor measure in %d ms\r\n", time_required);

        UTIL_TIMER_SetPeriod(&s_measure_timer, 1 + time_required); // give some margin
        UTIL_TIMER_Start(&s_measure_timer);

        s_sensor_measure_state = SENSOR_MEASURE_STATE_ACQUIRE;
        s_time_elapsed += time_required; // Take note on time used

        break;

    case SENSOR_MEASURE_STATE_ACQUIRE:
        LOG("read sensor: %d\r\n", sensor_hwd_unit_name(s_current_sensor));
        // Assumption is we have s_current_sensor.
        sensor_read_and_process(s_current_sensor);

        next_sensor = next_sensor_to_read(s_time_elapsed, &time_required);

        if (next_sensor == HWD_SENSOR_UNIT_NONE) {
            // all sensors done.
            print_sensors();
            s_sensor_measure_state = SENSOR_MEASURE_STATE_IDLE;
            LOG("delay measure in %d ms\r\n", TEMP_HUM_SAMPLING_INTERVAL_MS - s_time_elapsed);
            UTIL_TIMER_SetPeriod(&s_measure_timer, TEMP_HUM_SAMPLING_INTERVAL_MS - s_time_elapsed);
        } else {
            LOG("next sensor measure in %d ms\r\n", time_required);
            s_sensor_measure_state = SENSOR_MEASURE_STATE_ACQUIRE;
            s_time_elapsed += time_required;
            s_current_sensor = next_sensor;
            UTIL_TIMER_SetPeriod(&s_measure_timer, 1 + time_required); // give some margin
        }

        UTIL_TIMER_Start(&s_measure_timer);
        break;
    }
}

static uint8_t encode_humidity(uint32_t hum_millipercent)
{
    /*
     Encodes a humidity (0-100%) into 8-bit, using 0.5% accuracy

     To extract the humidity from the data (B), use:

      H = B/2 <=> B = 2*H

     Cached humidity (C) is given by:
      C = H*1000 <=> H = C/1000

     Hence:
      B = 2*(C/1000) <=> B = C/500

     */
    int32_t hum = hum_millipercent / 500;

    if (hum < 0) {
        LOG("warn: humidity value out of bounds: forcing value 0\r\n");
        hum = 0;
    }

    if (hum > 200) {
        LOG("warn: humidity value out of bounds: forcing value 200\r\n");
        hum = 200;
    }

    return (int8_t)(hum & 0xff);
}

static uint8_t encode_temperature(uint32_t temp_millicentigrades)
{
    /*
     Encodes a temperature between -11.75C and +52C into 8-bit, with 0.25C accuracy

     To extract the temperature from the data (B), use:

      T = (B-47)/4 <=> B = 4*T + 47

     Cached temperature (C) is given by:
      C = 1000*T <=> T = C/1000

     Hence:
      B = 4*(C/1000) + 47 <=> B = C/250 + 47
     */

    int32_t temp = (temp_millicentigrades / 250) + 47;
    if (temp < 0) {
        LOG("warn: temperature value out of bounds: forcing value 0\r\n");
        temp = 0;
    }

    if (temp > 255){
        LOG("warn: temperature value out of bounds: forcing value 255\r\n");
        temp = 255;
    }
    return (uint8_t)(temp & 0xff);
}

/* API implementation */

sensors_op_result_t UAIR_sensors_init(void)
{
    int i, j;

    if (SAMPLE_AVG_ROTATION_THRESHOLD < SAMPLE_AVG_MIN_THRESHOLD) {
        LOG("fatal: SAMPLE_AVG_ROTATION_THRESHOLD < SAMPLE_AVG_MIN_THRESHOLD\r\n");
        return SENSORS_OP_FAIL;
    }

    for (i = 0; i < SENSOR_MEASUREMENT_SIZE; ++i) {
        s_sensor_data[i].rotation_index = 0;
        s_sensor_data[i].value_avg = INVALID_SAMPLE;
        s_sensor_data[i].value_max = INVALID_SAMPLE;
        s_sensor_data[i].value_current = INVALID_SAMPLE;

        for (j = 0; j < SAMPLE_AVG_ROTATION_THRESHOLD; j++)
            s_sensor_data[i].previous_values[j] = INVALID_SAMPLE;
    }

    s_sensor_measure_state = SENSOR_MEASURE_STATE_IDLE;

    UTIL_TIMER_Create(&s_measure_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, on_measure_timer_event, NULL);
    UTIL_TIMER_SetPeriod(&s_measure_timer, TEMP_HUM_SAMPLING_INTERVAL_MS);
    UTIL_TIMER_Start(&s_measure_timer);

    LOG("successfully intialized all sensors \r\n");
    return SENSORS_OP_SUCCESS;
}

void UAIR_sensors_clear_measures()
{
    sensor_id_t i;

    for (i = 0; i < NUM_API_SENSORS; i++)
        UAIR_sensors_clear_measures_id(i);
}

void UAIR_sensors_clear_measures_id(sensor_id_t id)
{
    sensor_measurement_t measurement;
    int j;

    switch (id)
    {
    case SENSOR_ID_AIR_QLT:
    case SENSOR_ID_AIR_QLT_MAX:
        measurement = SENSOR_MEASUREMENT_AQI;
        break;

    case SENSOR_ID_TEMP_AVG_EXTERNAL:
        measurement = SENSOR_MEASUREMENT_TEMP_EXTERNAL;
        break;

    case SENSOR_ID_TEMP_MAX_INTERNAL:
        measurement = SENSOR_MEASUREMENT_TEMP_INTERNAL;
        break;

    case SENSOR_ID_HUM_AVG_EXTERNAL:
        measurement = SENSOR_MEASUREMENT_HUM_EXTERNAL;
        break;

    case SENSOR_ID_HUM_MAX_INTERNAL:
        measurement = SENSOR_MEASUREMENT_HUM_INTERNAL;
        break;

    case SENSOR_ID_SOUND_LVL_AVG:
    case SENSOR_ID_SOUND_LVL_MAX:
        measurement = SENSOR_MEASUREMENT_SOUND;
        break;

    default:
        return;
    }

    s_sensor_data[measurement].rotation_index = 0;
    s_sensor_data[measurement].value_avg = INVALID_SAMPLE;
    s_sensor_data[measurement].value_max = INVALID_SAMPLE;
    s_sensor_data[measurement].value_current = INVALID_SAMPLE;

    for (j = 0; j < SAMPLE_AVG_ROTATION_THRESHOLD; j++)
        s_sensor_data[measurement].previous_values[j] = INVALID_SAMPLE;
}

sensors_op_result_t UAIR_sensors_read_measure(sensor_id_t id, uint16_t* value)
{
    switch (id)
    {
    case SENSOR_ID_AIR_QLT:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_AQI))
            return SENSORS_OP_FAIL;

        *value = s_sensor_data[SENSOR_MEASUREMENT_AQI].value_avg;
        break;

    case SENSOR_ID_AIR_QLT_MAX:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_AQI))
            return SENSORS_OP_FAIL;

        *value = s_sensor_data[SENSOR_MEASUREMENT_AQI].value_max;
        break;

    case SENSOR_ID_TEMP_AVG_EXTERNAL:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_TEMP_EXTERNAL))
            return SENSORS_OP_FAIL;

        *value = encode_temperature(s_sensor_data[SENSOR_MEASUREMENT_TEMP_EXTERNAL].value_avg);
        break;

    case SENSOR_ID_TEMP_MAX_INTERNAL:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_TEMP_INTERNAL))
            return SENSORS_OP_FAIL;

        *value = encode_temperature(s_sensor_data[SENSOR_MEASUREMENT_TEMP_INTERNAL].value_max);
        break;

    case SENSOR_ID_HUM_AVG_EXTERNAL:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_HUM_EXTERNAL))
            return SENSORS_OP_FAIL;

        *value = encode_humidity(s_sensor_data[SENSOR_MEASUREMENT_HUM_EXTERNAL].value_avg);
        break;

    case SENSOR_ID_HUM_MAX_INTERNAL:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_HUM_INTERNAL))
            return SENSORS_OP_FAIL;

        *value = encode_humidity(s_sensor_data[SENSOR_MEASUREMENT_HUM_INTERNAL].value_max);
        break;

    case SENSOR_ID_SOUND_LVL_AVG:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_SOUND))
            return SENSORS_OP_FAIL;

        *value = (uint16_t)((float)s_sensor_data[SENSOR_MEASUREMENT_SOUND].value_avg / 1000.0);
        break;

    case SENSOR_ID_SOUND_LVL_MAX:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_SOUND))
            return SENSORS_OP_FAIL;

        *value = (uint16_t)((float)s_sensor_data[SENSOR_MEASUREMENT_SOUND].value_max / 1000.0);
        break;

    default:
        return SENSORS_OP_FAIL;
    }

    return SENSORS_OP_SUCCESS;
}

void UAIR_sensors_set_config_param_uint8(config_key_t key, uint8_t value)
{
    // TBD
}

void UAIR_sensors_set_config_param_uint16(config_key_t key, uint16_t value)
{
    // TBD
}

void UAIR_sensors_audit_register_listener(void* userdata, audit_event_cb_t cb)
{
    int i;
    for (i = 0; i < NUM_API_SENSORS; i++) {
        s_audit_listeners[i].cb = cb;
        s_audit_listeners[i].userdata = userdata;
    }
}

void UAIR_sensors_audit_unregister_listener()
{
    int i;
    for (i = 0; i < NUM_API_SENSORS; i++) {
        s_audit_listeners[i].cb = NULL;
        s_audit_listeners[i].userdata = NULL;
    }
}

void UAIR_sensors_audit_register_listener_id(void* userdata, audit_event_cb_t cb, sensor_id_t id)
{
    s_audit_listeners[id].cb = cb;
    s_audit_listeners[id].userdata = userdata;
}

void UAIR_sensors_audit_unregister_listener_id(sensor_id_t id)
{
    s_audit_listeners[id].cb = NULL;
    s_audit_listeners[id].userdata = NULL;
}

