#include "stm32l1xx_hal.h"
#include "stm32l1xx_ll_adc.h"
#include "debug.h"
#include "internal_adc.h"

extern ADC_HandleTypeDef hadc;
extern TIM_HandleTypeDef htim6;

struct adc_values
{
    union {
        struct {
            uint16_t vchg;
            uint16_t vref;
        };
        uint32_t raw;
    };
};

static struct adc_values adc_data;
static unsigned int adc_completed = 0;
static unsigned int internal_vref_mV = 0;

unsigned internal_adc_get_completed()
{
    return adc_completed;
}

unsigned internal_adc_get_vref_mv()
{
    return internal_vref_mV;
}

void internal_adc_start()
{
    HAL_ADC_Start_DMA(&hadc, &adc_data.raw, 2);
    //HAL_ADC_Start_IT(&hadc);
    HAL_TIM_Base_Start(&htim6);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *adc)
{
    //dprintf("ADC %d %d\r\n", adc_data.vchg, adc_data.vref);
    //HAL_ADC_Start_DMA(&hadc, &adc_data.raw, sizeof(adc_data));
    adc_completed++;

    internal_vref_mV = __LL_ADC_CALC_VREFANALOG_VOLTAGE( adc_data.vref, ADC_RESOLUTION_12B );
    internal_adc_completed_callback();
}

unsigned internal_adc_get_vchg_mv(void)
{
    // 65535 = Vref
    // Vchg = (ADCvchg*Vref)/(1<<12);
    uint32_t adcchg = adc_data.vchg;
    adcchg *= internal_vref_mV;
    // Resistor network is 3M / 1.43M
    // adcchg>>=12;
    adcchg /= 1322;
    return adcchg;
}

