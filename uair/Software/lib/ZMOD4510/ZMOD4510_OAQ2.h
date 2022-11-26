#ifndef ZMOD4510_OAQ2_H__
#define ZMOD4510_OAQ2_H__

#include "zmod4xxx_types.h"
#include "zmod4510_config_oaq2.h"
#include "oaq_2nd_gen.h"
#include <stdbool.h>

#ifndef OAQ_VERSION_MAJOR
#error  OAQ_VERSION_MAJOR not defined!
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    oaq_2nd_gen_handle_t algo_handle;
    oaq_2nd_gen_results_t algo_results;
    zmod4xxx_dev_t *dev;
#if OAQ_VERSION_MAJOR >= 4
    bool stabilized;
#endif
} ZMOD4510_OAQ2_t;

typedef enum {
    ZMOD4510_OAQ2_NO_ERROR = 0,
    ZMOD4510_OAQ2_ERROR_STABILIZING = -1,
    ZMOD4510_OAQ2_ERROR_UNSPECIFIED = -2
} ZMOD4510_OAQ2_error_t;



ZMOD4510_OAQ2_error_t ZMOD4510_OAQ2_init(ZMOD4510_OAQ2_t *oaq, zmod4xxx_dev_t *dev);
ZMOD4510_OAQ2_error_t ZMOD4510_OAQ2_calculate(ZMOD4510_OAQ2_t *oaq,
                                              const uint8_t *adc_result,
                                              const float humidity_pct,
                                              const float temperature_degc,
                                              oaq_2nd_gen_results_t *results);

/* Wrappers to allow testing to capture OAQ library calls */

int8_t wrap_init_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev);
int8_t wrap_calc_oaq_2nd_gen(oaq_2nd_gen_handle_t *handle, zmod4xxx_dev_t *dev,
                             const uint8_t *sensor_results_table,
                             const float humidity_pct, const float temperature_degc,
                             oaq_2nd_gen_results_t *results);


#ifdef __cplusplus
}
#endif

#endif
