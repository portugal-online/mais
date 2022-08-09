#include "ldo.h"
#include "main.h"

static ldo_voltage_t ldo_voltage = VSEL_2V2;

void ldo_enable(void)
{
    HAL_GPIO_WritePin(LDOEN_GPIO_Port, LDOEN_Pin, 1);
}

void ldo_disable(void)
{
    HAL_GPIO_WritePin(LDOEN_GPIO_Port, LDOEN_Pin, 0);
}

void ldo_set_voltage(ldo_voltage_t v)
{
    uint8_t vsel0 = 0;
    uint8_t vsel1 = 0;

    switch (v) {
    case VSEL_2V2:
        break;
    case VSEL_2V5:
        vsel0 = 1;
    case VSEL_3V7:
        vsel1 = 1;
    case VSEL_4V0:
        vsel0 = 1;
        vsel1 = 1;
        break;
    default:
        break;
    }
    HAL_GPIO_WritePin(VSEL0_GPIO_Port, VSEL0_Pin, vsel0);
    HAL_GPIO_WritePin(VSEL1_GPIO_Port, VSEL1_Pin, vsel1);
    ldo_voltage = v;
    ldo_enable();
}

ldo_voltage_t ldo_get_voltage(void)
{
    return ldo_voltage;
}

bool ldo_enabled(void)
{
    return HAL_GPIO_ReadPin(LDOEN_GPIO_Port, LDOEN_Pin);
}
