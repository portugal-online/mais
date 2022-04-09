#include "stm32wlxx_hal.h"
#include <assert.h>

#include "models/hw_lptim.h"

HAL_StatusTypeDef HAL_LPTIM_Init(LPTIM_HandleTypeDef *hlptim)
{
    assert ( hlptim->Init.Clock.Source == LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC );

    //UAIR_BSP_lptim.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32; // 1.333333us per tick @ 24Mhz, 16us @ 2Mhz
    lptim_engine_init( 1U<< hlptim->Init.Clock.Prescaler );

    return HAL_OK;
}

HAL_StatusTypeDef HAL_LPTIM_DeInit(LPTIM_HandleTypeDef *hlptim)
{
    lptim_engine_deinit();

    return HAL_OK;
}

HAL_StatusTypeDef HAL_LPTIM_OnePulse_Start_IT(LPTIM_HandleTypeDef *hlptim, uint32_t Period, uint32_t Pulse)
{
    lptim_engine_start_it( Period );

    return HAL_OK;
}

HAL_StatusTypeDef HAL_LPTIM_OnePulse_Stop_IT(LPTIM_HandleTypeDef *hlptim)
{
    lptim_engine_stop_it();

    return HAL_OK;
}

void HAL_LPTIM_IRQHandler(LPTIM_HandleTypeDef*hlptim)
{
    HAL_LPTIM_AutoReloadMatchCallback(hlptim);
}
