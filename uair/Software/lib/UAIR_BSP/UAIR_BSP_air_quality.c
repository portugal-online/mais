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
static unsigned sampleno = 0;
static BSP_I2C_busnumber_t i2c_busno;

static HAL_GPIODef_t reset_gpio = {
    .port = GPIOC,
    .pin = GPIO_PIN_13,
    .af = 0,
    .clock_control = &HAL_clk_GPIOC_clock_control
};


BSP_error_t UAIR_BSP_air_quality_init()
{
    BSP_powerzone_t powerzone = UAIR_POWERZONE_NONE;

    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        i2c_busno = BSP_I2C_BUS0;
        powerzone = UAIR_POWERZONE_AMBIENTSENS;
        break;
    case UAIR_NUCLEO_REV2:
        i2c_busno = BSP_I2C_BUS2;
        powerzone = UAIR_POWERZONE_AMBIENTSENS;
        break;
    default:
        break;
    }
    if (powerzone==UAIR_POWERZONE_NONE)
        return BSP_ERROR_NO_INIT;

    HAL_I2C_bus_t bus = UAIR_BSP_I2C_GetHALHandle(i2c_busno);

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

BSP_error_t BSP_air_quality_measurement_completed()
{
    ZMOD4510_op_result_t r = ZMOD4510_is_sequencer_completed(&zmod);
    if (r==ZMOD4510_OP_BUSY) {
        return BSP_ERROR_BUSY;
    }
    return BSP_ERROR_NONE;
}

static BSP_error_t UAIR_BSP_air_quality_zmod_op(ZMOD4510_op_result_t (*op)(ZMOD4510_t *zmod))
{
    ZMOD4510_op_result_t r;
    BSP_I2C_recover_action_t action;
    int retries=1;
    BSP_error_t err = BSP_ERROR_NONE;

    do {
        r = op(&zmod);
        switch (r) {
        case ZMOD4510_OP_DEVICE_ERROR:
            action = UAIR_BSP_I2C_analyse_and_recover_error(i2c_busno);
            BSP_TRACE("Recover action: %d", action);
            err = BSP_ERROR_COMPONENT_FAILURE;
            break;
        case ZMOD4510_OP_BUSY:
            err = BSP_ERROR_BUSY;
            break;
        case ZMOD4510_OP_SUCCESS:
            err = BSP_ERROR_NONE;
            break;
        default:
            err = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }
    } while ((err!=BSP_ERROR_NONE) && (retries--));

    return err;
}

static BSP_error_t UAIR_BSP_air_quality_read_adc(void)
{
    return UAIR_BSP_air_quality_zmod_op( &ZMOD4510_read_adc );
}

BSP_error_t BSP_air_quality_start_measurement()
{
    return UAIR_BSP_air_quality_zmod_op( &ZMOD4510_start_measurement );
}


static BSP_error_t UAIR_BSP_air_quality_sequencer_completed(void)
{
    return UAIR_BSP_air_quality_zmod_op( &ZMOD4510_is_sequencer_completed );
}



BSP_error_t BSP_air_quality_calculate(const float temp_c,
                                      const float hum_pct,
                                      BSP_air_quality_results_t *results)
{
    oaq_2nd_gen_results_t libresult;
    BSP_error_t err;

    err = UAIR_BSP_air_quality_sequencer_completed();

    if (err != BSP_ERROR_NONE)
        return err;


    err = UAIR_BSP_air_quality_read_adc();

    if (err != BSP_ERROR_NONE)
        return err;

    BSP_TRACE("Aquired sample no %d (901 needed for cal.)", sampleno);
    sampleno++;

    {
            char tmp[128];
            int i;
            char *p = tmp;
            for (i=0; i<ZMOD4510_ADC_DATA_LEN;i++) {
                p += tiny_sprintf(p, "%02x ", zmod.adc_result[i]);
            }
            BSP_TRACE("Sensor data: [%s]", tmp);
        }

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
