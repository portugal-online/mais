#include "ZMOD4510_OAQ2.h"
#include "zmod4510_config_oaq2.h"
#include "oaq_2nd_gen.h"
#include "BSP.h"

ZMOD4510_OAQ2_error_t ZMOD4510_OAQ2_init(ZMOD4510_OAQ2_t *oaq, zmod4xxx_dev_t *dev)
{
    int8_t lib_ret = init_oaq_2nd_gen(&oaq->algo_handle, dev);
    if (lib_ret!=0) {
        BSP_TRACE("Cannot init OAQ library");
        return ZMOD4510_OAQ2_ERROR_UNSPECIFIED;
    }
    oaq->dev = dev;
    return ZMOD4510_OAQ2_NO_ERROR;
}


ZMOD4510_OAQ2_error_t ZMOD4510_OAQ2_calculate(ZMOD4510_OAQ2_t *oaq,
                               const uint8_t *adc_result,
                               const float humidity_pct,
                               const float temperature_degc,
                               oaq_2nd_gen_results_t *results)
{
    ZMOD4510_OAQ2_error_t err;
    BSP_TRACE("Calculating OAQ with temp=%f, hum=%f", temperature_degc, humidity_pct);
    int8_t lib_ret = calc_oaq_2nd_gen(&oaq->algo_handle,
                                      oaq->dev,
                                      adc_result,
                                      humidity_pct,
                                      temperature_degc,
                                      results);

    switch (lib_ret) {
    case OAQ_2ND_GEN_STABILIZATION:
        BSP_TRACE("Stabilizing: samples required %d", oaq->algo_handle.stabilization_sample);
        err = ZMOD4510_OAQ2_ERROR_STABILIZING;
        break;
    case OAQ_2ND_GEN_OK:
        err = ZMOD4510_OAQ2_NO_ERROR;
        break;
    default:
        BSP_TRACE("OAQ error: %d", lib_ret);
        err = ZMOD4510_OAQ2_ERROR_UNSPECIFIED;
        break;
    }
    return err;
}
