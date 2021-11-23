/** Copyright © 2021 MAIS Project
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
 * @file UAIR_BSP_microphone.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_MICROPHONE_H__
#define UAIR_BSP_MICROPHONE_H__

#ifdef __cplusplus
extern "C" {
#endif

    //NOTE: for rev2. microphone sits on I2C2

#if 0

#define MICROPHONE_USE_I2S 1

extern DMA_HandleTypeDef UAIR_BSP_microphone_hdma_rx;
#ifdef MICROPHONE_USE_I2S
extern I2S_HandleTypeDef UAIR_BSP_microphone_i2s;
#else
extern SPI_HandleTypeDef UAIR_BSP_microphone_spi;
#endif

#define MICROPHONE_SPI SPI2

#define MICROPHONE_LEFT 0
#define MICROPHONE_RIGHT 1

#define MICROPHONE_CHANNEL MICROPHONE_RIGHT

    /* Speed of the SPI interface depends on both the system clock speed
     (24 MHz is default) and a prescaler */
#define MICROPHONE_SPI_BAUDRATE            SPI_BAUDRATEPRESCALER_4

#define MICROPHONE_SPI_CLK_ENABLE()  __HAL_RCC_SPI2_CLK_ENABLE()
#define MICROPHONE_SPI_CLK_DISABLE() __HAL_RCC_SPI2_CLK_DISABLE()

#define MICROPHONE_SPI_GPIO_CLK_ENABLE() do { \
    __HAL_RCC_GPIOA_CLK_ENABLE(); \
    __HAL_RCC_GPIOB_CLK_ENABLE(); } while (0)

#define MICROPHONE_SPI_FORCE_RESET()   __HAL_RCC_SPI2_FORCE_RESET()
#define MICROPHONE_SPI_RELEASE_RESET() __HAL_RCC_SPI2_RELEASE_RESET()

#define MICROPHONE_SPI_GPIO_PORT GPIOA
#define MICROPHONE_SPI_SCK_PIN GPIO_PIN_10

#define MICROPHONE_SPI_MISO_PORT GPIOB
#define MICROPHONE_SPI_MISO_PIN GPIO_PIN_13

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

#else

/*

TEST TEST TEST TETS

*/

#undef MICROPHONE_USE_I2S1

extern DMA_HandleTypeDef UAIR_BSP_microphone_hdma_rx;
extern SPI_HandleTypeDef UAIR_BSP_microphone_spi;

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

#endif // TEST TEST TEST

int32_t UAIR_BSP_MICROPHONE_Init(void);
int32_t UAIR_BSP_MICROPHONE_InitDisable(void);
int32_t UAIR_BSP_MICROPHONE_Start(void);

int32_t UAIR_BSP_MICROPHONE_SwitchNormalMode(void);
int32_t UAIR_BSP_MICROPHONE_SwitchZPL(void);

// Callbacks from SPI
void UAIR_BSP_MICROPHONE_RxCpltCallback(void);
void UAIR_BSP_MICROPHONE_RxHalfCpltCallback(void);


#ifdef __cplusplus
}
#endif

#endif
