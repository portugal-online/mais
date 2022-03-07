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
#include "pvt/UAIR_BSP_i2c_p.h"
#include "VM3011.h"
#include "pvt/UAIR_BSP_microphone_p.h"

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
static BSP_I2C_busnumber_t i2c_busno;
static BSP_powerzone_t powerzone;
static VM3011_t vm3011;

BSP_error_t UAIR_BSP_microphone_init(void)
{
    BSP_error_t err;
    powerzone = UAIR_POWERZONE_MICROPHONE;
    i2c_busno = BSP_I2C_BUS_NONE;

    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        i2c_busno = BSP_I2C_BUS0;
        break;
    case UAIR_NUCLEO_REV2:
        i2c_busno = BSP_I2C_BUS1;
        break;
    default:
        break;
    }
    

    if (i2c_busno==BSP_I2C_BUS_NONE) {
        return BSP_ERROR_NO_INIT;
    }

    // Ensure we have no power

    BSP_powerzone_disable(powerzone);

    HAL_delay_us(100);


    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = MICROPHONE_SPI_SCK_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(MICROPHONE_SPI_SCK_PORT, &gpio_init_structure);
    HAL_GPIO_WritePin(MICROPHONE_SPI_SCK_PORT, MICROPHONE_SPI_SCK_PIN, GPIO_PIN_RESET);

#if 0
    gpio_init_structure.Pin = MICROPHONE_SPI_MISO_PIN;
    gpio_init_structure.Mode = GPIO_MODE_ANALOG;

    HAL_GPIO_Init(MICROPHONE_SPI_MISO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = MICROPHONE_SPI_MOSI_PIN;

    HAL_GPIO_Init(MICROPHONE_SPI_MOSI_PORT, &gpio_init_structure);
#endif

    BSP_powerzone_enable(powerzone);


    HAL_delay_us(100);
    err = UAIR_BSP_I2C_InitBus(i2c_busno);

    if (err!=BSP_ERROR_NONE)
        return err;


    // Init VM
    if (VM3011_Init( &vm3011, UAIR_BSP_I2C_GetHALHandle(i2c_busno)) != VM3011_OP_SUCCESS) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    // probe
    if (VM3011_Probe( &vm3011 ) != VM3011_OP_SUCCESS) {
        return BSP_ERROR_COMPONENT_FAILURE;
    }

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

#if 0
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

#}
#endif

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
BSP_error_t BSP_microphone_read_gain(uint8_t *gain)
{
    VM3011_op_result_t r = VM3011_Read_Threshold(&vm3011, gain);
    if (r==VM3011_OP_SUCCESS) {
        return BSP_ERROR_NONE;
    }
    return BSP_ERROR_COMPONENT_FAILURE;
}
