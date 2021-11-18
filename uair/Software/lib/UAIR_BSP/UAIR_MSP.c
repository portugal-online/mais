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
 * @file UAIR_msp.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP.h"

static void msp_error_handler(void);

extern UART_HandleTypeDef UAIR_BSP_debug_usart;
extern DMA_HandleTypeDef UAIR_BSP_debug_hdma_tx;

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    if (uartHandle->Instance == DEBUG_USART)
    {
        PeriphClkInit.PeriphClockSelection = DEBUG_USART_PERIPH_CLK;
        PeriphClkInit.Usart2ClockSelection = DEBUG_USART_SOURCE_CLK; // Configures USART2
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            msp_error_handler();
        }

        DEBUG_USART_CLK_CONTROL(1);
        DEBUG_USART_TX_GPIO_CLK_CONTROL(1);
        DEBUG_USART_RX_GPIO_CLK_CONTROL(1);

        gpio_init_structure.Pin = DEBUG_USART_TX_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_structure.Alternate = DEBUG_USART_TX_AF;
        HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &gpio_init_structure);

        gpio_init_structure.Pin = DEBUG_USART_RX_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_structure.Alternate = DEBUG_USART_RX_AF;
        HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &gpio_init_structure);

        /* Configure the DMA handler for Transmission process */
        UAIR_BSP_debug_hdma_tx.Instance = DEBUG_USART_TX_DMA_CHANNEL;
        UAIR_BSP_debug_hdma_tx.Init.Request = DEBUG_USART_TX_DMA_REQUEST;
        UAIR_BSP_debug_hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        UAIR_BSP_debug_hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        UAIR_BSP_debug_hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        UAIR_BSP_debug_hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        UAIR_BSP_debug_hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        UAIR_BSP_debug_hdma_tx.Init.Mode = DMA_NORMAL;
        UAIR_BSP_debug_hdma_tx.Init.Priority = DMA_PRIORITY_LOW;

        if (HAL_DMA_Init(&UAIR_BSP_debug_hdma_tx) != HAL_OK)
        {
            msp_error_handler();
        }

        if (HAL_DMA_ConfigChannelAttributes(&UAIR_BSP_debug_hdma_tx, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            msp_error_handler();
        }

        /* Associate the initialized DMA handle to the UART handle */
        __HAL_LINKDMA(uartHandle, hdmatx, UAIR_BSP_debug_hdma_tx);

        /* NVIC for DEBUG_USART, to catch the TX complete */
        HAL_NVIC_SetPriority(DEBUG_USART_IRQn, DEBUG_USART_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(DEBUG_USART_IRQn);
        /* Enable DEBUG_USART wakeup interrupt */
        LL_EXTI_EnableIT_0_31(DEBUG_USART_EXTI_WAKEUP);
    }
    else
    {
        msp_error_handler();
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == DEBUG_USART)
    {
        DEBUG_USART_CLK_CONTROL(0);
        HAL_GPIO_DeInit(GPIOA, DEBUG_USART_TX_PIN | DEBUG_USART_RX_PIN);
        HAL_DMA_DeInit(uartHandle->hdmatx);
        HAL_NVIC_DisableIRQ(DEBUG_USART_IRQn);
    }
    else
    {
        msp_error_handler();
    }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle)
{
    GPIO_InitTypeDef gpio_init_structure;
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;

    if (i2cHandle->Instance == EXT_SENSOR_I2C3)
    {
        RCC_PeriphCLKInitStruct.PeriphClockSelection = EXT_SENSOR_I2C3_PERIPH_CLK;
        RCC_PeriphCLKInitStruct.I2c1ClockSelection = EXT_SENSOR_I2C3_SOURCE_CLK;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
        BSP_TRACE("Config I2c3");

        EXT_SENSOR_I2C3_SDA_GPIO_CLK_ENABLE();
        EXT_SENSOR_I2C3_SCL_GPIO_CLK_ENABLE();
        EXT_SENSOR_I2C3_CLK_ENABLE();
        __HAL_RCC_I2C3_CLK_ENABLE();


        gpio_init_structure.Pin = EXT_SENSOR_I2C3_SCL_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_OD;
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_structure.Alternate = EXT_SENSOR_I2C3_SCL_SDA_AF;
        HAL_GPIO_Init(EXT_SENSOR_I2C3_SCL_GPIO_PORT, &gpio_init_structure);

        gpio_init_structure.Pin = EXT_SENSOR_I2C3_SDA_PIN;
        gpio_init_structure.Alternate = EXT_SENSOR_I2C3_SCL_SDA_AF;
        HAL_GPIO_Init(EXT_SENSOR_I2C3_SDA_GPIO_PORT, &gpio_init_structure);
    }
    else
    {
        msp_error_handler();
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle)
{
    if (i2cHandle->Instance == EXT_SENSOR_I2C3)
    {
        EXT_SENSOR_I2C3_FORCE_RESET();
        EXT_SENSOR_I2C3_RELEASE_RESET();

        HAL_GPIO_DeInit(EXT_SENSOR_I2C3_SCL_GPIO_PORT, EXT_SENSOR_I2C3_SCL_PIN);
        HAL_GPIO_DeInit(EXT_SENSOR_I2C3_SDA_GPIO_PORT, EXT_SENSOR_I2C3_SDA_PIN);
    }
    else
    {
        msp_error_handler();
    }
}
#ifdef MICROPHONE_USE_I2S

void HAL_I2S_MspInit(I2S_HandleTypeDef *spiHandle)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    if (spiHandle->Instance == MICROPHONE_SPI)
    {
        PeriphClkInit.PeriphClockSelection = MICROPHONE_SPI_PERIPH_CLK;
        PeriphClkInit.MICROPHONE_SPI_PERIPHCLOCKSELECTION = MICROPHONE_SPI_SOURCE_CLK;

        __HAL_RCC_I2S2_CONFIG(RCC_I2S2CLKSOURCE_HSI);

        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            msp_error_handler();
        }
        /* Peripheral clock enable */
        MICROPHONE_SPI_CLK_ENABLE();
        MICROPHONE_SPI_GPIO_CLK_ENABLE();

        gpio_init_structure.Pin = MICROPHONE_SPI_SCK_PIN | MICROPHONE_SPI_MISO_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
        gpio_init_structure.Alternate = MICROPHONE_SPI_AF;

        //MICROPHONE_SPI_MISO_PIN

        HAL_GPIO_Init(MICROPHONE_SPI_GPIO_PORT, &gpio_init_structure);


#if 1
        /* Configure the DMA handler for EX*/
        UAIR_BSP_microphone_hdma_rx.Instance = MICROPHONE_RX_DMA_CHANNEL;
        UAIR_BSP_microphone_hdma_rx.Init.Request = MICROPHONE_RX_DMA_REQUEST;
        UAIR_BSP_microphone_hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        UAIR_BSP_microphone_hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        UAIR_BSP_microphone_hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
        UAIR_BSP_microphone_hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        UAIR_BSP_microphone_hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        UAIR_BSP_microphone_hdma_rx.Init.Mode = DMA_CIRCULAR;
        UAIR_BSP_microphone_hdma_rx.Init.Priority = DMA_PRIORITY_LOW;

        if (HAL_DMA_Init(&UAIR_BSP_microphone_hdma_rx) != HAL_OK)
        {
            msp_error_handler();
        }

        if (HAL_DMA_ConfigChannelAttributes(&UAIR_BSP_microphone_hdma_rx, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            msp_error_handler();
        }

        /* Associate the initialized DMA handle to the SPI handle */
        __HAL_LINKDMA(spiHandle, hdmarx, UAIR_BSP_microphone_hdma_rx);

#endif
        /* NVIC for SPI */
        HAL_NVIC_SetPriority(MICROPHONE_IRQn, MICROPHONE_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(MICROPHONE_IRQn);
      //  HAL_NVIC_SetPriority(MICROPHONE_DMA_RX_IRQn, MICROPHONE_DMA_IT_PRIORITY, 0);
      //  HAL_NVIC_EnableIRQ(MICROPHONE_DMA_RX_IRQn);
    }
    else
    {
        msp_error_handler();
    }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef *i2sHandle)
{
    if (i2sHandle->Instance == MICROPHONE_SPI)
    {
        /* Reset peripherals */
        MICROPHONE_SPI_FORCE_RESET();
        MICROPHONE_SPI_RELEASE_RESET();

        /* Peripheral clock disable */
        MICROPHONE_SPI_CLK_DISABLE();

        HAL_GPIO_DeInit(MICROPHONE_SPI_GPIO_PORT, MICROPHONE_SPI_MISO_PIN | MICROPHONE_SPI_SCK_PIN );
    }
    else
    {
        msp_error_handler();
    }
}


#else

void HAL_SPI_MspInit(SPI_HandleTypeDef *spiHandle)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    //RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    if (spiHandle->Instance == MICROPHONE_SPI)
    {
        //LLD('@');

#ifdef  MICROPHONE_SPI_PERIPH_CLK
        PeriphClkInit.PeriphClockSelection = MICROPHONE_SPI_PERIPH_CLK;
        PeriphClkInit.MICROPHONE_SPI_PERIPHCLOCKSELECTION = MICROPHONE_SPI_SOURCE_CLK;

        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
        {
            msp_error_handler();
        }
#endif
        /* Peripheral clock enable */
        MICROPHONE_SPI_CLK_ENABLE();
        MICROPHONE_SPI_GPIO_CLK_ENABLE();

        MICROPHONE_DMAMUX_CLK_ENABLE();
        MICROPHONE_DMA_CLK_ENABLE();

        HAL_NVIC_SetPriority(MICROPHONE_DMA_RX_IRQn, MICROPHONE_DMA_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(MICROPHONE_DMA_RX_IRQn);


        gpio_init_structure.Pin = MICROPHONE_SPI_SCK_PIN ;
        gpio_init_structure.Mode = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
        gpio_init_structure.Alternate = MICROPHONE_SPI_AF;

        HAL_GPIO_Init(MICROPHONE_SPI_SCK_PORT, &gpio_init_structure);
#if 0
        gpio_init_structure.Pin = MICROPHONE_SPI_MISO_PIN ;

        HAL_GPIO_Init(MICROPHONE_SPI_MISO_PORT, &gpio_init_structure);
#endif
        gpio_init_structure.Pin = MICROPHONE_SPI_MOSI_PIN ;

        HAL_GPIO_Init(MICROPHONE_SPI_MOSI_PORT, &gpio_init_structure);


        /* Configure the DMA handler for EX*/
        UAIR_BSP_microphone_hdma_rx.Instance = MICROPHONE_RX_DMA_CHANNEL;
        UAIR_BSP_microphone_hdma_rx.Init.Request = MICROPHONE_RX_DMA_REQUEST;
        UAIR_BSP_microphone_hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        UAIR_BSP_microphone_hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        UAIR_BSP_microphone_hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
        UAIR_BSP_microphone_hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        UAIR_BSP_microphone_hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        UAIR_BSP_microphone_hdma_rx.Init.Mode = DMA_NORMAL;
        UAIR_BSP_microphone_hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;

        if (HAL_DMA_Init(&UAIR_BSP_microphone_hdma_rx) != HAL_OK)
        {
            msp_error_handler();
        }

        if (HAL_DMA_ConfigChannelAttributes(&UAIR_BSP_microphone_hdma_rx, DMA_CHANNEL_NPRIV) != HAL_OK)
        {
            msp_error_handler();
        }

        /* Associate the initialized DMA handle to the SPI handle */
        __HAL_LINKDMA(spiHandle, hdmarx, UAIR_BSP_microphone_hdma_rx);

        /* NVIC for SPI */
        HAL_NVIC_SetPriority(MICROPHONE_IRQn, MICROPHONE_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(MICROPHONE_IRQn);

    }
    else
    {
        msp_error_handler();
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *spiHandle)
{
    if (spiHandle->Instance == MICROPHONE_SPI)
    {
        /* Reset peripherals */
        MICROPHONE_SPI_FORCE_RESET();
        MICROPHONE_SPI_RELEASE_RESET();

        /* Peripheral clock disable */
        MICROPHONE_SPI_CLK_DISABLE();

        HAL_GPIO_DeInit(MICROPHONE_SPI_SCK_PORT, MICROPHONE_SPI_SCK_PIN );
        HAL_GPIO_DeInit(MICROPHONE_SPI_MISO_PORT, MICROPHONE_SPI_MISO_PIN );
    }
    else
    {
        msp_error_handler();
    }
}

#endif



#if 0

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *timerHandle)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    if (timerHandle->Instance == BUZZER_TIMER)
    {
        BUZZER_TIMER_CLK_ENABLE();
        BUZZER_TIMER_GPIO_CLK_ENABLE();

        gpio_init_structure.Pin = BUZZER_TIMER_PWM_PIN;
        gpio_init_structure.Mode = GPIO_MODE_AF_PP;
        gpio_init_structure.Pull = GPIO_PULLUP;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_init_structure.Alternate = BUZZER_TIMER_AF;
        HAL_GPIO_Init(BUZZER_TIMER_PWM_PORT, &gpio_init_structure);
        HAL_GPIO_WritePin(BUZZER_TIMER_PWM_PORT, BUZZER_TIMER_PWM_PIN, GPIO_PIN_RESET);

        HAL_NVIC_SetPriority(BUZZER_TIMER_IRQn, UAIR_BSP_BUZZER_TIMER_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(BUZZER_TIMER_IRQn);
    }
    else
    {
        msp_error_handler();
    }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *timerHandle)
{
    if (timerHandle->Instance == BUZZER_TIMER)
    {
        BUZZER_TIMER_CLK_DISABLE();

        HAL_GPIO_DeInit(BUZZER_TIMER_PWM_PORT, BUZZER_TIMER_PWM_PIN);

        HAL_NVIC_DisableIRQ(BUZZER_TIMER_IRQn);
    }
    else
    {
        msp_error_handler();
    }
}
#endif

void HAL_SUBGHZ_MspInit(SUBGHZ_HandleTypeDef *subghzHandle)
{
    /* SUBGHZ clock enable */
    __HAL_RCC_SUBGHZSPI_CLK_ENABLE();

    /* SUBGHZ interrupt Init */
    HAL_NVIC_SetPriority(SUBGHZ_Radio_IRQn, UAIR_BSP_SUBGHZ_RADIO_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);
}

void HAL_SUBGHZ_MspDeInit(SUBGHZ_HandleTypeDef *subghzHandle)
{
    /* Peripheral clock disable */
    __HAL_RCC_SUBGHZSPI_CLK_DISABLE();

    /* SUBGHZ interrupt Deinit */
    HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *rtcHandle)
{

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (rtcHandle->Instance == RTC)
    {
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            msp_error_handler();
        }

        /* RTC clock enable */
        __HAL_RCC_RTC_ENABLE();
        __HAL_RCC_RTCAPB_CLK_ENABLE();

        /* RTC interrupt Init */
        HAL_NVIC_SetPriority(TAMP_STAMP_LSECSS_SSRU_IRQn, UAIR_BSP_RTC_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(TAMP_STAMP_LSECSS_SSRU_IRQn);
        HAL_NVIC_SetPriority(RTC_Alarm_IRQn, UAIR_BSP_RTC_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
        //LLD('P');
    }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef *rtcHandle)
{
    if (rtcHandle->Instance == RTC)
    {
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();
        __HAL_RCC_RTCAPB_CLK_DISABLE();

        /* RTC interrupt Deinit */
        HAL_NVIC_DisableIRQ(TAMP_STAMP_LSECSS_SSRU_IRQn);
        HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
    }
}


void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
  if(adcHandle->Instance == VBAT_ADC)
  {
    __HAL_RCC_ADC_CLK_ENABLE();
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
  if(adcHandle->Instance == VBAT_ADC)
  {
    __HAL_RCC_ADC_CLK_DISABLE();
  }
}

static void msp_error_handler(void)
{
    while (1)
    {
    }
}

void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef *hlptim)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hlptim->Instance == LPTIM1) {
        // TBD.
        BSP_TRACE("LPTIM init");
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
        PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK1;
        //PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;

        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            msp_error_handler();
        }

        __HAL_RCC_LPTIM1_CLK_ENABLE();

        HAL_NVIC_SetPriority(LPTIM1_IRQn, UAIR_BSP_LPTIM1_IT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
    } else {
        BSP_TRACE("Invalid LPTIM?");
    }
}
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef *hlptim)
{
    if (hlptim->Instance == LPTIM1) {
        __HAL_RCC_LPTIM1_CLK_DISABLE();
    }
}

