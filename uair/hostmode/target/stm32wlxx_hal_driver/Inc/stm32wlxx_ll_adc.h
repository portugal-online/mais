#ifndef STM32WLxx_LL_ADC_H
#define STM32WLxx_LL_ADC_H

#define ADC_ISR_EOC (1<<0)


#define LL_ADC_TRIGGER_FREQ_HIGH (1<<0)

#define LL_ADC_REG_RANK_1        (1<<0)

#define LL_ADC_SAMPLINGTIME_COMMON_1 (1<<0)

#define LL_ADC_SAMPLINGTIME_160CYCLES_5 (1<<0)

#define LL_ADC_CLOCK_SYNC_PCLK_DIV4 (1<<0)

#define LL_ADC_RESOLUTION_12B (1<<0)

#define LL_ADC_DATA_ALIGN_RIGHT (1<<0)

#define LL_ADC_REG_TRIG_SOFTWARE (1<<0)

#define LL_ADC_REG_OVR_DATA_OVERWRITTEN (1<<0)

#define LL_ADC_CHANNEL_VREFINT 9
#define LL_ADC_CHANNEL_VBAT 10
#define LL_ADC_CHANNEL_3 3

#define VREFINT_CAL_VREF                   ( 3300UL)                    /* Analog voltage reference (Vref+) voltage with which VrefInt has been calibrated in production (tolerance: +-10 mV) (unit: mV). */


#define ADC_CFGR1_RES_BITOFFSET_POS        ( 3UL) /* Value equivalent to bitfield "ADC_CFGR1_RES" position in register */

#define __LL_ADC_DIGITAL_SCALE(__ADC_RESOLUTION__)                             \
  (0xFFFUL >> ((__ADC_RESOLUTION__) >> (ADC_CFGR1_RES_BITOFFSET_POS - 1UL)))

#define __LL_ADC_CALC_DATA_TO_VOLTAGE(__VREFANALOG_VOLTAGE__,\
                                      __ADC_DATA__,\
                                      __ADC_RESOLUTION__)                    \
((__ADC_DATA__) * (__VREFANALOG_VOLTAGE__)                                   \
 / __LL_ADC_DIGITAL_SCALE(__ADC_RESOLUTION__)                                \
)


#endif
