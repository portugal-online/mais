#ifndef INTERNAL_ADC_H__
#define INTERNAL_ADC_H__

void internal_adc_start(void);
unsigned internal_adc_get_completed(void);
unsigned internal_adc_get_vref_mv(void);
unsigned internal_adc_get_vchg_mv(void);

void internal_adc_completed_callback(void);

#endif

