#include "stm32wlxx_hal.h"

HAL_StatusTypeDef HAL_LPTIM_Init(LPTIM_HandleTypeDef *hlptim)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_LPTIM_DeInit(LPTIM_HandleTypeDef *hlptim)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_LPTIM_OnePulse_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
    return -1;
}

HAL_StatusTypeDef HAL_LPTIM_OnePulse_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
    return -1;
}

void HAL_LPTIM_IRQHandler(LPTIM_HandleTypeDef*hlptim)
{
}
