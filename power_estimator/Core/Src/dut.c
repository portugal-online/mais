#include "dut.h"
#include "main.h"

void dut_enable_power(void)
{
    HAL_GPIO_WritePin(DUTEN_GPIO_Port, DUTEN_Pin, 1);
}

void dut_disable_power(void)
{
    HAL_GPIO_WritePin(DUTEN_GPIO_Port, DUTEN_Pin, 0);
}

