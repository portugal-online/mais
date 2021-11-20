#ifndef __UAIR_BSP_AIR_QUALITY_H__
#define __UAIR_BSP_AIR_QUALITY_H__

#include "BSP.h"
#include "HAL.h"

typedef struct {
    float O3_conc_ppb;
    uint16_t FAST_AQI;
    uint16_t EPA_AQI;
} BSP_air_quality_results_t;

BSP_error_t BSP_air_quality_start_measurement(void);
BSP_error_t BSP_air_quality_measurement_completed(void);
BSP_error_t BSP_air_quality_calculate(const float temp_c,
                                      const float hum_pct,
                                      BSP_air_quality_results_t *results);
unsigned int BSP_air_quality_get_measure_delay_us(void);
BSP_sensor_state_t BSP_air_quality_get_sensor_state(void);

#endif
