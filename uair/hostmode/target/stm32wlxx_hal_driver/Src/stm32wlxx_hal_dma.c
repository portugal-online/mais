#include "stm32wlxx_hal_def.h"
#include "stm32wlxx_hal.h"
#include <stdlib.h>
#include "models/hw_dma.h"
#include "hlog.h"

DECLARE_LOG_TAG(HAL_DMA)
#define TAG "HAL_DMA"


void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
    DMA_Channel_TypeDef *chan = hdma->Instance;
    if (chan->Offset==0) {
        hdma->XferCpltCallback(hdma);
    } else {
        hdma->XferHalfCpltCallback(hdma);
    }
}

HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma)
{
    hdma->Instance->Request = hdma->Init.Request;
    hdma->Instance->Direction = hdma->Init.Direction;
    hdma->Instance->PeriphInc = hdma->Init.PeriphInc;
    hdma->Instance->MemInc = hdma->Init.MemInc;
    hdma->Instance->PeriphDataAlignment = hdma->Init.PeriphDataAlignment;
    hdma->Instance->MemDataAlignment = hdma->Init.MemDataAlignment;
    hdma->Instance->Mode = hdma->Init.Mode;

    HLOG(TAG, "DMA init req=%d instance=%d",  hdma->Instance->Request, hdma->Instance->id );

    dma_mux_configure_request(hdma->Init.Request, hdma->Instance->id );


    return HAL_OK;
}


HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef *hdma, size_t SrcAddress, size_t DstAddress, uint32_t DataLength)
{
    dma_channel_start( hdma->Instance, SrcAddress, DstAddress, DataLength);
    return HAL_OK;
}


HAL_StatusTypeDef HAL_DMA_ConfigChannelAttributes(DMA_HandleTypeDef *hdma, uint32_t ChannelAttributes)
{
    return HAL_OK;
}
