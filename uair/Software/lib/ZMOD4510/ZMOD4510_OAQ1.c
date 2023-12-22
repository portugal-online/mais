#include "ZMOD4510_OAQ1.h"
#include "BSP.h"

#ifndef OAQ_GEN
# error OAQ_GEN not defined!
#endif

#if OAQ_GEN==1

#include "zmod4510_config_oaq1.h"
#include "oaq_1st_gen.h"
#include "zmod4xxx_api.h"

ZMOD4510_OAQ1_error_t ZMOD4510_OAQ1_init(ZMOD4510_OAQ1_t *oaq, zmod4xxx_dev_t *dev)
{
    ZMOD4510_OAQ1_error_t r;

//    dev->init(dev);

    init_oaq_1st_gen(&oaq->algo_handle,
                     dev->prod_data,
                     4);
                     //STABILIZATION_SAMPLES);

    oaq->dev = dev;

    r = ZMOD4510_OAQ1_NO_ERROR;

    return r;
}


ZMOD4510_OAQ1_error_t ZMOD4510_OAQ1_calculate(ZMOD4510_OAQ1_t *oaq,
                                              const uint8_t *adc_result,
                                              oaq_1st_gen_results_t *results)
{
    ZMOD4510_OAQ1_error_t err;

#define RMOX_SIZE 15
    float rmox[RMOX_SIZE];

    bool highperf = false;

    BSP_TRACE("Calculating OAQ1");

    do {
        char adctext[1+ZMOD4510_ADC_DATA_LEN*3];
        char *p = &adctext[0];
        int i;
        for (i=0;i<ZMOD4510_ADC_DATA_LEN;i++) {
            p+=sprintf(p,"%02x ", adc_result[i]);
        }
        BSP_TRACE("ADC result: [ %s]", adctext);
    } while (0);

    bool stabilizing = (oaq->algo_handle.stabilization_sample>0);
    if (!stabilizing)
    {
        if (UAIR_HAL_request_high_performance()==UAIR_HAL_OP_SUCCESS) {
            highperf = true;
        }
    }

    int ret = zmod4xxx_calc_rmox(oaq->dev, adc_result, rmox);

    if (ret<0) {
        BSP_TRACE("Cannot calculate RMOX");
        err = ZMOD4510_OAQ1_ERROR_UNSPECIFIED;
    } else {

        do {
            char rmoxtext[1+RMOX_SIZE*16];
            char *p = &rmoxtext[0];
            int i;
            for (i=0; i<RMOX_SIZE; i++) {
                p+=sprintf(p,"%.02f ", rmox[i]);
            }
            BSP_TRACE("RMOX result: [ %s]", rmoxtext);
        } while (0);


        float AQI = calc_oaq_1st_gen(&oaq->algo_handle,
                                     rmox,
                                     RCDA_STRATEGY_ADJ,
                                     GAS_DETECTION_STRATEGY_AUTO,
                                     D_RISING_M1,
                                     D_FALLING_M1,
                                     D_CLASS_M1);

        // Release high-performance if we required it above.
        if (highperf)
        {
            UAIR_HAL_release_high_performance();
        }

        if (stabilizing) {
            BSP_TRACE("Stabilizing: samples required %d", oaq->algo_handle.stabilization_sample);
            err = ZMOD4510_OAQ1_ERROR_STABILIZING;
        } else {
            err = ZMOD4510_OAQ1_NO_ERROR;
            BSP_TRACE("AQI: %.02f", AQI);
            results->EPA_AQI = AQI;
            results->conc_no2 = oaq->algo_handle.conc_no2;
            results->conc_o3 = oaq->algo_handle.conc_o3;
            results->aqi_no2 = oaq->algo_handle.aqi_no2;
            results->aqi_o3 = oaq->algo_handle.aqi_o3;
        }
    }

    return err;
}
#endif // OAQ_GEN==1
