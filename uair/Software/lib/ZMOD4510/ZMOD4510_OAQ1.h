#ifndef ZMOD4510_OAQ1_H__
#define ZMOD4510_OAQ1_H__

#include "zmod4xxx_types.h"
#include <stdbool.h>

#ifndef OAQ_VERSION_MAJOR
#error  OAQ_VERSION_MAJOR not defined!
#endif

#ifndef OAQ_GEN
# error OAQ_GEN not defined!
#endif

#if OAQ_GEN==1

#include "zmod4510_config_oaq1.h"
#include "oaq_1st_gen.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        float EPA_AQI;
        float conc_no2; /**< Equivalent to NO2 concentration [ppb]. */
        float conc_o3; /**< Equivalent to O3 concentration [ppb]. */
        float aqi_no2; /**< Equivalent to NO2 AQI. */
        float aqi_o3; /**< Equivalent to O3 AQI. */
    } oaq_1st_gen_results_t;

typedef struct {
    oaq_base_handle_t algo_handle;
    oaq_1st_gen_results_t results;
    zmod4xxx_dev_t *dev;
#if OAQ_VERSION_MAJOR >= 4
    bool stabilized;
#endif
} ZMOD4510_OAQ1_t;

typedef enum {
    ZMOD4510_OAQ1_NO_ERROR = 0,
    ZMOD4510_OAQ1_ERROR_STABILIZING = -1,
    ZMOD4510_OAQ1_ERROR_UNSPECIFIED = -2
} ZMOD4510_OAQ1_error_t;



ZMOD4510_OAQ1_error_t ZMOD4510_OAQ1_init(ZMOD4510_OAQ1_t *oaq, zmod4xxx_dev_t *dev);
ZMOD4510_OAQ1_error_t ZMOD4510_OAQ1_calculate(ZMOD4510_OAQ1_t *oaq,
                                              const uint8_t *adc_result,
                                              oaq_1st_gen_results_t *results);


#ifdef __cplusplus
}
#endif

#endif // OAQ_GEN==2

#endif
