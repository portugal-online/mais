/** Copyright © 2021 The Things Industries B.V.
 *  Copyright © 2021 MAIS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file UAIR_BSP_radio.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP_radio.h"
#include "UAIR_BSP_lpm.h"

SUBGHZ_HandleTypeDef hsubghz;

static void UAIR_BSP_TCXO_Init()
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    RF_TCXO_VCC_CLK_ENABLE();

    gpio_init_structure.Pin = RF_TCXO_VCC_PIN;
    gpio_init_structure.Mode = GPIO_MODE_ANALOG;//OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(RF_TCXO_VCC_GPIO_PORT, &gpio_init_structure);

    HAL_GPIO_WritePin(RF_TCXO_VCC_GPIO_PORT, RF_TCXO_VCC_PIN, 1);
}

int32_t UAIR_BSP_SUBGHZ_Init(void)
{
    hsubghz.Init.BaudratePrescaler = SUBGHZSPI_BAUDRATEPRESCALER_4;
    if (HAL_SUBGHZ_Init(&hsubghz) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }
    UAIR_BSP_TCXO_Init();
    return BSP_ERROR_NONE;
}

int32_t RBI_Init(void)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    /* Enable the Radio Switch Clock */
    RF_SW_CTRL3_GPIO_CLK_ENABLE();

    /* Configure the Radio Switch pin */
    gpio_init_structure.Pin = RF_SW_CTRL1_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(RF_SW_CTRL1_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = RF_SW_CTRL2_PIN;
    HAL_GPIO_Init(RF_SW_CTRL2_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = RF_SW_CTRL3_PIN;
    HAL_GPIO_Init(RF_SW_CTRL3_GPIO_PORT, &gpio_init_structure);

    HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);

    return 0;
}

int32_t RBI_DeInit(void)
{
    RF_SW_CTRL3_GPIO_CLK_ENABLE();

    /* Turn off switch */
    HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);

    /* DeInit the Radio Switch pin */
    HAL_GPIO_DeInit(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN);
    HAL_GPIO_DeInit(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN);
    HAL_GPIO_DeInit(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN);

    return 0;
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef Config)
{
    switch (Config)
    {
    case RBI_SWITCH_OFF:
        {
            UAIR_BSP_LPM_disable_lownoise_operation();
            /* Turn off switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);

            break;
        }
    case RBI_SWITCH_RX:
        {
            UAIR_BSP_LPM_enable_lownoise_operation();
            /*Turns On in Rx Mode the RF Switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_RESET);
            break;
        }
    case RBI_SWITCH_RFO_LP:
        {
            UAIR_BSP_LPM_enable_lownoise_operation();
            /*Turns On in Tx Low Power the RF Switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_SET);
            break;
        }
    case RBI_SWITCH_RFO_HP:
        {
            UAIR_BSP_LPM_enable_lownoise_operation();
            /*Turns On in Tx High Power the RF Switch */
            HAL_GPIO_WritePin(RF_SW_CTRL3_GPIO_PORT, RF_SW_CTRL3_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RF_SW_CTRL1_GPIO_PORT, RF_SW_CTRL1_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RF_SW_CTRL2_GPIO_PORT, RF_SW_CTRL2_PIN, GPIO_PIN_SET);
            break;
        }
    default:
        break;
    }

    __HAL_RCC_HSE_CONFIG(RCC_HSE_OFF);

    return 0;
}

int32_t RBI_GetTxConfig(void)
{
    return RBI_CONF_RFO;
}

int32_t RBI_GetWakeUpTime(void)
{
    return RF_WAKEUP_TIME;
}

int32_t RBI_IsTCXO(void)
{
    return IS_TCXO_SUPPORTED;
}

int32_t RBI_IsDCDC(void)
{
    return IS_DCDC_SUPPORTED;
}
