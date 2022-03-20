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


static HS300X_t hs300x = {0};
static BSP_sensor_state_t sensor_state = SENSOR_OFFLINE;

enum {
    HS300X_NOT_INIT,
    HS300X_IDLE,
    HS300X_MEASURE
} hs300x_state = HS300X_NOT_INIT;

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

int UAIR_BSP_external_temp_hum_init(BSP_temp_accuracy_t temp_acc, BSP_hum_accuracy_t hum_acc)
{
    HAL_I2C_bus_t bus;
    BSP_I2C_busnumber_t busno;
    
    // TBD: board variations
    BSP_powerzone_t powerzone = UAIR_POWERZONE_NONE;
    BSP_error_t err = BSP_ERROR_NO_INIT;

    do {
        switch (BSP_get_board_version()) {
        case UAIR_NUCLEO_REV1:
            busno = BSP_I2C_BUS0;
            /* Although we have no powerzone, we need to power up
             the I2C pullups */
            powerzone = UAIR_POWERZONE_AMBIENTSENS;
            break;
        case UAIR_NUCLEO_REV2:
            busno = BSP_I2C_BUS2;
            powerzone = UAIR_POWERZONE_AMBIENTSENS;
            break;
        default:
            busno = BSP_I2C_BUS0;
            break;
        }

        // Power down

        if (UAIR_POWERZONE_NONE != powerzone) {
            if (BSP_powerzone_disable(powerzone)!=BSP_ERROR_NONE) {
                err = BSP_ERROR_BUS_FAILURE;
                break;
            }
        }
        /* Initialise bus */

        err = UAIR_BSP_I2C_InitBus(busno);

        if (err!=BSP_ERROR_NONE)
            break;

        bus = UAIR_BSP_I2C_GetHALHandle(busno);

        if (NULL==bus) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        // Power up

        BSP_TRACE("Initializing HS300X sensor");
        if (HS300X_init(&hs300x, bus) != HAL_OK) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }

        BSP_TRACE("Probing HS300X sensor");

        if (UAIR_POWERZONE_NONE != powerzone) {
            if (BSP_powerzone_enable(powerzone)!=BSP_ERROR_NONE) {
                err = BSP_ERROR_BUS_FAILURE;
                break;
            }
        }
        HAL_Delay(5);

        HS300X_accuracy_t hs_temp_acc = UAIR_BSP_BSP_temp_accuracy_to_hs300x(temp_acc);
        HS300X_accuracy_t hs_hum_acc = UAIR_BSP_BSP_hum_accuracy_to_hs300x(hum_acc);

        hs_temp_acc = HS300X_ACCURACY_NONE;
        hs_hum_acc = HS300X_ACCURACY_NONE;

        // Probe
        if (HS300X_probe(&hs300x, hs_hum_acc, hs_temp_acc)!=HAL_OK) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        BSP_TRACE("HS300X sensor detected and initialised (%08x)", HS300X_get_probed_serial(&hs300x));

        hs300x_state = HS300X_IDLE;
        err = BSP_ERROR_NONE;
        sensor_state = SENSOR_AVAILABLE;

    } while (0);
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
        if (hs300x_state==HS300X_MEASURE) {
            BSP_TRACE("Sensor is busy, measuring anyway!");
            //ret = BSP_ERROR_BUSY;
            //break;
        }

        if (HS300X_start_measurement(&hs300x)!=0) {
            BSP_TRACE("Cannot start measure on HS300X");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
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
        if (hs300x_state==HS300X_IDLE) {
            BSP_TRACE("while reading: Sensor is busy!");
            ret = BSP_ERROR_BUSY;
            break;
        }
        if (HS300X_read_measurement(&hs300x, temp, hum, &stale)!=0) {
            BSP_TRACE("Error reading sensor measurement");
            ret = BSP_ERROR_COMPONENT_FAILURE;

            break;
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
