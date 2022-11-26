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
 * @file UAIR_BSP_internaltemp.c
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_INTERNAL_TEMP
 *
 * uAir interfacing to internal temperature/humidity sensor implementation
 *
 */
#include "BSP.h"
#include "SHTC3.h"

#include "UAIR_BSP_internaltemp.h"
#include "pvt/UAIR_BSP_internaltemp_p.h"
#include "UAIR_BSP_i2c.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "UAIR_sensor.h"

/* SHTC3 temp. sensor */
static SHTC3_t shtc3 = {0};
static BSP_sensor_state_t sensor_state = SENSOR_OFFLINE;

enum {
    SHTC3_NOT_INIT,
    SHTC3_IDLE,
    SHTC3_MEASURE
} shtc3_state = SHTC3_NOT_INIT;

static BSP_powerzone_t UAIR_BSP_internal_temp_hum_get_powerzone()
{
    return UAIR_POWERZONE_INTERNALI2C;
}

static BSP_I2C_busnumber_t UAIR_BSP_internal_temp_hum_get_bus()
{
    return BSP_I2C_BUS0;
}


static void UAIR_BSP_internal_temp_hum_set_faulty()
{
    UAIR_BSP_internal_temp_hum_deinit();
    shtc3_state = SHTC3_NOT_INIT;
    sensor_state = SENSOR_FAULTY;
}

UAIR_sensor_ops_t internal_sensor_ops = {
    .init = UAIR_BSP_internal_temp_hum_init,
    .deinit = UAIR_BSP_internal_temp_hum_deinit,
    .reset = NULL,
    .get_powerzone = UAIR_BSP_internal_temp_hum_get_powerzone,
    .get_bus = UAIR_BSP_internal_temp_hum_get_bus,
    .set_faulty = UAIR_BSP_internal_temp_hum_set_faulty
};

static UAIR_sensor_t internal_sensor = {
    .ops = &internal_sensor_ops,
    .failcount = 0
};

void UAIR_BSP_internal_temp_hum_powerzone_changed(void *userdata, const powerstate_t state)
{
    if ( state == POWER_OFF ) {
        BSP_TRACE("Internal temp/hum powered down");
        shtc3_state = SHTC3_NOT_INIT;
        sensor_state = SENSOR_OFFLINE;
    }
}

void UAIR_BSP_internal_temp_hum_deinit()
{
    if (sensor_state == SENSOR_AVAILABLE)
        BSP_powerzone_unref(UAIR_BSP_internal_temp_hum_get_powerzone());

    sensor_state = SENSOR_OFFLINE;
    shtc3_state = SHTC3_NOT_INIT;
}

static BSP_error_t UAIR_BSP_internal_temp_hum_init_i2c()
{
    BSP_error_t err;
    HAL_I2C_bus_t bus;

    // We need at least 240us for power wakeup
    BSP_delay_us(240);

    /* Initialise bus */

    err = UAIR_BSP_I2C_InitBus( UAIR_BSP_internal_temp_hum_get_bus() );

    if (err!=BSP_ERROR_NONE) {
        return err;
    }

    bus = UAIR_BSP_I2C_GetHALHandle( UAIR_BSP_internal_temp_hum_get_bus() );

    if (NULL==bus) {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    BSP_TRACE("Initializing SHTC3 sensor");
    if (SHTC3_init(&shtc3, bus, 200) != SHTC3_STATUS_OK) {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    BSP_TRACE("Probing SHTC3 sensor");
    // Probe
    if (SHTC3_probe(&shtc3)!=SHTC3_STATUS_OK) {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    if (SHTC3_sleep(&shtc3)!=SHTC3_STATUS_OK) {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_internal_temp_hum_init()
{
    BSP_error_t err = BSP_ERROR_NO_INIT;

    do {
        // Power up if required

        if (BSP_powerzone_ref( UAIR_BSP_internal_temp_hum_get_powerzone() )!=BSP_ERROR_NONE) {
            err = BSP_ERROR_BUS_FAILURE;
            break;
        }

        err = UAIR_BSP_internal_temp_hum_init_i2c();

        if (err == BSP_ERROR_NONE)
        {

            BSP_TRACE("SHTC3 sensor detected and initialised (%08x)", SHTC3_get_probed_serial(&shtc3));

            shtc3_state = SHTC3_IDLE;
            sensor_state = SENSOR_AVAILABLE;

        } else {
            BSP_powerzone_unref( UAIR_BSP_internal_temp_hum_get_powerzone() );
            sensor_state = SENSOR_OFFLINE;
        }
    } while (0);
    return err;
}



static BSP_error_t UAIR_BSP_internal_temp_hum_start_measure(void)
{
    BSP_error_t ret;
    //BSP_TRACE("Starting internal temp measure");
    do {
        if (shtc3_state==SHTC3_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
#ifdef UAIR_BSP_CHECK_SENSOR_STATE
        if (shtc3_state==SHTC3_MEASURE) {
            ret = BSP_ERROR_BUSY;
            break;
        }
#endif
        // start measure
        if (SHTC3_wake_up(&shtc3)!=0) {
            BSP_TRACE("Cannot wakeup SHTC3");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }

        if (SHTC3_measure(&shtc3)!=0) {
            BSP_TRACE("Cannot start measure on SHTC3");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }

        shtc3_state = SHTC3_MEASURE;
        ret = BSP_ERROR_NONE;

    } while (0);

    return ret;
}

/**
 * @ingroup UAIR_BSP_SENSOR_INTERNAL_TEMP
 * @brief Start internal temperature/humidity measurement
 *
 *
 * @return \ref BSP_ERROR_NONE if successful
 * @return \ref BSP_ERROR_NO_INIT if sensor was not successfully initialised
 * @return \ref BSP_ERROR_BUSY if sensor is still pending a read from a previous start measure
 * @return \ref BSP_ERROR_COMPONENT_FAILURE if any communication error occured. Action TBD
 */
BSP_error_t BSP_internal_temp_hum_start_measure(void)
{
    BSP_error_t err = UAIR_BSP_internal_temp_hum_start_measure();

    if (err==BSP_ERROR_NONE)
    {
        UAIR_sensor_ok(&internal_sensor);
    } else if (err==BSP_ERROR_COMPONENT_FAILURE) {
        UAIR_sensor_fault_detected(&internal_sensor);
    }
    return err;
}

static BSP_error_t UAIR_BSP_internal_temp_hum_read_measure(int32_t *temp, int32_t *hum)
{
    BSP_error_t ret;

    do {
        if (shtc3_state==SHTC3_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (shtc3_state==SHTC3_IDLE) {
            BSP_TRACE("SHTC3 idle!");
            ret = BSP_ERROR_BUSY;
            break;
        }
        if (SHTC3_read(&shtc3, temp, hum)!=0) {
            BSP_TRACE("Cannot read measure on SHTC3");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }
        if (SHTC3_sleep(&shtc3)!=0) {
            BSP_TRACE("Cannot sleep SHTC3");

            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }
        shtc3_state = SHTC3_IDLE;
        ret = BSP_ERROR_NONE;
    } while (0);

    return ret;
}

/**
 * @ingroup UAIR_BSP_SENSOR_INTERNAL_TEMP
 * @brief Read temperature/humidity measurement (previously started)
 *
 *
 * The values are stored in the temp and hum parameters.
 *
 * The value stored in temp is in milli degrees celsius. For example:
 * - A value of "1000" corresponds to 1 degree C
 * - A value of "23920" corresponds to 23.92 degrees celsius
 * - A value of "-8100" corresponds to -8.10 degrees celsius
 *
 * The value stored in hum is in milli percent. For example:
 * - A value of "1000" corresponds to 1% humidity.
 * - A value of "87230" corresponds to 87.23% humidity.
 *
 * @param temp Location where to store the temperature (in milli degrees centigrade)
 * @param hum Location where to store the humidity (in milli percent)
 *
 * @return \ref BSP_ERROR_NONE if successful
 * @return \ref BSP_ERROR_NO_INIT if sensor was not successfully initialised
 * @return \ref BSP_ERROR_BUSY if sensor is not currently measuring, if sensor reported stale data or
 * if the time interval between start of measure and the readout has not been observed
 * @return \ref BSP_ERROR_COMPONENT_FAILURE if any communication error occured. Action TBD
 */
BSP_error_t BSP_internal_temp_hum_read_measure(int32_t *temp, int32_t *hum)
{
    BSP_error_t err = UAIR_BSP_internal_temp_hum_read_measure(temp, hum);

    if (err==BSP_ERROR_NONE)
    {
        UAIR_sensor_ok(&internal_sensor);
    } else if (err==BSP_ERROR_COMPONENT_FAILURE) {
        UAIR_sensor_fault_detected(&internal_sensor);
    }

    return err;
}


/**
 * @ingroup UAIR_BSP_SENSOR_INTERNAL_TEMP
 * @brief Get internal temperature/humidity sensor measure delay
 *
 *
 * A delay is required between starting a sensor measure and performing the read.
 * This delay varies according to the resolution used.
 * This method allows the application to understand the minimum time interval
 * required between the start of measure
 *
 * @return The required delay (in microseconds) between start of measure and sensor readout
 */

unsigned int BSP_internal_temp_hum_get_measure_delay_us(void)
{
    return 12100;  // 12.1ms max in normal mode.
}

/**
 * @brief Get internal temperature/humidity sensor state
 * @ingroup UAIR_BSP_SENSOR_INTERNAL_TEMP
 *
 * @return \ref SENSOR_AVAILABLE If the internal temperature/humidity sensor is available and working.
 * @return \ref SENSOR_OFFLINE   If the internal temperature/humidity sensor is currently offline. Measurements should not be started or processed.
 * @return \ref SENSOR_FAULTY    If the internal temperature/humidity sensor is deemed faulty. Measurements should not be started or processed.
 */
BSP_sensor_state_t BSP_internal_temp_hum_get_sensor_state(void)
{
    return sensor_state;
}

