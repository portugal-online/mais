#ifndef UAIR_BSP_MICROPHONE_P_H__
#define UAIR_BSP_MICROPHONE_P_H__

#include "UAIR_BSP_error.h"
#include "HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t UAIR_BSP_microphone_init(void);

int32_t UAIR_BSP_MICROPHONE_Start(void);
int32_t UAIR_BSP_MICROPHONE_SwitchNormalMode(void);
int32_t UAIR_BSP_MICROPHONE_SwitchZPL(void);

// Callbacks from SPI
void UAIR_BSP_MICROPHONE_RxCpltCallback(void);
void UAIR_BSP_MICROPHONE_RxHalfCpltCallback(void);


#ifdef MICROPHONE_USE_I2S
extern I2S_HandleTypeDef UAIR_BSP_microphone_i2s;
#else
extern SPI_HandleTypeDef UAIR_BSP_microphone_spi;
extern DMA_HandleTypeDef UAIR_BSP_microphone_hdma_rx;
#endif


    //NOTE: for rev2. microphone sits on I2C2

#define MICROPHONE_SPI SPI2

#define MICROPHONE_LEFT 0
#define MICROPHONE_RIGHT 1

#define MICROPHONE_CHANNEL MICROPHONE_RIGHT

    /* Speed of the SPI interface depends on both the system clock speed
     (24 MHz is default) and a prescaler */
#define MICROPHONE_SPI_BAUDRATE            SPI_BAUDRATEPRESCALER_2

#define MICROPHONE_SPI_CLK_ENABLE()  __HAL_RCC_SPI2_CLK_ENABLE()
#define MICROPHONE_SPI_CLK_DISABLE() __HAL_RCC_SPI2_CLK_DISABLE()

#define MICROPHONE_SPI_GPIO_CLK_ENABLE() do { \
    __HAL_RCC_GPIOA_CLK_ENABLE(); \
    __HAL_RCC_GPIOB_CLK_ENABLE(); } while (0)

#define MICROPHONE_SPI_FORCE_RESET()   __HAL_RCC_SPI2_FORCE_RESET()
#define MICROPHONE_SPI_RELEASE_RESET() __HAL_RCC_SPI2_RELEASE_RESET()

#define MICROPHONE_SPI_SCK_PORT GPIOB
#define MICROPHONE_SPI_SCK_PIN GPIO_PIN_13

#define MICROPHONE_SPI_MISO_PORT GPIOA
#define MICROPHONE_SPI_MISO_PIN GPIO_PIN_5

#define MICROPHONE_SPI_MOSI_PORT GPIOA
#define MICROPHONE_SPI_MOSI_PIN GPIO_PIN_10

#define MICROPHONE_SPI_AF GPIO_AF5_SPI2

#define MICROPHONE_SPI_PERIPHCLOCKSELECTION I2s2ClockSelection
#define MICROPHONE_SPI_PERIPH_CLK RCC_PERIPHCLK_I2S2
#define MICROPHONE_SPI_SOURCE_CLK RCC_I2S2CLKSOURCE_HSI

#define MICROPHONE_DMA_CLK_ENABLE()           __HAL_RCC_DMA1_CLK_ENABLE()
#define MICROPHONE_DMAMUX_CLK_ENABLE()        __HAL_RCC_DMAMUX1_CLK_ENABLE()

#define MICROPHONE_RX_DMA_REQUEST             DMA_REQUEST_SPI2_RX
#define MICROPHONE_RX_DMA_CHANNEL             DMA1_Channel1

#define MICROPHONE_DMA_RX_IRQn                DMA1_Channel1_IRQn
#define MICROPHONE_DMA_RX_IRQHandler          DMA1_Channel1_IRQHandler

#define MICROPHONE_IRQn                       SPI2_IRQn

#ifdef __cplusplus
}
#endif

#endif
