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
 * @file UAIR_BSP_serial.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */


#ifndef UAIR_BSP_SERIAL_H
#define UAIR_BSP_SERIAL_H

#include "stm32wlxx_hal.h"
#include "UAIR_BSP_error.h"
#include "UAIR_BSP_conf.h"

//extern I2C_HandleTypeDef UAIR_BSP_ext_sensor_i2c3;
//extern SPI_HandleTypeDef UAIR_BSP_flash_spi;
//extern UART_HandleTypeDef UAIR_BSP_debug_usart;
//extern DMA_HandleTypeDef UAIR_BSP_debug_hdma_tx;

/**
 * HAL defines
 * Configure below for any pin change
 */

#define DEBUG_USART USART2
#define DEBUG_USART_BAUDRATE 115200
#define DEBUG_USART_PERIPH_CLK RCC_PERIPHCLK_USART2
#define DEBUG_USART_SOURCE_CLK RCC_USART2CLKSOURCE_SYSCLK //If adjusted, Update Usart2ClockSelection in UAIR_msp.c

#define DEBUG_USART_CLK_CONTROL        HAL_clk_USART2_clock_control

#define DEBUG_USART_TX_PIN GPIO_PIN_2
#define DEBUG_USART_TX_GPIO_PORT GPIOA
#define DEBUG_USART_RX_PIN GPIO_PIN_3
#define DEBUG_USART_RX_GPIO_PORT GPIOA

#define DEBUG_USART_RX_GPIO_CLK_CONTROL      HAL_clk_GPIOA_clock_control
#define DEBUG_USART_TX_GPIO_CLK_CONTROL      HAL_clk_GPIOA_clock_control

#define DEBUG_USART_TX_AF                     GPIO_AF7_USART2
#define DEBUG_USART_RX_AF                     GPIO_AF7_USART2

#define DEBUG_USART_EXTI_WAKEUP               LL_EXTI_LINE_27

#define DEBUG_USART_DMA_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define DEBUG_USART_DMAMUX_CLK_ENABLE()              __HAL_RCC_DMAMUX1_CLK_ENABLE()

#define DEBUG_USART_TX_DMA_REQUEST             DMA_REQUEST_USART2_TX
#define DEBUG_USART_TX_DMA_CHANNEL             DMA1_Channel5

#define DEBUG_USART_DMA_TX_IRQn                DMA1_Channel5_IRQn
#define DEBUG_USART_DMA_TX_IRQHandler          DMA1_Channel5_IRQHandler

#define DEBUG_USART_IRQn                      USART2_IRQn

#define EXT_SENSOR_I2C1                  I2C1
#define EXT_SENSOR_I2C2                  I2C2
#define EXT_SENSOR_I2C3                  I2C3
/* EXT_SENSOR_I2C3_TIMING is set at 100 kHz
 * This setting is altered by both the clock speed and I2C setting
 * Clock speed is set in SystemClock_Config, default is 48 MHz
 * I2C setting is set in UAIR_BSP_Sensor_I2C1_Init, default is Fast mode plus
 * Changing any of these variables without reconfiguring this define could break the I2C communication
 */

/*
 Timing was calculated using STM32Cube for sysclk=24Mhz

 Values for 24Mhz:
 100KHz: 0x00506682
 400KHz: 0x00200C28
 1MHz  : 0x0010030D
 */
#define I2C_SPEED_SYSCLK24_100KHZ 0x00506682
#define I2C_SPEED_SYSCLK24_400KHZ 0x00200C28
#define I2C_SPEED_SYSCLK24_1MHZ   0x0010030D


/* Internal I2C bus (SHTC3) */
#define EXT_SENSOR_I2C1_TIMING           I2C_SPEED_SYSCLK24_100KHZ
#define EXT_SENSOR_I2C1_TIMEOUT          100U //Read and write operations timeout in ms
#define EXT_SENSOR_I2C1_PERIPH_CLK      RCC_PERIPHCLK_I2C1
#define EXT_SENSOR_I2C1_SOURCE_CLK      RCC_I2C1CLKSOURCE_SYSCLK
#define EXT_SENSOR_I2C1_FASTMODEPLUS    I2C_FASTMODEPLUS_I2C1
#define EXT_SENSOR_I2C1_CLK_ENABLE()    HAL_clk_I2C1_clock_control(1)
#define EXT_SENSOR_I2C1_CLK_DISABLE()   HAL_clk_I2C1_clock_control(0)
#define EXT_SENSOR_I2C1_SDA_GPIO_CLK_ENABLE()      HAL_clk_GPIOA_clock_control(1)
#define EXT_SENSOR_I2C1_SCL_GPIO_CLK_ENABLE()      HAL_clk_GPIOB_clock_control(1)
#define EXT_SENSOR_I2C1_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define EXT_SENSOR_I2C1_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
#define EXT_SENSOR_I2C1_SCL_PIN                    GPIO_PIN_8
#define EXT_SENSOR_I2C1_SCL_GPIO_PORT              GPIOB
#define EXT_SENSOR_I2C1_SDA_PIN                    GPIO_PIN_10
#define EXT_SENSOR_I2C1_SDA_GPIO_PORT              GPIOA
#define EXT_SENSOR_I2C1_SCL_SDA_AF                 GPIO_AF4_I2C1

/* Microphone I2C bus  */
#define EXT_SENSOR_I2C2_TIMING           I2C_SPEED_SYSCLK24_100KHZ
#define EXT_SENSOR_I2C2_TIMEOUT          100U //Read and write operations timeout in ms
#define EXT_SENSOR_I2C2_PERIPH_CLK      RCC_PERIPHCLK_I2C2
#define EXT_SENSOR_I2C2_SOURCE_CLK      RCC_I2C2CLKSOURCE_SYSCLK
#define EXT_SENSOR_I2C2_FASTMODEPLUS    I2C_FASTMODEPLUS_I2C2
#define EXT_SENSOR_I2C2_CLK_ENABLE()    HAL_clk_I2C2_clock_control(1)
#define EXT_SENSOR_I2C2_CLK_DISABLE()   HAL_clk_I2C2_clock_control(0)
#define EXT_SENSOR_I2C2_SDA_GPIO_CLK_ENABLE()      HAL_clk_GPIOA_clock_control(1)
#define EXT_SENSOR_I2C2_SCL_GPIO_CLK_ENABLE()      HAL_clk_GPIOA_clock_control(1)
#define EXT_SENSOR_I2C2_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define EXT_SENSOR_I2C2_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
#define EXT_SENSOR_I2C2_SCL_PIN                    GPIO_PIN_12
#define EXT_SENSOR_I2C2_SCL_GPIO_PORT              GPIOA
#define EXT_SENSOR_I2C2_SDA_PIN                    GPIO_PIN_11
#define EXT_SENSOR_I2C2_SDA_GPIO_PORT              GPIOA
#define EXT_SENSOR_I2C2_SCL_SDA_AF                 GPIO_AF4_I2C1


#define EXT_SENSOR_I2C3_TIMING           I2C_SPEED_SYSCLK24_100KHZ
#define EXT_SENSOR_I2C3_TIMEOUT          100U //Read and write operations timeout in ms
#define EXT_SENSOR_I2C3_PERIPH_CLK      RCC_PERIPHCLK_I2C3
#define EXT_SENSOR_I2C3_SOURCE_CLK      RCC_I2C3CLKSOURCE_SYSCLK
#define EXT_SENSOR_I2C3_FASTMODEPLUS    I2C_FASTMODEPLUS_I2C3
#define EXT_SENSOR_I2C3_CLK_ENABLE()    HAL_clk_I2C3_clock_control(1)
#define EXT_SENSOR_I2C3_CLK_DISABLE()   HAL_clk_I2C3_clock_control(0)
#define EXT_SENSOR_I2C3_SDA_GPIO_CLK_ENABLE()      HAL_clk_GPIOC_clock_control(1)
#define EXT_SENSOR_I2C3_SCL_GPIO_CLK_ENABLE()      HAL_clk_GPIOC_clock_control(1)
#define EXT_SENSOR_I2C3_FORCE_RESET()              __HAL_RCC_I2C3_FORCE_RESET()
#define EXT_SENSOR_I2C3_RELEASE_RESET()            __HAL_RCC_I2C3_RELEASE_RESET()
#define EXT_SENSOR_I2C3_SCL_PIN                    GPIO_PIN_0
#define EXT_SENSOR_I2C3_SCL_GPIO_PORT              GPIOC
#define EXT_SENSOR_I2C3_SDA_PIN                    GPIO_PIN_1
#define EXT_SENSOR_I2C3_SDA_GPIO_PORT              GPIOC
#define EXT_SENSOR_I2C3_SCL_SDA_AF                 GPIO_AF4_I2C3

/**
 * BSP Serial APIs
 */

int32_t UAIR_BSP_USART_Init(void);
int32_t UAIR_BSP_UART_DMA_Init(void);

/*int32_t UAIR_BSP_Ext_Sensor_I2C3_Init(void);
int32_t UAIR_BSP_Ext_Sensor_I2C3_Resume(void);
int32_t UAIR_BSP_Ext_Sensor_I2C3_DeInit(void);
*/

UART_HandleTypeDef *BSP_get_debug_usart_handle(void);
DMA_HandleTypeDef *BSP_get_debug_dma_tx_handle(void);

#endif /* UAIR_BSP_SERIAL_H */
