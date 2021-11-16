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
 * @file UAIR_BSP_microphone.c
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP.h"
#include "UAIR_tracer.h"

DMA_HandleTypeDef UAIR_BSP_microphone_hdma_rx;

#ifdef MICROPHONE_USE_I2S
I2S_HandleTypeDef UAIR_BSP_microphone_i2s = {0};
#else
SPI_HandleTypeDef UAIR_BSP_microphone_spi = {0};
#endif

#define MICROPHONE_MIN_FREQUENCY 20 /* 20Hz */
#define MICROPHONE_BIT_SAMPLING_SPEED 2000000 /* 2MHz */

// TBD: round this
#define MICROPHONE_CAPTURE_BITS ((MICROPHONE_BIT_SAMPLING_SPEED)/MICROPHONE_MIN_FREQUENCY)
#define MICROPHONE_CAPTURE_BYTES (MICROPHONE_CAPTURE_BITS/8)

#define MICROPHONE_BUFFER_BYTES (MICROPHONE_CAPTURE_BYTES/2)

#define MICROPHONE_DECIMATION 64

#define MICROPHONE_SAMPLE_SIZE (MICROPHONE_CAPTURE_BITS/MICROPHONE_DECIMATION)

static uint8_t microphone_spi_buffer[MICROPHONE_BUFFER_BYTES];
//static uint16_t microphone_data[MICROPHONE_SAMPLE_SIZE];


int32_t UAIR_BSP_MICROPHONE_InitDisable(void)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    MICROPHONE_SPI_GPIO_CLK_ENABLE();

    gpio_init_structure.Pin = MICROPHONE_SPI_SCK_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_WritePin(MICROPHONE_SPI_SCK_PORT, MICROPHONE_SPI_SCK_PIN, GPIO_PIN_RESET);

    HAL_GPIO_Init(MICROPHONE_SPI_SCK_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = MICROPHONE_SPI_MISO_PIN;
    gpio_init_structure.Mode = GPIO_MODE_ANALOG;

    HAL_GPIO_Init(MICROPHONE_SPI_MISO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = MICROPHONE_SPI_MOSI_PIN;

    HAL_GPIO_Init(MICROPHONE_SPI_MOSI_PORT, &gpio_init_structure);

    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_MICROPHONE_SwitchNormalMode(void)
{
    HAL_GPIO_WritePin(MICROPHONE_SPI_SCK_PORT, MICROPHONE_SPI_SCK_PIN, GPIO_PIN_SET);
    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_MICROPHONE_SwitchZPL(void)
{
    HAL_GPIO_WritePin(MICROPHONE_SPI_SCK_PORT, MICROPHONE_SPI_SCK_PIN, GPIO_PIN_RESET);
    return BSP_ERROR_NONE;
}

int32_t UAIR_BSP_MICROPHONE_Init(void)
{

    MICROPHONE_SPI_CLK_ENABLE();
    MICROPHONE_SPI_GPIO_CLK_ENABLE();

#ifdef MICROPHONE_USE_I2S

    UAIR_BSP_microphone_i2s.Instance = MICROPHONE_SPI;
    UAIR_BSP_microphone_i2s.Init.Mode = I2S_MODE_MASTER_RX;
    UAIR_BSP_microphone_i2s.Init.Standard = I2S_STANDARD_MSB;
    UAIR_BSP_microphone_i2s.Init.DataFormat = I2S_DATAFORMAT_16B;
    UAIR_BSP_microphone_i2s.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    UAIR_BSP_microphone_i2s.Init.AudioFreq = 64000;
    UAIR_BSP_microphone_i2s.Init.CPOL = I2S_CPOL_LOW;
    if (HAL_I2S_Init(&UAIR_BSP_microphone_i2s) != HAL_OK)
    {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    return BSP_ERROR_NONE;
#else
    memset(&UAIR_BSP_microphone_spi,0,sizeof(UAIR_BSP_microphone_spi));

    UAIR_BSP_microphone_spi.Instance = MICROPHONE_SPI;
    UAIR_BSP_microphone_spi.Init.Mode = SPI_MODE_MASTER;

    UAIR_BSP_microphone_spi.Init.Direction = SPI_DIRECTION_1LINE;
    UAIR_BSP_microphone_spi.Init.DataSize = SPI_DATASIZE_8BIT;
    UAIR_BSP_microphone_spi.Init.CLKPolarity = SPI_POLARITY_LOW;
    UAIR_BSP_microphone_spi.Init.CLKPhase = SPI_PHASE_1EDGE;
    UAIR_BSP_microphone_spi.Init.NSS = SPI_NSS_SOFT;

    UAIR_BSP_microphone_spi.Init.BaudRatePrescaler = MICROPHONE_SPI_BAUDRATE;
    UAIR_BSP_microphone_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    UAIR_BSP_microphone_spi.Init.TIMode = SPI_TIMODE_DISABLE;
    UAIR_BSP_microphone_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    UAIR_BSP_microphone_spi.Init.CRCPolynomial = 7;
    UAIR_BSP_microphone_spi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    UAIR_BSP_microphone_spi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    if (HAL_SPI_Init(&UAIR_BSP_microphone_spi) != HAL_OK)
    {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    return BSP_ERROR_NONE;
#endif

}

void __attribute__((weak)) UAIR_BSP_MICROPHONE_RxCpltCallback(void)
{
}

void __attribute__((weak)) UAIR_BSP_MICROPHONE_RxHalfCpltCallback(void)
{
}

#if 0
void UAIR_BSP_MICROPHONE_Dump()
{
    unsigned i;
    APP_PRINTF("const char micdata[] = {\r\n");
    for (i=0;i<sizeof(microphone_spi_buffer);i++) {
        APP_PRINTF("0x%02x, ", microphone_spi_buffer[i]);
        if (i%16 == 15) {
            APP_PRINTF("\r\n");
        }
    }
    APP_PRINTF("};\r\n");
}
#endif

int32_t UAIR_BSP_MICROPHONE_Start()
{
    while(LL_RCC_HSI_IsReady() != 1)
    {
    }

#ifdef MICROPHONE_USE_I2S
    if (HAL_I2S_Receive(&UAIR_BSP_microphone_i2s,
                        (uint16_t*)&microphone_spi_buffer[0],
                        sizeof(microphone_spi_buffer)/2, 2000)!=HAL_OK)
        return BSP_ERROR_PERIPH_FAILURE;

#else
    return HAL_SPI_Receive_DMA(&UAIR_BSP_microphone_spi,
                               &microphone_spi_buffer[0],
                               sizeof(microphone_spi_buffer));
#endif
    return BSP_ERROR_NONE;

}