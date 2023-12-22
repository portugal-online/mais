#include "ZMOD4510_OAQ2.h"
#include "BSP.h"
#include "zmod4xxx_api.h"
#ifndef OAQ_GEN
# error OAQ_GEN not defined!
#endif

#if OAQ_GEN==2

#include "zmod4510_config_oaq2.h"
#include "oaq_2nd_gen.h"

int8_t __WEAK wrap_init_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev)
{
#if OAQ_VERSION_MAJOR >= 4
    return init_oaq_2nd_gen(handle);
#else
    return init_oaq_2nd_gen(handle, dev);
#endif
}

int8_t __WEAK wrap_calc_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev,
                                    uint8_t *sensor_results_table,
                                    const float humidity_pct, const float temperature_degc,
                                    oaq_2nd_gen_results_t *results)
{


#if OAQ_VERSION_MAJOR >= 4
    oaq_2nd_gen_inputs_t algo_input;
    algo_input.adc_result = sensor_results_table; // TBD: is this really not modifiable?
    algo_input.humidity_pct = humidity_pct;
    algo_input.temperature_degc = temperature_degc;
    return calc_oaq_2nd_gen(handle, dev, &algo_input, results);
#else
    return calc_oaq_2nd_gen(handle, dev, sensor_results_table, humidity_pct, temperature_degc, results);
#endif
}

ZMOD4510_OAQ2_error_t ZMOD4510_OAQ2_init(ZMOD4510_OAQ2_t *oaq, zmod4xxx_dev_t *dev)
{
    ZMOD4510_OAQ2_error_t r;

#if OAQ_VERSION_MAJOR >= 4
    oaq->stabilized = false;
#endif

    int8_t lib_ret = wrap_init_oaq_2nd_gen(&oaq->algo_handle, dev);

    if (lib_ret == 0)
    {
        oaq->dev = dev;
        r = ZMOD4510_OAQ2_NO_ERROR;
    }
    else
    {
        r = ZMOD4510_OAQ2_ERROR_UNSPECIFIED;
    }
    return r;
}

static void dump_rmox(float *rmox)
{
    // Dump RMOX
    char mox[256];
    char *p = mox;
    char units;
    for (int i=0; i<8;i++) {
        float r = rmox[i];
        if (r>1000000.0F) {
            r/=1000000.0F;
            units='M';
        } else if (r>1000.0F) {
            r/=1000.0F;
            units='K';
        } else {
            units='R';
        }
        p += sprintf(p," %.02f%c", r, units);
    }

    BSP_TRACE("Rmox:%s", mox);
}


ZMOD4510_OAQ2_error_t ZMOD4510_OAQ2_calculate(ZMOD4510_OAQ2_t *oaq,
                               uint8_t *adc_result,
                               const float humidity_pct,
                               const float temperature_degc,
                               oaq_2nd_gen_results_t *results)
{
    ZMOD4510_OAQ2_error_t err;

    bool highperf = false;

    BSP_TRACE("Calculating OAQ with temp=%f, hum=%f", temperature_degc, humidity_pct);

    do {
        char adctext[1+ZMOD4510_ADC_DATA_LEN*3];
        char *p = &adctext[0];
        int i;
        for (i=0;i<ZMOD4510_ADC_DATA_LEN;i++) {
            p+=sprintf(p,"%02x ", adc_result[i]);
        }
        BSP_TRACE("ADC result: [ %s]", adctext);
    } while (0);

    // If we already stabilized, request high-performance from the BSP.
#if OAQ_VERSION_MAJOR >= 4
    if (oaq->stabilized)
#else
    if (oaq->algo_handle.stabilization_sample==0)
#endif
    {
        if (UAIR_HAL_request_high_performance()==UAIR_HAL_OP_SUCCESS) {
            highperf = true;
        }
    }

    int8_t lib_ret = wrap_calc_oaq_2nd_gen(&oaq->algo_handle,
                                           oaq->dev,
                                           adc_result,
                                           humidity_pct,
                                           temperature_degc,
                                           results);

    // Release high-performance if we required it above.
    if (highperf)
    {
        UAIR_HAL_release_high_performance();
    }

    switch (lib_ret) {
    case OAQ_2ND_GEN_STABILIZATION:
#if OAQ_VERSION_MAJOR >= 4
        BSP_TRACE("Stabilizing: samples so far %d", oaq->algo_handle.sample_cnt);
#else
        BSP_TRACE("Stabilizing: samples required %d", oaq->algo_handle.stabilization_sample);
#endif
        err = ZMOD4510_OAQ2_ERROR_STABILIZING;
        break;
    case OAQ_2ND_GEN_OK:
        err = ZMOD4510_OAQ2_NO_ERROR;
#if OAQ_VERSION_MAJOR >= 4
        oaq->stabilized = true;
#endif

        break;
    default:
        BSP_TRACE("OAQ error: %d", lib_ret);
        err = ZMOD4510_OAQ2_ERROR_UNSPECIFIED;
        break;
    }

    if (err !=ZMOD4510_OAQ2_ERROR_UNSPECIFIED) {
        dump_rmox(results->rmox);
    }


    return err;
}
#endif // OAQ_GEN==2
