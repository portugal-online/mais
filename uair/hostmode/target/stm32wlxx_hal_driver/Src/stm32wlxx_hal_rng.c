#include "stm32wlxx_hal.h"
#include "cmsis_compiler.h"
#include <stdlib.h>

HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef*hrng, uint32_t *random32bit)
{
    *random32bit = random();
    return HAL_OK;
}
