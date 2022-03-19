#include "stm32wlxx_hal.h"
#include <stdlib.h>
#include <stdio.h>

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    if (huart->Instance == USART2) {
        fwrite(pData,Size,1, stdout);
    } else {
        abort();
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UARTEx_EnableFifoMode(UART_HandleTypeDef *huart)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UARTEx_DisableFifoMode(UART_HandleTypeDef *huart)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UARTEx_SetTxFifoThreshold(UART_HandleTypeDef *huart, uint32_t Threshold)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UARTEx_SetRxFifoThreshold(UART_HandleTypeDef *huart, uint32_t Threshold)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *dest, uint16_t size)
{
    return -1;
}

void HAL_UART_IRQHandler(UART_HandleTypeDef*huart)
{
    abort();
}
