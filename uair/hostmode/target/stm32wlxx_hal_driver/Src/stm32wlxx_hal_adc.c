#include "stm32wlxx_hal.h"

HAL_StatusTypeDef       HAL_ADC_Init(ADC_HandleTypeDef *hadc)
{
    return HAL_OK;
}

HAL_StatusTypeDef       HAL_ADC_DeInit(ADC_HandleTypeDef *hadc)
{
    return HAL_OK;
}

HAL_StatusTypeDef       HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *sConfig)
{
    return HAL_OK;
}

HAL_StatusTypeDef       HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef *hadc)
{
    return HAL_OK;
}

HAL_StatusTypeDef       HAL_ADC_Start(ADC_HandleTypeDef *hadc)
{
    return HAL_OK;
}

HAL_StatusTypeDef       HAL_ADC_Stop(ADC_HandleTypeDef *hadc)
{
    return HAL_OK;
}

HAL_StatusTypeDef       HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc, uint32_t Timeout)
{
    return HAL_OK;
}

uint32_t                HAL_ADC_GetValue(ADC_HandleTypeDef *hadc)
{
    return 0xFFFF;
}

void HAL_ADC_IRQHandler(ADC_HandleTypeDef *hadc)
{
}
