/*
 * Copyright (C) 2021, 2022 MAIS Project
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
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_MICROPHONE
 *
 * uAir interfacing to microphone implementation
 *
 */

#include "UAIR_BSP.h"
#include "UAIR_tracer.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "VM3011.h"
#include "pvt/UAIR_BSP_microphone_p.h"
#include "UAIR_sensor.h"

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

static VM3011_t vm3011;

static BSP_powerzone_t UAIR_BSP_microphone_get_powerzone(void);
static BSP_I2C_busnumber_t UAIR_BSP_microphone_get_bus(void);
static void UAIR_BSP_microphone_set_faulty(void);

static BSP_sensor_state_t sensor_state = SENSOR_OFFLINE;

UAIR_sensor_ops_t microphone_sensor_ops = {
    .init = UAIR_BSP_microphone_init,
    .deinit = UAIR_BSP_microphone_deinit,
    .reset = NULL,
    .get_powerzone = UAIR_BSP_microphone_get_powerzone,
    .get_bus = UAIR_BSP_microphone_get_bus,
    .set_faulty = UAIR_BSP_microphone_set_faulty
};

static UAIR_sensor_t microphone_sensor = {
    .ops = &microphone_sensor_ops,
    .failcount = 0
};


static BSP_powerzone_t UAIR_BSP_microphone_get_powerzone(void)
{
    return UAIR_POWERZONE_MICROPHONE;
}

static BSP_I2C_busnumber_t UAIR_BSP_microphone_get_bus(void)
{
    BSP_I2C_busnumber_t i2c_busno = BSP_I2C_BUS_NONE;

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
    return i2c_busno;
}

static BSP_error_t UAIR_BSP_microphone_init_i2c()
{
    HAL_delay_us(500);

    BSP_error_t err;

    err = UAIR_BSP_I2C_Bus_Ref(UAIR_BSP_microphone_get_bus());

    if (err==BSP_ERROR_NONE)
    {
        // Init VM
        if (VM3011_Init( &vm3011, UAIR_BSP_I2C_GetHALHandle(UAIR_BSP_microphone_get_bus())) != VM3011_OP_SUCCESS)
        {
                err = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
            if (VM3011_Probe( &vm3011 ) != VM3011_OP_SUCCESS)
            {
                err = BSP_ERROR_COMPONENT_FAILURE;
            }
            else
            {
                err = BSP_ERROR_NONE;
            }
        }
        if (err != BSP_ERROR_NONE) {
            UAIR_BSP_I2C_Bus_Unref(UAIR_BSP_microphone_get_bus());
        }
    }

    return err;
}

BSP_error_t UAIR_BSP_microphone_init(void)
{
    BSP_error_t err;

    // Ensure we have no power. TBD

    //  BSP_powerzone_disable(powerzone);

    HAL_delay_us(100);

    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = MICROPHONE_SPI_SCK_PIN;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(MICROPHONE_SPI_SCK_PORT, &gpio_init_structure);
    HAL_GPIO_WritePin(MICROPHONE_SPI_SCK_PORT, MICROPHONE_SPI_SCK_PIN, GPIO_PIN_RESET);

    gpio_init_structure.Pin = MICROPHONE_SPI_MISO_PIN;
    gpio_init_structure.Mode = MICROPHONE_SPI_MISO_MODE_UNUSED;
    HAL_GPIO_Init(MICROPHONE_SPI_MISO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = MICROPHONE_SPI_MOSI_PIN;
    gpio_init_structure.Mode = MICROPHONE_SPI_MOSI_MODE_UNUSED;
    HAL_GPIO_Init(MICROPHONE_SPI_MOSI_PORT, &gpio_init_structure);




    err = BSP_powerzone_ref(UAIR_BSP_microphone_get_powerzone());

    if (err != BSP_ERROR_NONE)
        return err;

    err = UAIR_BSP_microphone_init_i2c();

    if (err==BSP_ERROR_NONE)
    {
        sensor_state = SENSOR_AVAILABLE;
    } else {
        err = BSP_powerzone_unref(UAIR_BSP_microphone_get_powerzone());
    }

    return err;
}

/**
 * @brief Get microphone gain
 * @ingroup UAIR_BSP_SENSOR_MICROPHONE
 *
 *
 * @param gain Where to store the current microphone gain (0 to 31, where 0 is louder, 31 is quieter)
 *
 * @return \ref BSP_ERROR_NONE if the readout was successful.
 * @return \ref BSP_ERROR_COMPONENT_FAILURE if any communication error occured. Action TBD
 */

BSP_error_t BSP_microphone_read_gain(uint8_t *gain)
{
    VM3011_op_result_t r = VM3011_Read_Threshold(&vm3011, gain);

    if (r==VM3011_OP_SUCCESS)
    {
        UAIR_sensor_ok(&microphone_sensor);
        return BSP_ERROR_NONE;
    } else {
        UAIR_sensor_fault_detected(&microphone_sensor);
    }

    return BSP_ERROR_COMPONENT_FAILURE;
}

static void UAIR_BSP_microphone_set_faulty(void)
{
    UAIR_BSP_microphone_deinit();
    sensor_state = SENSOR_FAULTY;
}

void UAIR_BSP_microphone_deinit()
{
    if (sensor_state == SENSOR_AVAILABLE) {
        UAIR_BSP_I2C_Bus_Unref(UAIR_BSP_microphone_get_bus());
        BSP_powerzone_unref(UAIR_BSP_microphone_get_powerzone());
    }

    sensor_state = SENSOR_OFFLINE;
}

BSP_sensor_state_t BSP_microphone_get_sensor_state(void)
{
    return sensor_state;
}
