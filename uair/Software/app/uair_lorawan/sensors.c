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
#include "controller.h"

#include "app.h"
#include "stm32_timer.h"

#include "UAIR_rtc.h"
#include "UAIR_tracer.h"
#include "UAIR_BSP_watchdog.h"

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#ifndef VERBOSE
#define LOG_VERBOSE(...)	do { if(false) { APP_PPRINTF("[%010d] SENSORS: ", HAL_GetTick()); APP_PPRINTF(__VA_ARGS__); } } while(0)
#else
#define LOG_VERBOSE(...)	do { if(true) { APP_PPRINTF("[%010d] SENSORS: ", HAL_GetTick()); APP_PPRINTF(__VA_ARGS__); } } while(0)
#endif

#define LOG(...)			do { if(true) { APP_PPRINTF("[%010d] SENSORS: ", HAL_GetTick()); APP_PPRINTF(__VA_ARGS__); } } while(0)

#define TEMP_HUM_SAMPLING_INTERVAL_MS 1998 /* As per ZMOD OAQ2 */
//#define TEMP_HUM_SAMPLING_INTERVAL_MS 1900 /* As per ZMOD OAQ2 */

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
    HWD_SENSOR_UNIT_MICROPHONE,

    HWD_SENSOR_UNIT_SIZE
} hwd_sensor_unit_t;

typedef enum
{
    SENSOR_MEASUREMENT_HUM_INTERNAL,
    SENSOR_MEASUREMENT_TEMP_INTERNAL,
    SENSOR_MEASUREMENT_HUM_EXTERNAL,
    SENSOR_MEASUREMENT_TEMP_EXTERNAL,
    SENSOR_MEASUREMENT_AQI,
    SENSOR_MEASUREMENT_SOUND,
    SENSOR_MEASUREMENT_BATTERY,

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
    bool (*enabled_at_tick)(uint32_t tick);
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
static UTIL_TIMER_Object_t s_battery_enable_timer;
static UTIL_TIMER_Object_t s_battery_measure_timer;
static int s_time_elapsed;

static BSP_error_t internal_temp_hum_read_measure(void);
static BSP_error_t external_temp_hum_read_measure(void);
static BSP_error_t air_quality_read_measure(void);
static BSP_error_t microphone_read_measure(void);

static bool internal_temp_hum_enabled_at_tick(uint32_t ticks);
static bool external_temp_hum_enabled_at_tick(uint32_t ticks);
static bool air_quality_enabled_at_tick(uint32_t ticks);
static bool microphone_enabled_at_tick(uint32_t ticks);

static unsigned int microphone_get_measure_delay_us(void);
static BSP_error_t microphone_start_measurement(void);

static sensor_interface_t s_sensor_interfaces[] =
{
    {
        .get_measure_delay_us = BSP_internal_temp_hum_get_measure_delay_us,
        .get_state = BSP_internal_temp_hum_get_sensor_state,
        .start_measure = BSP_internal_temp_hum_start_measure,
        .read_measure = internal_temp_hum_read_measure,
        .enabled_at_tick = internal_temp_hum_enabled_at_tick,
    },
    {
        .get_measure_delay_us = BSP_external_temp_hum_get_measure_delay_us,
        .get_state = BSP_external_temp_hum_get_sensor_state,
        .start_measure = BSP_external_temp_hum_start_measure,
        .read_measure = external_temp_hum_read_measure,
        .enabled_at_tick = external_temp_hum_enabled_at_tick,
    },
    {
        .get_measure_delay_us = BSP_air_quality_get_measure_delay_us,
        .get_state = BSP_air_quality_get_sensor_state,
        .start_measure = BSP_air_quality_start_measurement,
        .read_measure = air_quality_read_measure,
        .enabled_at_tick = air_quality_enabled_at_tick,
    },
    {
        .get_measure_delay_us = microphone_get_measure_delay_us,
        .get_state = BSP_microphone_get_sensor_state,
        .start_measure = microphone_start_measurement,
        .read_measure = microphone_read_measure,
        .enabled_at_tick = microphone_enabled_at_tick,
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
static uint32_t s_total_measure_ticks = 0;

static bool s_battery_triggered = false;

#define BATTERY_TIME_SAMPLE_BEFORE_RADIO (6000) /* Sample 6s before radio starts */
#define BATTERY_TIME_DEADLINE_BEFORE_RADIO (3000) /* Deadline: 3s before radio starts */
#define BATTERY_TIME_CLEANUP (80000) /* Clean up 80s before radio completes */
#define BATTERY_TIME_SETTLE (500)     /* Time for battery to settle (after enabling ADC readout) in ms */
#define BATTERY_TIME_MARGIN (200)     /* Margin after battery sampling and next sensor acquisition in ms */

/* */
#if (! defined(RELEASE)) || (RELEASE==0)
static const char* sensor_measurement_name(sensor_measurement_t id)
{
    static const char *sensor_measurement_names[] = {
        "SENSOR_MEASUREMENT_HUM_INTERNAL",
        "SENSOR_MEASUREMENT_TEMP_INTERNAL",
        "SENSOR_MEASUREMENT_HUM_EXTERNAL",
        "SENSOR_MEASUREMENT_TEMP_EXTERNAL",
        "SENSOR_MEASUREMENT_AQI",
        "SENSOR_MEASUREMENT_SOUND",
        "SENSOR_MEASUREMENT_BATTERY"
    };

    return sensor_measurement_names[id];
}

static const char* sensor_hwd_unit_name(hwd_sensor_unit_t id)
{
    static const char *sensor_names[] = {
        "HWD_SENSOR_UNIT_TEMP_HUM_INTERNAL",
        "HWD_SENSOR_UNIT_TEMP_HUM_EXTERNAL",
        "HWD_SENSOR_UNIT_AQI",
        "HWD_SENSOR_UNIT_MICROPHONE",
    };

    return sensor_names[id];
}
#endif

static void schedule_battery_readout(int delay_to_next_sampling)
{
    int32_t next = UAIR_controller_time_to_next_transmission_ms();

    if ( (next < BATTERY_TIME_SAMPLE_BEFORE_RADIO) // We need to sample
        && (next >= BATTERY_TIME_DEADLINE_BEFORE_RADIO) // But don't overlap radio and battery
        && (!s_battery_triggered)) {

        LOG_VERBOSE("Battery: scheduling read next=%d\r\n", next);
        s_battery_triggered = false;
        UTIL_TIMER_SetPeriod(&s_battery_enable_timer, delay_to_next_sampling - (BATTERY_TIME_SETTLE + BATTERY_TIME_MARGIN) );
        UTIL_TIMER_Start(&s_battery_enable_timer);
    }
    if (next >= BATTERY_TIME_CLEANUP)
    {
        s_battery_triggered = false;
    }
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
        LOG_VERBOSE("not enough valid samples\r\n");
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
        if (new_value >= 0 * 1000 && new_value <= 100 * 1000)
            return 0;
        break;

    case SENSOR_MEASUREMENT_TEMP_INTERNAL:
        if (new_value >= -40 * 1000 && new_value <= 125 * 1000)
            return 0;
        break;

    case SENSOR_MEASUREMENT_HUM_EXTERNAL:
        if (new_value >= 0 && new_value <= 100 * 1000)
            return 0;
        break;

    case SENSOR_MEASUREMENT_TEMP_EXTERNAL:
        if (new_value >= -40 * 1000 && new_value <= 125 * 1000)
            return 0;
        break;

    case SENSOR_MEASUREMENT_SOUND:
        if (new_value >= 0 && new_value <= 31 * 1000)
            return 0;
        break;

    case SENSOR_MEASUREMENT_AQI:
        if (new_value >= 0 && new_value <= 501)
            return 0;
        break;

    case SENSOR_MEASUREMENT_BATTERY:
        if (new_value >= 1700 && new_value <= 3900)
            return 0;
        break;

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

#if defined (FORCE_TEMP_HUM)
    /* Force temp/hum. This is only for debugging purposes */
    temp = 20000; // 20C temp
    hum = 50000; // 50% hum
#endif

    err = BSP_air_quality_calculate((float)temp / 1000.0,
                                    (float)hum / 1000.0,
                                    &aqi);

    LOG("BSP_air_quality_calculate %d\r\n", err);

    if (err == BSP_ERROR_NONE) {
        LOG("O3 concentration (ppb): %f \r\n", aqi.O3_conc_ppb );
        if (!isnanf(aqi.NO2_conc_ppb)) {
            LOG("NO2 concentration (ppb): %f \r\n", aqi.NO2_conc_ppb );
        }
        LOG("fast AQI : %d\r\n", aqi.FAST_AQI);
        LOG("EPA AQI  : %d\r\n", aqi.EPA_AQI);

        process_new_value(SENSOR_MEASUREMENT_AQI, aqi.FAST_AQI);
        s_sensor_data[SENSOR_MEASUREMENT_AQI].value_avg = aqi.EPA_AQI;
    }

    return err;
}

/**
 * @return delay or
 * -1 if no delay is applicable
 * 0 if the measure needs to be done prior to sampling
 */
static int sensor_start_measuring(hwd_sensor_unit_t sensor)
{
    int delay = -1;
    BSP_error_t err;

    s_sensor_status[sensor] = SENSOR_IDLE;

    sensor_interface_t *intf = &s_sensor_interfaces[sensor];

    BSP_sensor_state_t internal_sensor_state = intf->get_state();

    if (internal_sensor_state == SENSOR_AVAILABLE) {

        if (intf->enabled_at_tick(s_total_measure_ticks))
        {
            err = intf->start_measure();
            if (BSP_ERROR_NONE == err) {
                s_sensor_status[sensor] = SENSOR_MEASURING;
                delay = (int) (intf->get_measure_delay_us() + 999) / 1000;
                return delay;
            } else
                LOG("cannot start measure, err %d\r\n", err);
        } else {
            // Do not log invalid samples.
            return -1;
        }

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

    case HWD_SENSOR_UNIT_MICROPHONE:
        process_new_value(SENSOR_MEASUREMENT_SOUND, INVALID_SAMPLE);
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
        LOG_VERBOSE("sensor %s: time=%d delay=%d\r\n", sensor_hwd_unit_name(i), s_sensor_measuring_times[i], delay);

        if (s_sensor_measuring_times[i] > 0) {
            if (s_sensor_measuring_times[i] < delay) {
                delay = s_sensor_measuring_times[i];
                sensor = i;
            }
        }
    }

    if (sensor != HWD_SENSOR_UNIT_NONE) {
        *time_required = delay - elapsed;

        LOG_VERBOSE("selected sensor: %d\r\ntime required:%d\r\n", sensor, *time_required);
    }

    LOG_VERBOSE("delay: %d\r\nelapsed: %d\r\n", delay, elapsed);
    return sensor;
}

static void sensor_read_and_process(hwd_sensor_unit_t sensor)
{
    sensor_interface_t *intf = &s_sensor_interfaces[sensor];
    intf->read_measure();
    s_sensor_measuring_times[sensor] = -1;
    s_sensor_status[sensor] = SENSOR_IDLE;
}

#if (! defined(RELEASE)) || (RELEASE==0)

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
                snprintf((char*)(bufs + j), 32, "%f.02", (float)s_sensor_data[i].value_current / 1000.0);
        }

        j++;
        if (s_sensor_data[i].value_avg == INVALID_SAMPLE)
            strcpy((char*)(bufs + j), "NaN");
        else {
            if (i == SENSOR_MEASUREMENT_AQI)
                snprintf((char*)(bufs + j), 32, "%ld", (long)s_sensor_data[i].value_avg);
            else
                snprintf((char*)(bufs + j), 32, "%.02f", (float)s_sensor_data[i].value_avg / 1000.0);
        }

        j++;
        if (s_sensor_data[i].value_max == INVALID_SAMPLE)
            strcpy((char*)(bufs + j), "NaN");
        else {
            if (i == SENSOR_MEASUREMENT_AQI)
                snprintf((char*)(bufs + j), 32, "%ld", (long)s_sensor_data[i].value_max);
            else
                snprintf((char*)(bufs + j), 32, "%.02f", (float)s_sensor_data[i].value_max / 1000.0);
        }
    }

    LOG("Summary:\r\n");
    LOG("time (s): %d.%d\r\n", seconds, mseconds);
    for (unsigned m = 0; m<SENSOR_MEASUREMENT_SIZE; m++) {
        LOG("%s: current=%s avg=%s max=%s\r\n",
            sensor_measurement_name(m),
            bufs[3 * m],
            bufs[3 * m + 1],
            bufs[3 * m + 2]);
    }
}

#endif

static void on_measure_timer_event(void __attribute__((unused)) *data)
{
    int time_required;
    unsigned sensor;

#if !defined(RELEASE) || (RELEASE==0)
    uint32_t ticks = HAL_GetTick();
    LOG("ticks: %d - measure ticks %d\r\n", ticks, s_total_measure_ticks);
#endif

    switch (s_sensor_measure_state) {
    case SENSOR_MEASURE_STATE_IDLE:
        sensor_start_measure_callback();
        UAIR_BSP_watchdog_kick();

        /* Check if any sensor is pending read */
        for (sensor = 0; sensor < HWD_SENSOR_UNIT_SIZE; sensor++) {

            sensor_interface_t *intf = &s_sensor_interfaces[sensor];

            if (intf->enabled_at_tick(s_total_measure_ticks)) {
                if (s_sensor_measuring_times[sensor] == 0) {
                    sensor_read_and_process(sensor);
                    s_sensor_measuring_times[sensor] = -1;
                }
            }
        }

        /* Start all sensors at same time if they are enabled */
        for (unsigned sensor = 0; sensor < HWD_SENSOR_UNIT_SIZE; sensor++) {
            if (s_sensor_interfaces[sensor].enabled_at_tick(s_total_measure_ticks)) {

                LOG("start measure %s\r\n", sensor_hwd_unit_name(sensor));
                s_sensor_measuring_times[sensor] = sensor_start_measuring(sensor);

            }
        }

        s_time_elapsed = 0;
        hwd_sensor_unit_t next_sensor = next_sensor_to_read(s_time_elapsed, &time_required);

        if (next_sensor == HWD_SENSOR_UNIT_NONE) {
            LOG("warn: no sensors available!\r\n");

            // Increase number of ticks.
            s_total_measure_ticks++;

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
        LOG("Read sensor %s\r\n", sensor_hwd_unit_name(s_current_sensor));
        // Assumption is we have s_current_sensor.
        sensor_read_and_process(s_current_sensor);

        next_sensor = next_sensor_to_read(s_time_elapsed, &time_required);

        if (next_sensor == HWD_SENSOR_UNIT_NONE) {
            // all sensors done.
#if (!defined RELEASE) && (RELEASE==0)
            print_sensors();
#endif
            sensor_end_measure_callback();
            s_sensor_measure_state = SENSOR_MEASURE_STATE_IDLE;

            LOG("delay measure in %d ms\r\n", TEMP_HUM_SAMPLING_INTERVAL_MS - s_time_elapsed);
            // Increase number of ticks.
            s_total_measure_ticks++;

            UTIL_TIMER_SetPeriod(&s_measure_timer, TEMP_HUM_SAMPLING_INTERVAL_MS - s_time_elapsed);
            // Schedule a battery readout if required
            schedule_battery_readout( TEMP_HUM_SAMPLING_INTERVAL_MS - s_time_elapsed );
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
    int32_t hum = (hum_millipercent+499) / 1000;

    if (hum < 0) {
        LOG("warn: humidity value out of bounds: forcing value 0\r\n");
        hum = 0;
    }

    if (hum > 127) {
        LOG("warn: humidity value out of bounds: forcing value 200\r\n");
        hum = 127;
    }

    return (int8_t)(hum & 0xff);
}

static uint8_t encode_temperature(int32_t temp_millicentigrades)
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
    int32_t temp;

    if (temp_millicentigrades>0) {
        temp = ((temp_millicentigrades + 124) / 250) + 47;
    } else {
        temp = ((temp_millicentigrades - 124) / 250) + 47;
    }

    if (temp < 0) {
        LOG("warn: temperature value (in %d) out of bounds: forcing value 0\r\n", temp_millicentigrades);
        temp = 0;
    }

    if (temp > 255){
        LOG("warn: temperature value (in %d) out of bounds: forcing value 255\r\n", temp_millicentigrades);
        temp = 255;
    }
    return (uint8_t)(temp & 0xff);
}

static void battery_data_ready(BSP_error_t err, const battery_measurements_t *meas)
{
    UAIR_BSP_BM_DisableBatteryRead();
    UAIR_BSP_BM_EndAcquisition();

    if (err == BSP_ERROR_NONE) {
        LOG("Battery voltage mv: %d\r\n", meas->battery_voltage_mv);
        process_new_value(SENSOR_MEASUREMENT_BATTERY, meas->battery_voltage_mv);
    }
}

static void battery_measure_timer_event(void __attribute__((unused)) *data)
{
    BSP_error_t err = UAIR_BSP_BM_PrepareAcquisition();

    if (err == BSP_ERROR_NONE)
    {
        if (UAIR_BSP_BM_StartMeasure( &battery_data_ready )!=BSP_ERROR_NONE)
        {
            UAIR_BSP_BM_EndAcquisition();
            UAIR_BSP_BM_DisableBatteryRead();
        }
    } else {
        UAIR_BSP_BM_DisableBatteryRead();
    }
}

static void battery_enable_timer_event(void __attribute__((unused)) *data)
{
    UAIR_BSP_BM_EnableBatteryRead();
    // Allow for battery signal to settle
    UTIL_TIMER_SetPeriod(&s_battery_measure_timer, BATTERY_TIME_SETTLE);
    UTIL_TIMER_Start(&s_battery_measure_timer);
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

    for (unsigned sensor = 0; sensor < HWD_SENSOR_UNIT_SIZE; sensor++) {
        s_sensor_measuring_times[sensor] = -1;
    }

    s_sensor_measure_state = SENSOR_MEASURE_STATE_IDLE;

    UTIL_TIMER_Create(&s_measure_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, on_measure_timer_event, NULL);
    UTIL_TIMER_SetPeriod(&s_measure_timer, TEMP_HUM_SAMPLING_INTERVAL_MS);
    UTIL_TIMER_Start(&s_measure_timer);

    UTIL_TIMER_Create(&s_battery_enable_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, &battery_enable_timer_event, NULL);
    UTIL_TIMER_Create(&s_battery_measure_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, &battery_measure_timer_event, NULL);

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

    case SENSOR_ID_BATTERY:
        measurement = SENSOR_MEASUREMENT_BATTERY;
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

    case SENSOR_ID_BATTERY:
        if (-1 == has_valid_sample(SENSOR_MEASUREMENT_BATTERY))
            return SENSORS_OP_FAIL;

        *value = s_sensor_data[SENSOR_MEASUREMENT_BATTERY].value_current;
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

static bool internal_temp_hum_enabled_at_tick(uint32_t ticks)
{
    // Approx. every 64 seconds (32 ticks)
    return (ticks % 32) == 0;
}

static bool microphone_enabled_at_tick(uint32_t ticks)
{
    return 1;
}

static BSP_error_t microphone_start_measurement()
{
    return BSP_ERROR_NONE;
}

static BSP_error_t microphone_read_measure(void)
{
    uint8_t gain;

    BSP_error_t err = BSP_ERROR_NONE;
    if (BSP_microphone_get_sensor_state() == SENSOR_AVAILABLE) {
        err = BSP_microphone_read_gain(&gain);
        if (err== BSP_ERROR_NONE)
            process_new_value(SENSOR_MEASUREMENT_SOUND, (MICROPHONE_MAX_GAIN - gain) * 1000);
    }
    return err;
}

static unsigned int microphone_get_measure_delay_us(void)
{
    return 0; // Pre-read
}


static bool external_temp_hum_enabled_at_tick(uint32_t ticks)
{
    // Approx. every 64 seconds (32 ticks)
    return (ticks % 32) == 0;
}

#ifndef OAQ_GEN
# error OAQ generation not defined!
#endif

static bool air_quality_enabled_at_tick(uint32_t ticks)
{
#if OAQ_GEN==1
    // Every 30 ticks (one minute). TBC.
    return (ticks % 30) == 0;
#else
    // Always
    return true;
#endif
}
