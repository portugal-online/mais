#ifndef LL_DMAMUX_H
#define LL_DMAMUX_H


#define LL_DMAMUX_REQ_USART2_TX (0)
#define LL_DMAMUX_REQ_USART2_RX (1)
#define LL_DMAMUX_REQ_SPI2_RX (2)
#define LL_DMAMUX_REQ_SPI2_TX (3)

#define LL_DMA_DIRECTION_MEMORY_TO_PERIPH (0)
#define LL_DMA_DIRECTION_PERIPH_TO_MEMORY (1)

#define LL_DMA_PERIPH_NOINCREMENT (0)
#define LL_DMA_MEMORY_INCREMENT (1)

#define LL_DMA_PDATAALIGN_BYTE (0)
#define LL_DMA_MDATAALIGN_BYTE (0)

#define LL_DMA_MODE_NORMAL (0)

#define LL_DMA_PRIORITY_LOW (0)
#define LL_DMA_PRIORITY_HIGH (1)

#endif