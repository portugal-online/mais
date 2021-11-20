#include "UAIR_BSP_air_quality.h"
#include "pvt/UAIR_BSP_air_quality_p.h"
#include "BSP.h"
#include "HAL_gpio.h"
#include "ZMOD4510.h"
#include "ZMOD4510_OAQ2.h"
#include "pvt/UAIR_BSP_i2c_p.h"

static ZMOD4510_t zmod;
static ZMOD4510_OAQ2_t zmod_oaq;
static BSP_sensor_state_t sensor_state = SENSOR_OFFLINE;

static HAL_GPIODef_t reset_gpio = {
    .port = GPIOC,
    .pin = GPIO_PIN_13,
    .af = 0,
    .clock_control = &HAL_clk_GPIOC_clock_control
};


BSP_error_t UAIR_BSP_air_quality_init()
{
    HAL_I2C_bus_t bus = UAIR_BSP_I2C_GetHALHandle(BSP_I2C_BUS0);

    BSP_error_t err = ZMOD4510_Init(&zmod, bus, &reset_gpio);
    if (err!=BSP_ERROR_NONE)
        return err;
    err = ZMOD4510_Probe(&zmod);

    if (err!=BSP_ERROR_NONE)
        return err;

    err = ZMOD4510_OAQ2_init(&zmod_oaq, ZMOD4510_get_dev(&zmod));

    if (err==BSP_ERROR_NONE) {
        sensor_state = SENSOR_AVAILABLE;
    }
    return err;
}

BSP_error_t BSP_air_quality_start_measurement()
{
    if (ZMOD4510_start_measurement(&zmod)==ZMOD4510_OP_SUCCESS)
        return BSP_ERROR_NONE;
    return BSP_ERROR_COMPONENT_FAILURE;
}

BSP_error_t BSP_air_quality_measurement_completed()
{
    ZMOD4510_op_result_t r = ZMOD4510_is_sequencer_completed(&zmod);
    if (r==ZMOD4510_OP_BUSY) {
        return BSP_ERROR_BUSY;
    }
    return BSP_ERROR_NONE;
}

static unsigned sampleno = 0;

BSP_error_t BSP_air_quality_calculate(const float temp_c,
                                      const float hum_pct,
                                      BSP_air_quality_results_t *results)
{
    oaq_2nd_gen_results_t libresult;
    BSP_error_t err;

    ZMOD4510_op_result_t r = ZMOD4510_read_adc(&zmod);

    if (r!=ZMOD4510_OP_SUCCESS)
        return BSP_ERROR_COMPONENT_FAILURE;
    BSP_TRACE("Aquired sample no %d (901 needed for cal.)", sampleno);
    sampleno++;

    UAIR_BSP_DP_On(DEBUG_PIN3);

    ZMOD4510_OAQ2_error_t oaqerr = ZMOD4510_OAQ2_calculate(&zmod_oaq,
                                                           ZMOD4510_get_adc(&zmod),
                                                           hum_pct,
                                                           temp_c, &libresult);
    UAIR_BSP_DP_Off(DEBUG_PIN3);

    switch (oaqerr) {

    case ZMOD4510_OAQ2_ERROR_STABILIZING:
        BSP_TRACE("Sensor stabilizing");
        err = BSP_ERROR_BUSY;
        break;

    case ZMOD4510_OAQ2_NO_ERROR:
        results->O3_conc_ppb = libresult.O3_conc_ppb;
        results->FAST_AQI    = libresult.FAST_AQI;
        results->EPA_AQI     = libresult.EPA_AQI;
        err = BSP_ERROR_NONE;
        break;
    default:
        err = BSP_ERROR_COMPONENT_FAILURE;
        break;
    }
    return err;
}

BSP_sensor_state_t BSP_air_quality_get_sensor_state(void)
{
    return sensor_state;
}

unsigned int BSP_air_quality_get_measure_delay_us(void)
{
    // TBD: Unknown for now. We use a boilerplate value
    return 64000;
}
