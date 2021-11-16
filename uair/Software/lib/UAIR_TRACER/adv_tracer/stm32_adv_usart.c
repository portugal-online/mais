/**
 ******************************************************************************
 * @file    stm32_adv_usart.c
 * @author  MCD Application Team
 * @brief   Configuration of UART interface for hyperterminal communication
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32_adv_usart.h"

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief Trace driver callbacks handler
  */
const ADV_TRACER_Driver_s UTIL_TraceDriver =
{
  vcom_Init,
  vcom_DeInit,
  vcom_ReceiveInit,
  vcom_Trace_DMA,
};
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/**
  * @brief UART handle
  */
extern UART_HandleTypeDef UAIR_BSP_debug_usart;
#define UartHandle UAIR_BSP_debug_usart

/**
  * @brief buffer to receive 1 character
  */
uint8_t charRx;

/* Private function prototypes -----------------------------------------------*/
/**
* @brief  TX complete callback
* @param  None
* @return none
*/
static void (*TxCpltCallback)(void*);
/**
  * @brief  RX complete callback
  * @param  rxChar ptr of chars buffer sent by user
  * @param  size buffer size
  * @param  error errorcode
  * @return none
  */
static void (*RxCpltCallback)(uint8_t *rxChar, uint16_t size, uint8_t error);

/* Functions Definition ------------------------------------------------------*/

ADV_TRACER_Status_t vcom_Init(void (*cb)(void *))
{
  TxCpltCallback = cb;
  UAIR_BSP_UART_DMA_Init();
  UAIR_BSP_USART_Init();
  return ADV_TRACER_OK;
}

ADV_TRACER_Status_t vcom_DeInit(void)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_USART2_FORCE_RESET();
  __HAL_RCC_USART2_RELEASE_RESET();

  /*##-2- MspDeInit ##################################################*/
  HAL_UART_MspDeInit(&UartHandle);

  /*##-3- Disable the NVIC for DMA ###########################################*/
  /* temorary while waiting CR 50840: MX implementation of  MX_DMA_DeInit() */
  /* For the time being user should change mannualy the channel according to the MX settings */
  HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);

  return ADV_TRACER_OK;
}

void vcom_Trace(uint8_t *p_data, uint16_t size)
{
  HAL_UART_Transmit(&UartHandle, p_data, size, 1000);
}

ADV_TRACER_Status_t vcom_Trace_DMA(uint8_t *p_data, uint16_t size)
{
  HAL_UART_Transmit_DMA(&UartHandle, p_data, size);
  return ADV_TRACER_OK;
}

ADV_TRACER_Status_t vcom_ReceiveInit(void (*RxCb)(uint8_t *rxChar, uint16_t size, uint8_t error))
{
  UART_WakeUpTypeDef WakeUpSelection;

  /*record call back*/
  RxCpltCallback = RxCb;

  /*Set wakeUp event on start bit*/
  WakeUpSelection.WakeUpEvent = UART_WAKEUP_ON_STARTBIT;

  HAL_UARTEx_StopModeWakeUpSourceConfig(&UartHandle, WakeUpSelection);

  /* Make sure that no UART transfer is on-going */
  while (__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_BUSY) == SET);

  /* Make sure that UART is ready to receive)   */
  while (__HAL_UART_GET_FLAG(&UartHandle, USART_ISR_REACK) == RESET);

  /* Enable USART interrupt */
  __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_WUF);

  /*Enable wakeup from stop mode*/
  HAL_UARTEx_EnableStopMode(&UartHandle);

  /*Start LPUART receive on IT*/
  HAL_UART_Receive_IT(&UartHandle, &charRx, 1);

  return ADV_TRACER_OK;
}

void vcom_Resume(void)
{
  /*to re-enable lost UART & DMA settings*/
  HAL_UART_Init(BSP_get_debug_usart_handle());

  HAL_DMA_Init(BSP_get_debug_dma_tx_handle());
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* buffer transmission complete*/
  TxCpltCallback(NULL);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  if ((NULL != RxCpltCallback) && (HAL_UART_ERROR_NONE == UartHandle->ErrorCode))
  {
    RxCpltCallback(&charRx, 1, 0);
  }
  HAL_UART_Receive_IT(UartHandle, &charRx, 1);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
