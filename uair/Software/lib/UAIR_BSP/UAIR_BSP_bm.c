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

#include "stm32wlxx_hal.h"
#include "UAIR_BSP_bm.h"
#include "UAIR_lpm.h"
#include "UAIR_BSP_clk_timer.h"
#include "HAL_gpio.h"
#include <cmsis_compiler.h>

ADC_HandleTypeDef UAIR_BSP_adc_handle;
DMA_HandleTypeDef UAIR_BSP_adc_dma;

static uint16_t raw_adc_reads[3];
static bool adc_valid = false;
static battery_measure_callback_t s_measure_callback = NULL;
static BSP_error_t block_measure_status;
static battery_measurements_t battery_meas;

#define HAL_BM_OUTPUT_DIVISION_RATIO ( (150+75) / 75 )

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

    UAIR_BSP_adc_handle.Instance = VBAT_ADC;
    UAIR_BSP_adc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    UAIR_BSP_adc_handle.Init.Resolution = VBAT_ADC_RES;
    UAIR_BSP_adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    UAIR_BSP_adc_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
    UAIR_BSP_adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    UAIR_BSP_adc_handle.Init.LowPowerAutoWait = DISABLE;
    UAIR_BSP_adc_handle.Init.LowPowerAutoPowerOff = DISABLE;
    UAIR_BSP_adc_handle.Init.ContinuousConvMode = DISABLE;
    UAIR_BSP_adc_handle.Init.NbrOfConversion = 3;
    UAIR_BSP_adc_handle.Init.DiscontinuousConvMode = DISABLE;
    UAIR_BSP_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    UAIR_BSP_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    UAIR_BSP_adc_handle.Init.DMAContinuousRequests = DISABLE;
    UAIR_BSP_adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    UAIR_BSP_adc_handle.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_19CYCLES_5;
    UAIR_BSP_adc_handle.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
    UAIR_BSP_adc_handle.Init.OversamplingMode = DISABLE;
    UAIR_BSP_adc_handle.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;

    return status == HAL_OK ? BSP_ERROR_NONE: BSP_ERROR_COMPONENT_FAILURE;
}

BSP_error_t UAIR_BSP_BM_PrepareAcquisition()
{
    ADC_ChannelConfTypeDef channel_config = {0};

    if (HAL_ADC_Init(&UAIR_BSP_adc_handle) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    channel_config.Channel = VREF_ADC_CHANNEL;
    channel_config.Rank = ADC_REGULAR_RANK_1;
    channel_config.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
    if (HAL_ADC_ConfigChannel(&UAIR_BSP_adc_handle, &channel_config) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    channel_config.Channel = VBAT_ADC_CHANNEL;
    channel_config.Rank = ADC_REGULAR_RANK_2;
    channel_config.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
    if (HAL_ADC_ConfigChannel(&UAIR_BSP_adc_handle, &channel_config) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    channel_config.Channel = VREF_ADC_CHANNEL;
    channel_config.Rank = ADC_REGULAR_RANK_3;
    channel_config.SamplingTime = ADC_SAMPLETIME_19CYCLES_5;
    if (HAL_ADC_ConfigChannel(&UAIR_BSP_adc_handle, &channel_config) != HAL_OK)
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
    HAL_ADC_DeInit(&UAIR_BSP_adc_handle);

    HAL_DMA_DeInit(&UAIR_BSP_adc_dma);

    /* ADC interrupt DeInit */
    HAL_NVIC_DisableIRQ(ADC_IRQn);

    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_BM_EndAcquisition(void)
{
    HAL_ADC_DeInit(&UAIR_BSP_adc_handle);
    return BSP_ERROR_NONE;
}

/**
 * @brief Reads (measures) Channel ADC value
 *
 * @return uint32_t measurement value or 0 in case of read failure
 */
BSP_error_t UAIR_BSP_BM_StartMeasure(battery_measure_callback_t measure_callback)
{
    BSP_error_t err;

    UAIR_LPM_SetStopMode((1 << UAIR_LPM_ADC_BM), UAIR_LPM_DISABLE);

    s_measure_callback = measure_callback;

    /* Start Calibration */
    if (HAL_ADCEx_Calibration_Start(&UAIR_BSP_adc_handle) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    if (HAL_ADC_Start_DMA(&UAIR_BSP_adc_handle, (uint32_t*)&raw_adc_reads[0],
                          sizeof(raw_adc_reads)/sizeof(raw_adc_reads[0])
                         ) == HAL_OK)
    {
        err = BSP_ERROR_NONE;
    }
    else
    {
        // Do nothing and return a 0 RAW value indicating an error
        UAIR_LPM_SetStopMode((1 << UAIR_LPM_ADC_BM), UAIR_LPM_ENABLE);
        err = BSP_ERROR_COMPONENT_FAILURE;
    }
    return err;
}

static void measure_callback_wait(BSP_error_t error,
                                  const battery_measurements_t *meas)
{
    block_measure_status = error;
}

BSP_error_t UAIR_BSP_BM_MeasureBlocking(battery_measurements_t *meas)
{
    block_measure_status = BSP_ERROR_BUSY;

    BSP_error_t err = UAIR_BSP_BM_StartMeasure(measure_callback_wait);

    if (err!=BSP_ERROR_NONE)
        return err;

    do {
        __disable_irq();
        if (block_measure_status!=BSP_ERROR_BUSY) {
            __enable_irq();
            break;
        } else {
            __WFI();
        }
        __enable_irq();
    } while (1);

    meas->battery_voltage_mv = battery_meas.battery_voltage_mv;
    meas->supply_voltage_mv = battery_meas.supply_voltage_mv;

    return block_measure_status;
}


static void UAIR_BSP_ADC_calculate_values()
{
    /** We can use multiple methonds if the SOC is calibrated in production
     * (uint32_t)*VREFINT_CAL_ADDR
     * __LL_ADC_CALC_VREFANALOG_VOLTAGE(measuredLevel, ADC_RESOLUTION_12B)
     */
    uint32_t vrefint_avg = (raw_adc_reads[0] + raw_adc_reads[2]) >> 1;

    battery_meas.supply_voltage_mv = __LL_ADC_CALC_VREFANALOG_VOLTAGE( vrefint_avg, VBAT_ADC_RES );

    battery_meas.battery_voltage_mv = __LL_ADC_CALC_DATA_TO_VOLTAGE( battery_meas.supply_voltage_mv,
                                                                    raw_adc_reads[1],
                                                                    VBAT_ADC_RES)  * HAL_BM_OUTPUT_DIVISION_RATIO;


}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *handle)
{
    HAL_ADC_Stop(&UAIR_BSP_adc_handle); /* it calls also ADC_Disable() */
    adc_valid = true;
    UAIR_LPM_SetStopMode((1 << UAIR_LPM_ADC_BM), UAIR_LPM_ENABLE);

    UAIR_BSP_ADC_calculate_values();

    if (s_measure_callback)
        s_measure_callback(BSP_ERROR_NONE, &battery_meas);

}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *handle)
{
    HAL_ADC_Stop(&UAIR_BSP_adc_handle); /* it calls also ADC_Disable() */
    adc_valid = false;
    UAIR_LPM_SetStopMode((1 << UAIR_LPM_ADC_BM), UAIR_LPM_ENABLE);

    if (s_measure_callback)
        s_measure_callback(BSP_ERROR_COMPONENT_FAILURE, &battery_meas);
}

const uint16_t *UAIR_BSP_BM_GetRawValues(void)
{
    return &raw_adc_reads[0];
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

