#ifndef HW_DMA_H__
#define HW_DMA_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "stm32wlxx_hal_def.h"

int dma_channel_start(DMA_Channel_TypeDef *handle, size_t source, size_t dest, unsigned len);
size_t dma_alloc_periph_read_request( int (*read)(void *user, size_t location), void *user);
void dma_release_periph_read_request( size_t ptr );
void dma_notify(int line);
void dma_mux_configure_request(int dmamuxline, int dmachannel);



#ifdef __cplusplus
}
#endif

#endif
