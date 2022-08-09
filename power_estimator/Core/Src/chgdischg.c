#include "chgdischg.h"
#include "main.h"
#include "ldo.h"

void chgdischg_enable_charge()
{
    chgdischg_disable_discharge();
    HAL_GPIO_WritePin(CHG_GPIO_Port, CHG_Pin, 1);
}

void chgdischg_disable_charge()
{
    HAL_GPIO_WritePin(CHG_GPIO_Port, CHG_Pin, 0);
}

void chgdischg_enable_discharge()
{
    // Forcibly turn LDO off
    ldo_disable();
    chgdischg_disable_charge();
    HAL_GPIO_WritePin(DISCHG_GPIO_Port, DISCHG_Pin, 1);
}

void chgdischg_disable_discharge()
{
    HAL_GPIO_WritePin(DISCHG_GPIO_Port, DISCHG_Pin, 0);
}

bool chgdischg_charge_enabled(void)
{
    return HAL_GPIO_ReadPin(CHG_GPIO_Port, CHG_Pin);
}

bool chgdischg_discharge_enabled(void)
{
    return HAL_GPIO_ReadPin(DISCHG_GPIO_Port, DISCHG_Pin);
}
