#include "models/OAQ.hpp"


static OAQInterface *interface;
static uint16_t init_samples = 901;

namespace OAQ
{
    bool setOAQInterface(OAQInterface*i)
    {
        if (nullptr!=interface)
            return false;
        interface  = i;
        return true;
    }

    void unsetOAQInterface()
    {
        interface = nullptr;
    }
}


extern "C"
{

    int8_t init_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev);

    int8_t wrap_init_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev)
    {
        init_samples = 901;
        return init_oaq_2nd_gen(handle, dev);
    }

    int8_t calc_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev,
                            const uint8_t *sensor_results_table,
                            const float humidity_pct, const float temperature_degc,
                            oaq_2nd_gen_results_t *results);

    int8_t wrap_calc_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev,
                                 const uint8_t *sensor_results_table,
                                 const float humidity_pct, const float temperature_degc,
                                 oaq_2nd_gen_results_t *results)
    {
        if (interface)
        {
            handle->stabilization_sample = init_samples;
            if (init_samples>0) {
                init_samples--;
                return OAQ_2ND_GEN_STABILIZATION;
            }
            else
            {
                results->O3_conc_ppb = interface->getO3ppb();
                results->FAST_AQI = interface->getFAST_AQI();
                results->EPA_AQI  = interface->getEPA_AQI();

                return OAQ_2ND_GEN_OK;
            }
        }
        else
        {
            return calc_oaq_2nd_gen(handle, dev, sensor_results_table, humidity_pct, temperature_degc, results);
        }
    }
}
