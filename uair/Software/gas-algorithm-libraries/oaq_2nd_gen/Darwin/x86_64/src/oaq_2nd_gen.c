#include "oaq_2nd_gen.h"


int8_t init_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev)
{
    handle->stabilization_sample = 602;
    return OAQ_2ND_GEN_OK;
}

int8_t calc_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev,
                        const uint8_t *sensor_results_table,
                        const float humidity_pct, const float temperature_degc,
                        oaq_2nd_gen_results_t *results)
{
    unsigned i;

    if (handle->stabilization_sample>0) {
        handle->stabilization_sample--;
        return OAQ_2ND_GEN_STABILIZATION;
    }

    /* Put some dummy values */

    for (i=0; i<8; i++) {
        results->rmox[i] = 10e3; /**< MOx resistance. */
    }
    results->O3_conc_ppb = 50.0;

    results->FAST_AQI = 8;
    results->EPA_AQI = 7;

    return OAQ_2ND_GEN_OK;
}


