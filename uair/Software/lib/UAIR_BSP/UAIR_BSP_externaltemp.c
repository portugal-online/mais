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
 * @file UAIR_BSP_externaltemp.c
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_EXTERNAL_TEMP
 *
 * uAir interfacing to external temperature/humidity sensor implementation
 *
 */

#include "BSP.h"
#include "HS300X.h"

#include "UAIR_BSP_externaltemp.h"
#include "pvt/UAIR_BSP_externaltemp_p.h"
#include "UAIR_BSP_i2c.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "UAIR_sensor.h"

static HS300X_t hs300x = {0};
static BSP_sensor_state_t sensor_state = SENSOR_OFFLINE;
static BSP_temp_accuracy_t sensor_temp_acc = TEMP_ACCURACY_HIGH;
static BSP_hum_accuracy_t sensor_hum_acc = HUM_ACCURACY_HIGH;

enum {
    HS300X_NOT_INIT,
    HS300X_IDLE,
    HS300X_MEASURE
} hs300x_state = HS300X_NOT_INIT;


static BSP_powerzone_t UAIR_BSP_external_temp_hum_get_powerzone(void);
static BSP_I2C_busnumber_t UAIR_BSP_external_temp_hum_get_bus(void);



static inline HS300X_accuracy_t UAIR_BSP_BSP_temp_accuracy_to_hs300x(const BSP_temp_accuracy_t t)
{
    HS300X_accuracy_t acc;
    switch (t) {
    case TEMP_ACCURACY_LOW:
        acc = HS300X_ACCURACY_8BIT;
        break;
    case TEMP_ACCURACY_MED:
        acc = HS300X_ACCURACY_10BIT;
        break;
    case TEMP_ACCURACY_HIGH:
        acc = HS300X_ACCURACY_12BIT;
        break;
    default:
        acc = HS300X_ACCURACY_8BIT;
        break;
    }
    return acc;
}

static inline HS300X_accuracy_t UAIR_BSP_BSP_hum_accuracy_to_hs300x(const BSP_hum_accuracy_t t)
{
    HS300X_accuracy_t acc;
    switch (t) {
    case HUM_ACCURACY_LOW:
        acc = HS300X_ACCURACY_8BIT;
        break;
    case HUM_ACCURACY_MED:
        acc = HS300X_ACCURACY_10BIT;
        break;
    case HUM_ACCURACY_HIGH:
        acc = HS300X_ACCURACY_12BIT;
        break;
    default:
        acc = HS300X_ACCURACY_8BIT;
        break;
    }
    return acc;

}

UAIR_sensor_ops_t external_sensor_ops = {
    .init = UAIR_BSP_external_temp_hum_init,
    .deinit = UAIR_BSP_external_temp_hum_deinit,
    .reset = NULL,
    .get_powerzone = UAIR_BSP_external_temp_hum_get_powerzone,
    .get_bus = UAIR_BSP_external_temp_hum_get_bus,
    .set_faulty = UAIR_BSP_external_temp_hum_set_faulty
};

static UAIR_sensor_t external_sensor = {
    .ops = &external_sensor_ops,
    .failcount = 0
};

void UAIR_BSP_external_temp_set_defaults(BSP_temp_accuracy_t temp_acc, BSP_hum_accuracy_t hum_acc)
{
    sensor_temp_acc = temp_acc;
    sensor_hum_acc = hum_acc;
}

void UAIR_BSP_external_temp_hum_deinit(void)
{
    if (sensor_state == SENSOR_AVAILABLE)
         BSP_powerzone_unref(UAIR_BSP_external_temp_hum_get_powerzone());

    sensor_state = SENSOR_OFFLINE;
    hs300x_state = HS300X_NOT_INIT;

}

static BSP_powerzone_t UAIR_BSP_external_temp_hum_get_powerzone()
{
    return UAIR_POWERZONE_AMBIENTSENS;
}

static BSP_I2C_busnumber_t UAIR_BSP_external_temp_hum_get_bus()
{
    BSP_I2C_busnumber_t busno;

    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        busno = BSP_I2C_BUS0;
        break;
    case UAIR_NUCLEO_REV2:
        busno = BSP_I2C_BUS2;
        break;
    default:
        busno = BSP_I2C_BUS0;
        break;
    }
    return busno;
}

static BSP_error_t UAIR_BSP_external_temp_hum_init_i2c(void)
{
    BSP_error_t err;
    HAL_I2C_bus_t bus;

    /* Initialise bus */

    err = UAIR_BSP_I2C_InitBus(UAIR_BSP_external_temp_hum_get_bus());

    if (err!=BSP_ERROR_NONE) {
        BSP_TRACE("Cannot initialise I2C bus");
        return err;
    }

    bus = UAIR_BSP_I2C_GetHALHandle(UAIR_BSP_external_temp_hum_get_bus());

    if (NULL==bus) {
        BSP_TRACE("Cannot get HAL handle!");
        return BSP_ERROR_PERIPH_FAILURE;
    }

    if (HS300X_init(&hs300x, bus) != HAL_OK) {
        BSP_TRACE("Error initialising HS300X sensor!");
        return BSP_ERROR_PERIPH_FAILURE;
    }

    //HAL_Delay();

    HS300X_accuracy_t hs_temp_acc = UAIR_BSP_BSP_temp_accuracy_to_hs300x(sensor_temp_acc);
    HS300X_accuracy_t hs_hum_acc = UAIR_BSP_BSP_hum_accuracy_to_hs300x(sensor_hum_acc);

    hs_temp_acc = HS300X_ACCURACY_NONE;
    hs_hum_acc = HS300X_ACCURACY_NONE;

    // Probe
    if (HS300X_probe(&hs300x, hs_hum_acc, hs_temp_acc)!=HAL_OK) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        BSP_TRACE("HS300X sensor detected and initialised (%08x)", HS300X_get_probed_serial(&hs300x));
    }
    return BSP_ERROR_NONE;
}

int UAIR_BSP_external_temp_hum_init(void)
{
    BSP_error_t err = BSP_ERROR_NO_INIT;

    err = BSP_powerzone_ref(UAIR_BSP_external_temp_hum_get_powerzone());

    if (err != BSP_ERROR_NONE)
    {
        BSP_TRACE("Cannot initialize powerzone");
        return err;
    }

    /*
     // Force power cycle, so we can enter program mode.
    err = UAIR_BSP_powerzone_cycle(UAIR_BSP_external_temp_hum_get_powerzone());

    if (err != BSP_ERROR_NONE)
    {
        BSP_TRACE("Cannot power cycle HS300X");
        return err;
    }
    */



    err = UAIR_BSP_external_temp_hum_init_i2c();

    if (err==BSP_ERROR_NONE)
    {
        hs300x_state = HS300X_IDLE;
        sensor_state = SENSOR_AVAILABLE;
    } else {
        BSP_TRACE("Cannot initialize HS300X");
        BSP_powerzone_unref(UAIR_BSP_external_temp_hum_get_powerzone());
        sensor_state = SENSOR_OFFLINE;
    }

    return err;
}

/**
 * @ingroup UAIR_BSP_SENSOR_EXTERNAL_TEMP
 * @brief Start external temperature/humidity measurement
 *
 *
 * @return \ref BSP_ERROR_NONE if successful
 * @return \ref BSP_ERROR_NO_INIT if sensor was not successfully initialised
 * @return \ref BSP_ERROR_BUSY if sensor is still pending a read from a previous start measure
 * @return \ref BSP_ERROR_COMPONENT_FAILURE if any communication error occured. Action TBD
 */
BSP_error_t BSP_external_temp_hum_start_measure(void)
{
    BSP_error_t ret;
    //BSP_TRACE("Starting external temp measure");
    do {
        if (hs300x_state==HS300X_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (HS300X_start_measurement(&hs300x)!=0) {
            BSP_TRACE("Cannot start measure on HS300X");
            UAIR_sensor_fault_detected(&external_sensor);
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        } else {
            UAIR_sensor_ok(&external_sensor);
        }

        hs300x_state = HS300X_MEASURE;
        ret = BSP_ERROR_NONE;

    } while (0);

    return ret;
}

/**
 * @ingroup UAIR_BSP_SENSOR_EXTERNAL_TEMP
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
 * @return \ref BSP_ERROR_COMPONENT_FAILURE if any communication error occured
 */
BSP_error_t BSP_external_temp_hum_read_measure(int32_t *temp, int32_t *hum)
{
    BSP_error_t ret;
   // BSP_TRACE("Reading external temp measure");

    int stale;
    do {
        if (hs300x_state==HS300X_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (HS300X_read_measurement(&hs300x, temp, hum, &stale)!=0) {
            BSP_TRACE("Error reading sensor measurement");
            UAIR_sensor_fault_detected(&external_sensor);

            ret = BSP_ERROR_COMPONENT_FAILURE;

            break;
        } else {
            UAIR_sensor_ok(&external_sensor);
        }

        hs300x_state = HS300X_IDLE;
        if (stale)
            ret = BSP_ERROR_BUSY;
        else
            ret = BSP_ERROR_NONE;
    } while (0);
    return ret;
}

/**
 * @ingroup UAIR_BSP_SENSOR_EXTERNAL_TEMP
 * @brief Get external temperature/humidity sensor measure delay
 *
 *
 * A delay is required between starting a sensor measure and performing the read.
 * This delay varies according to the resolution used.
 * This method allows the application to understand the minimum time interval
 * required between the start of measure
 *
 * @return The required delay (in microseconds) between start of measure and sensor readout
 */
unsigned int BSP_external_temp_hum_get_measure_delay_us(void)
{
    return HS300X_time_for_measurement_us(hs300x.temp_acc, hs300x.hum_acc);

}

/**
 * @brief Get external temperature/humidity sensor state
 * @ingroup UAIR_BSP_SENSOR_EXTERNAL_TEMP
 *
 * @return \ref SENSOR_AVAILABLE If the external temperature/humidity sensor is available and working.
 * @return \ref SENSOR_OFFLINE   If the external temperature/humidity sensor is currently offline. Measurements should not be started or processed.
 * @return \ref SENSOR_FAULTY    If the external temperature/humidity sensor is deemed faulty. Measurements should not be started or processed.
 */
BSP_sensor_state_t BSP_external_temp_hum_get_sensor_state(void)
{
    return sensor_state;
}

void UAIR_BSP_external_temp_hum_set_faulty(void)
{
    sensor_state = SENSOR_FAULTY;
}
