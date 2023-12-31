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
 * @file UAIR_BSP_air_quality.c
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 * uAir interfacing to air quality sensor implementation
 */

#include "UAIR_BSP_air_quality.h"
#include "pvt/UAIR_BSP_air_quality_p.h"
#include "BSP.h"
#include "HAL_gpio.h"
#include "ZMOD4510.h"
#include "ZMOD4510_OAQ2.h"
#include "ZMOD4510_OAQ1.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "UAIR_sensor.h"

static ZMOD4510_t zmod;

#if OAQ_GEN==2
static ZMOD4510_OAQ2_t zmod_oaq;
#else
static ZMOD4510_OAQ1_t zmod_oaq;
#endif

static BSP_sensor_state_t sensor_state = SENSOR_OFFLINE;
static unsigned sampleno = 0;

static HAL_GPIODef_t reset_gpio = {
    .port = GPIOC,
    .pin = GPIO_PIN_13,
    .af = 0,
    .clock_control = &HAL_clk_GPIOC_clock_control
};

static BSP_I2C_busnumber_t UAIR_BSP_air_quality_get_bus(void);
static BSP_powerzone_t UAIR_BSP_air_quality_get_powerzone(void);
static void UAIR_BSP_air_quality_set_faulty(void);

UAIR_sensor_ops_t air_quality_sensor_ops = {
    .init = UAIR_BSP_air_quality_init,
    .deinit = UAIR_BSP_air_quality_deinit,
    .reset = NULL,
    .get_powerzone = UAIR_BSP_air_quality_get_powerzone,
    .get_bus = UAIR_BSP_air_quality_get_bus,
    .set_faulty = UAIR_BSP_air_quality_set_faulty
};

static UAIR_sensor_t air_quality_sensor = {
    .ops = &air_quality_sensor_ops,
    .failcount = 0
};

static BSP_powerzone_t UAIR_BSP_air_quality_get_powerzone()
{
    BSP_powerzone_t powerzone;

    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        powerzone = UAIR_POWERZONE_AMBIENTSENS;
        break;
    case UAIR_NUCLEO_REV2:
        powerzone = UAIR_POWERZONE_AMBIENTSENS;
        break;
    default:
        break;
    }
    return powerzone;
}

static BSP_I2C_busnumber_t UAIR_BSP_air_quality_get_bus()
{
    BSP_I2C_busnumber_t i2c_busno = BSP_I2C_BUS_NONE;

    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        i2c_busno = BSP_I2C_BUS0;
        break;
    case UAIR_NUCLEO_REV2:
        i2c_busno = BSP_I2C_BUS2;
        break;
    default:
        break;
    }
    return i2c_busno;
}


void UAIR_BSP_air_quality_powerzone_changed(void *userdata, const powerstate_t state)
{
    ZMOD4510_t *z = (ZMOD4510_t*)userdata;
    if ( state == POWER_OFF ) {
        ZMOD4510_deinit(z);
    }
}

ZMOD4510_t *UAIR_BSP_air_quality_get_zmod(void)
{
    return &zmod;
}

BSP_error_t UAIR_BSP_air_quality_init_i2c()
{
    BSP_error_t err;

    err = UAIR_BSP_I2C_Bus_Ref(UAIR_BSP_air_quality_get_bus());

    if (err!=BSP_ERROR_NONE) {
        BSP_TRACE("Cannot initialise I2C bus");
        return err;
    }

    HAL_I2C_bus_t bus = UAIR_BSP_I2C_GetHALHandle(UAIR_BSP_air_quality_get_bus());

    if (bus == NULL) {
        BSP_TRACE("Cannot initialise I2C bus");
        return err;
    }

    err = ZMOD4510_Init(&zmod, bus, &reset_gpio);

    if (err!=BSP_ERROR_NONE)
    {
        BSP_TRACE("Cannot init ZMOD");
        return err;
    }

    err = ZMOD4510_Probe(&zmod);

    if (err!=BSP_ERROR_NONE)
    {
        BSP_TRACE("Cannot probe ZMOD");
        return err;
    }
#if OAQ_GEN==2
    err = ZMOD4510_OAQ2_init(&zmod_oaq, ZMOD4510_get_dev(&zmod));
#else
    err = ZMOD4510_OAQ1_init(&zmod_oaq, ZMOD4510_get_dev(&zmod));
#endif
    return err;
}

BSP_error_t UAIR_BSP_air_quality_init()
{
    BSP_error_t err;

    err = BSP_powerzone_ref(UAIR_BSP_air_quality_get_powerzone());

    if (err == BSP_ERROR_NONE)
    {
        // Let power settle.
        HAL_Delay(200);
        err = UAIR_BSP_air_quality_init_i2c();
        if (err==BSP_ERROR_NONE) {
            sensor_state = SENSOR_AVAILABLE;
        } else {
            BSP_TRACE("Cannot init ZMOD");
            BSP_powerzone_unref(UAIR_BSP_air_quality_get_powerzone());
        }
    } else {
        BSP_TRACE("Cannot init powerzone");
    }
    return err;
}

/**
 * @brief Verify if the air quality measurement has completed.
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 *
 * This function should only be used to confirm if the measurements are complete. It's not intended
 * to be used on a busy-loop wait for the completion. Please refer to \ref BSP_air_quality_get_measure_delay_us about how to
 * obtain the nominal time required for a measurement.
 *
 * @return \ref BSP_ERROR_NONE if measurement has completed (or never started)
 * @return \ref BSP_ERROR_BUSY if measurement is still being performed.
 *
 */
BSP_error_t BSP_air_quality_measurement_completed()
{
    ZMOD4510_op_result_t r = ZMOD4510_is_sequencer_completed(&zmod);

    if (r==ZMOD4510_OP_BUSY) {
        BSP_TRACE("Sequencer has not completed!");
        return BSP_ERROR_BUSY;
    }
    return BSP_ERROR_NONE;
}

static BSP_error_t UAIR_BSP_air_quality_zmod_op(ZMOD4510_op_result_t (*op)(ZMOD4510_t *zmod), const char *opname)
{
    ZMOD4510_op_result_t r;
    int retries=1;
    BSP_error_t err = BSP_ERROR_NONE;

    do {
        r = op(&zmod);
        if (r!=ZMOD4510_OP_SUCCESS) {
            BSP_TRACE("ZMOD: op '%s' returned %d", opname, r);
        }
        switch (r) {
        case ZMOD4510_OP_DEVICE_ERROR:

            UAIR_sensor_fault_detected(&air_quality_sensor);
            err = BSP_ERROR_COMPONENT_FAILURE;
            break;
        case ZMOD4510_OP_BUSY:
            err = BSP_ERROR_BUSY;
            break;
        case ZMOD4510_OP_SUCCESS:
            UAIR_sensor_ok(&air_quality_sensor);
            break;
        default:
            UAIR_sensor_fault_detected(&air_quality_sensor);
            err = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }
    } while ((err!=BSP_ERROR_NONE) && (retries--));

    return err;
}

#define ZMOD_OP(x) \
    UAIR_BSP_air_quality_zmod_op( &x, #x )

// TODO: this should be static
BSP_error_t UAIR_BSP_air_quality_read_adc(void)
{
    BSP_error_t err = ZMOD_OP( ZMOD4510_read_adc );
    return err;
}

// TODO: this should be static
BSP_error_t UAIR_BSP_air_quality_sequencer_completed(void)
{
    BSP_error_t err = ZMOD_OP( ZMOD4510_is_sequencer_completed );
    return err;
}

/**
 * @brief Start air quality measurement
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 *
 * @return \ref BSP_ERROR_NONE if measurement started successfully.
 * @return \ref BSP_ERROR_COMPONENT_FAILURE if a device error was detected. Actions to be performed TBD.
 * @return \ref BSP_ERROR_BUSY is the device is still performing a measurement. This can happen if a measurement was started before
 *         a previous measurement completed.
 */
BSP_error_t BSP_air_quality_start_measurement()
{
    BSP_error_t err = UAIR_BSP_air_quality_sequencer_completed();
    if (err==BSP_ERROR_NONE) {
        err = ZMOD_OP( ZMOD4510_start_measurement );
    } else {
        BSP_TRACE("Attempting to start sequencer in busy mode!");
    }
    return err;
}

static inline uint16_t float_to_aqi(float aqi)
{
    if (!isnormal(aqi))
        return 511;

    if (aqi<0.0F)
        aqi = 0.0F;
    if (aqi>500.0F)
        aqi = 500.0F;
    aqi = roundf(aqi);
    return (uint16_t)aqi;
}

/**
 * @brief Calculate OAQ values
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 *
 * @param temp_c Outside temperature in Degrees Celsius
 * @param hum_pct Outside humidity percentage (0.0F to 100.0F)
 * @param results Structure to hold OAQ results
 *
 * @return BSP_ERROR_NONE when the OAQ calculation was successfuly performed
 * @return BSP_ERROR_BUSY if the sensor is still performing a measurement
 * @return BSP_ERROR_COMPONENT_FAILURE if a device error was detected. Actions to be performed TBD.
 * @return BSP_ERROR_CALIBRATING if the sensor was processed OK, but the sensor it still stabilizing.
 */

BSP_error_t BSP_air_quality_calculate(const float temp_c,
                                      const float hum_pct,
                                      BSP_air_quality_results_t *results)
{
    BSP_error_t err;

    err = UAIR_BSP_air_quality_sequencer_completed();

    if (err != BSP_ERROR_NONE)
        return err;


    err = UAIR_BSP_air_quality_read_adc();

    if (err != BSP_ERROR_NONE) {
        UAIR_sensor_fault_detected(&air_quality_sensor);
        return err;
    }
    UAIR_sensor_ok(&air_quality_sensor);


    BSP_TRACE("Aquired sample no %d (901 needed for cal.)", sampleno);
    sampleno++;

#if (!defined(RELEASE)) || (RELEASE==0)
    {
        char tmp[128];
        int i;
        char *p = tmp;
        for (i=0; i<ZMOD4510_ADC_DATA_LEN;i++) {
            p += tiny_sprintf(p, "%02x ", zmod.adc_result[i]);
        }
        BSP_TRACE("Sensor data: [%s]", tmp);
    }

#endif


#if OAQ_GEN==2
    oaq_2nd_gen_results_t libresult;

    //UAIR_BSP_DP_On(DEBUG_PIN3);

    ZMOD4510_OAQ2_error_t oaqerr = ZMOD4510_OAQ2_calculate(&zmod_oaq,
                                                           ZMOD4510_get_adc(&zmod),
                                                           hum_pct,
                                                           temp_c, &libresult);
    //UAIR_BSP_DP_Off(DEBUG_PIN3);

    switch (oaqerr) {

    case ZMOD4510_OAQ2_ERROR_STABILIZING:
        BSP_TRACE("Sensor stabilizing");
        err = BSP_ERROR_CALIBRATING;
        break;

    case ZMOD4510_OAQ2_NO_ERROR:

        results->O3_conc_ppb = libresult.O3_conc_ppb;
        results->NO2_conc_ppb = NAN;
        results->FAST_AQI    = libresult.FAST_AQI;
        results->EPA_AQI     = libresult.EPA_AQI;
        err = BSP_ERROR_NONE;
        break;
    default:
        err = BSP_ERROR_COMPONENT_FAILURE;
        break;
    }
    return err;
#else
    oaq_1st_gen_results_t libresult;

    ZMOD4510_OAQ1_error_t oaqerr = ZMOD4510_OAQ1_calculate(&zmod_oaq,
                                                           ZMOD4510_get_adc(&zmod),
                                                           &libresult);

    switch (oaqerr) {

    case ZMOD4510_OAQ1_ERROR_STABILIZING:
        BSP_TRACE("Sensor stabilizing");
        err = BSP_ERROR_CALIBRATING;
        break;

    case ZMOD4510_OAQ1_NO_ERROR:

        results->O3_conc_ppb = libresult.conc_o3;
        results->NO2_conc_ppb = libresult.conc_no2;
        results->FAST_AQI    = float_to_aqi(libresult.aqi_o3 > libresult.aqi_no2? libresult.aqi_o3:libresult.aqi_no2);
        results->EPA_AQI     = float_to_aqi(libresult.EPA_AQI);

        err = BSP_ERROR_NONE;
        break;
    default:
        err = BSP_ERROR_COMPONENT_FAILURE;
        break;
    }
    return err;
#endif
}

/**
 * @brief Get OAQ sensor state
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 *
 * @return \ref SENSOR_AVAILABLE If the OAQ sensor is available and working.
 * @return \ref SENSOR_OFFLINE   If the OAQ sensor is currently offline. Measurements should not be started or processed.
 * @return \ref SENSOR_FAULTY    If the OAQ sensor is deemed faulty. Measurements should not be started or processed.
 */
BSP_sensor_state_t BSP_air_quality_get_sensor_state(void)
{
    return sensor_state;
}

/**
 * @brief Get OAQ measurement delay
 * @ingroup UAIR_BSP_SENSOR_AIR_QUALITY
 *
 *
 * Return the OAQ measurement delay, i.e., the delay required between starting a measurement and processing the data
 * from that measure.
 *
 *
 * @return The delay in microseconds
 */
unsigned int BSP_air_quality_get_measure_delay_us(void)
{
    // TBD: Unknown for now. We use a boilerplate value
#if OAQ_GEN==2
    return 64000;
#else
    return 0;
#endif
}

static void UAIR_BSP_air_quality_set_faulty(void)
{
    UAIR_BSP_air_quality_deinit();
    sensor_state = SENSOR_FAULTY;
}

void UAIR_BSP_air_quality_deinit()
{
    if (sensor_state == SENSOR_AVAILABLE)
        BSP_powerzone_unref(UAIR_BSP_air_quality_get_powerzone());

    sensor_state = SENSOR_OFFLINE;
}
