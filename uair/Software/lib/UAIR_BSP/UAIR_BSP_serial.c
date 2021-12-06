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
 * @file UAIR_BSP_serial.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP_serial.h"
#include "UAIR_BSP.h"

UART_HandleTypeDef UAIR_BSP_debug_usart;
DMA_HandleTypeDef UAIR_BSP_debug_hdma_tx;
#ifdef UAIR_UART_RX_DMA
DMA_HandleTypeDef UAIR_BSP_debug_hdma_rx;
#endif
/**
 * @brief Init the UART interface for debugging.
 *
 * @return UAIR_BSP status
 */
int32_t UAIR_BSP_USART_Init(void)
{
  UAIR_BSP_debug_usart.Instance = DEBUG_USART;
  UAIR_BSP_debug_usart.Init.BaudRate = DEBUG_USART_BAUDRATE;
  UAIR_BSP_debug_usart.Init.WordLength = UART_WORDLENGTH_8B;
  UAIR_BSP_debug_usart.Init.StopBits = UART_STOPBITS_1;
  UAIR_BSP_debug_usart.Init.Parity = UART_PARITY_NONE;
  UAIR_BSP_debug_usart.Init.Mode = UART_MODE_TX_RX;
  UAIR_BSP_debug_usart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UAIR_BSP_debug_usart.Init.OverSampling = UART_OVERSAMPLING_16;
  UAIR_BSP_debug_usart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  UAIR_BSP_debug_usart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  UAIR_BSP_debug_usart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

  if (HAL_UART_Init(&UAIR_BSP_debug_usart) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }

  if (HAL_UARTEx_SetTxFifoThreshold(&UAIR_BSP_debug_usart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&UAIR_BSP_debug_usart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }
  if (HAL_UARTEx_EnableFifoMode(&UAIR_BSP_debug_usart) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }

  return BSP_ERROR_NONE;
}

/**
 * @brief Enable DMA controller clock.
 *
 * @return UAIR_BSP status
 */
int32_t UAIR_BSP_UART_DMA_Init(void)
{
  /* DMA controller clock enable */
  DEBUG_USART_DMAMUX_CLK_ENABLE();
  DEBUG_USART_DMA_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DEBUG_USART_DMA_TX_IRQn, DEBUG_USART_DMA_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DEBUG_USART_DMA_TX_IRQn);

#ifdef UAIR_UART_RX_DMA
  HAL_NVIC_SetPriority(DEBUG_USART_DMA_RX_IRQn, DEBUG_USART_DMA_IT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(DEBUG_USART_DMA_RX_IRQn);
#endif

  return BSP_ERROR_NONE;
}

// TBD: RAM function?
UART_HandleTypeDef *BSP_get_debug_usart_handle(void)
{
    return &UAIR_BSP_debug_usart;
}

// TBD: RAM function?
DMA_HandleTypeDef *BSP_get_debug_dma_tx_handle(void)
{
    return &UAIR_BSP_debug_hdma_tx;
}
