/** Copyright (c) 2021 MAIS Project
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
 * @file UAIR_BSP_bm.c
 *
 * @copyright (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP_bm.h"
#include "HAL_gpio.h"

ADC_HandleTypeDef UAIR_BSP_voltage_adc;

/**
 * Battery Monitoring (BM) APIs
 */

static HAL_GPIODef_t vbat_control_gpio = {
    .port = GPIOA,
    .pin = GPIO_PIN_7,
    .af = 0,
    .clock_control = &HAL_clk_GPIOA_clock_control
};

static HAL_GPIODef_t vbat_adc_gpio = {
    .port = GPIOB,
    .pin = GPIO_PIN_4,
    .af = 0,
    .clock_control = &HAL_clk_GPIOA_clock_control
};



/**
 * @brief Configure battery monitoring control GPIO and ADC pin.
 *
 * @return UAIR_BSP status
 */
BSP_error_t UAIR_BSP_BM_Init(void)
{
    HAL_clk_GPIOA_clock_control(1);

    HAL_StatusTypeDef status = HAL_GPIO_configure_output_pp(&vbat_control_gpio);
    status |= HAL_GPIO_configure_input_analog(&vbat_adc_gpio);

    HAL_GPIO_write(&vbat_control_gpio, 0);

    UAIR_BSP_voltage_adc.Instance = VBAT_ADC;
    UAIR_BSP_voltage_adc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    UAIR_BSP_voltage_adc.Init.Resolution = VBAT_ADC_RES;
    UAIR_BSP_voltage_adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    UAIR_BSP_voltage_adc.Init.ScanConvMode = ADC_SCAN_DISABLE;
    UAIR_BSP_voltage_adc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    UAIR_BSP_voltage_adc.Init.LowPowerAutoWait = DISABLE;
    UAIR_BSP_voltage_adc.Init.LowPowerAutoPowerOff = DISABLE;
    UAIR_BSP_voltage_adc.Init.ContinuousConvMode = DISABLE;
    UAIR_BSP_voltage_adc.Init.NbrOfConversion = 1;
    UAIR_BSP_voltage_adc.Init.DiscontinuousConvMode = DISABLE;
    UAIR_BSP_voltage_adc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    UAIR_BSP_voltage_adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    UAIR_BSP_voltage_adc.Init.DMAContinuousRequests = DISABLE;
    UAIR_BSP_voltage_adc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    UAIR_BSP_voltage_adc.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
    UAIR_BSP_voltage_adc.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
    UAIR_BSP_voltage_adc.Init.OversamplingMode = DISABLE;
    UAIR_BSP_voltage_adc.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;

    return status == HAL_OK ? BSP_ERROR_NONE: BSP_ERROR_COMPONENT_FAILURE;
}

BSP_error_t UAIR_BSP_BM_PrepareAcquisition()
{
    if (HAL_ADC_Init(&UAIR_BSP_voltage_adc) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    /* Start Calibration */
    if (HAL_ADCEx_Calibration_Start(&UAIR_BSP_voltage_adc) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    return BSP_ERROR_NONE;
}

/**
 * @brief DeInit BM pins configurations.
 *
 * @return UAIR_BSP status
 */
int32_t UAIR_BSP_BM_DeInit(void)
{
    /* Turn off VBAT pin */
 //   HAL_GPIO_WritePin(VBAT_READ_PORT, VBAT_READ_PIN, GPIO_PIN_RESET);

    /* DeInit the VBAT pin */
 //   HAL_GPIO_DeInit(VBAT_READ_PORT, VBAT_READ_PIN);

    HAL_ADC_DeInit(&UAIR_BSP_voltage_adc);

    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_BM_EndAcquisition(void)
{
    HAL_ADC_DeInit(&UAIR_BSP_voltage_adc);
    return BSP_ERROR_NONE;
}

/**
 * @brief Configures the ADC Channel used for measurement.
 *
 * @param channel can be VREF_ADC_CHANNEL or VBAT_ADC_CHANNEL
 * @return UAIR_BSP status
 */
int32_t UAIR_BSP_BM_ConfChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef channel_config = {0};
    channel_config.Channel = channel; // typically: VREF_ADC_CHANNEL or VBAT_ADC_CHANNEL
    channel_config.Rank = ADC_REGULAR_RANK_1;
    channel_config.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
    if (HAL_ADC_ConfigChannel(&UAIR_BSP_voltage_adc, &channel_config) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }
    return BSP_ERROR_NONE;
}

/**
 * @brief Reads (measures) Channel ADC value
 *
 * @return uint32_t measurement value or 0 in case of read failure
 */
uint32_t UAIR_BSP_BM_ReadChannel(void)
{
    uint32_t raw_adc_read = 0;

    if (HAL_ADC_Start(&UAIR_BSP_voltage_adc) == HAL_OK)
    {
        HAL_ADC_PollForConversion(&UAIR_BSP_voltage_adc, HAL_MAX_DELAY);
        HAL_ADC_Stop(&UAIR_BSP_voltage_adc); /* it calls also ADC_Disable() */
        raw_adc_read = HAL_ADC_GetValue(&UAIR_BSP_voltage_adc);
    }
    else
    {
        // Do nothing and return a 0 RAW value indicating an error
    }
    return raw_adc_read;
}

BSP_error_t UAIR_BSP_BM_EnableBatteryRead()
{
    HAL_GPIO_write(&vbat_control_gpio, 1);
    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_BM_DisableBatteryRead()
{
    HAL_GPIO_write(&vbat_control_gpio, 0);
    return BSP_ERROR_NONE;
}

