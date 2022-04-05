#include "stm32wlxx_hal.h"
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include "models/console_uart.h"
#include "models/hw_dma.h"
#include <assert.h>

int uart_read_wrapper(void *user, size_t location)
{
    return console_uart_get_char();
}

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
    console_uart_init();

    huart->hdmatx = NULL;
    huart->hdmarx = NULL;

    huart->Instance->virtual_read_address = dma_alloc_periph_read_request( uart_read_wrapper, huart);


    HAL_UART_MspInit(huart);

    printf("RX DMA %p", huart->hdmarx);

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

static void UART_DMA_RxHalfCpltCallback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);

    HAL_UART_RxHalfCpltCallback(huart);

}

static void UART_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)(hdma->Parent);
    HAL_UART_RxCpltCallback(huart);

}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *dest, uint16_t size)
{
#if 1
    DMA_HandleTypeDef *hdmarx = huart->hdmarx;

    assert (hdmarx != NULL);

    hdmarx->XferHalfCpltCallback = UART_DMA_RxHalfCpltCallback;
    hdmarx->XferCpltCallback = UART_DMA_RxCpltCallback;


    HAL_StatusTypeDef r = HAL_DMA_Start(hdmarx,
                                        huart->Instance->virtual_read_address,
                                        (size_t)dest,
                                        size);


    if (r==HAL_OK) {
        return console_uart_start_dma(dest) == 0 ? HAL_OK : HAL_ERROR;
    }
#endif
    return HAL_OK;
}

void HAL_UART_IRQHandler(UART_HandleTypeDef*huart)
{
    //abort();
    
}
