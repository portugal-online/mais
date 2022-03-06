#include "stm32wlxx_hal.h"
#include "cmsis_compiler.h"

void              HAL_PWR_EnableBkUpAccess(void)
{
}

void              HAL_PWR_DisableBkUpAccess(void)
{
}

void              HAL_PWR_EnterSLEEPMode(uint32_t Regulator, uint8_t SLEEPEntry)
{
    __WFI();
}

void              HAL_PWREx_EnableLowPowerRunMode(void)
{
}

HAL_StatusTypeDef HAL_PWREx_DisableLowPowerRunMode(void)
{
    return HAL_OK;
}

void              HAL_PWREx_EnterSTOP0Mode(uint8_t STOPEntry)
{
}

void              HAL_PWREx_EnterSTOP1Mode(uint8_t STOPEntry)
{
}

void              HAL_PWREx_EnterSTOP2Mode(uint8_t STOPEntry)
{
    __WFI();
}

void              HAL_PWREx_EnterSHUTDOWNMode(void)
{
}

void LL_PWR_ClearFlag_C1STOP_C1STB()
{
}


