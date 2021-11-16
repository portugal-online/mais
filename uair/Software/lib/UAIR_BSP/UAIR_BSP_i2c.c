/** 
 *  Copyright (C) 2021 MAIS Project
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
 * @file UAIR_BSP_i2c.c
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP_i2c.h"
#include "pvt/UAIR_BSP_i2c_p.h"
#include "UAIR_BSP.h"
#include "HAL_gpio.h"

static struct i2c_bus_def i2c3 = {
    .sda = {
        .port = GPIOC,
        .pin = GPIO_PIN_1,
        .af = GPIO_AF4_I2C3,
        .clock_control = HAL_clk_GPIOC_clock_control
    },
    .scl = {
        .port = GPIOC,
        .pin = GPIO_PIN_0,
        .af = GPIO_AF4_I2C3,
        .clock_control = HAL_clk_GPIOC_clock_control
    },
    .i2c_bus = I2C3,
    .i2c_clock_control = HAL_clk_I2C3_clock_control
};



static struct i2c_bus_def *i2c_bus_r1[] = {
    &i2c3,
    NULL,
    NULL,
};
/*
const struct i2c_bus_def *i2c_bus_r2[] = {
    &i2c1,
    &i2c2,
    &i2c3
};
 */
// Bus instances

static I2C_HandleTypeDef i2c_buses[BSP_I2C_MAX_BUS];

static BSP_error_t UAIR_BSP_I2C_Bus_Init_Internal(HAL_I2C_bus_t bus_instance, const struct i2c_bus_def *bus_def);

BSP_error_t UAIR_BSP_I2C_InitAll()
{
    BSP_error_t err;
    unsigned i;
    do {
        for (i=0; i<BSP_I2C_MAX_BUS;i++) {
            err = UAIR_BSP_I2C_InitBus(i);
            if (err!=BSP_ERROR_NONE)
                break;
        }
    } while (0);
    return err;
}

BSP_error_t UAIR_BSP_I2C_InitBus(BSP_I2C_busnumber_t busno)
{
    BSP_error_t err = BSP_ERROR_NONE;
    BSP_TRACE("Initialising I2C bus %d", busno);
    if (i2c_bus_r1[busno]) {
        err = UAIR_BSP_I2C_Bus_Init_Internal(&i2c_buses[busno], i2c_bus_r1[busno]);
    }
    return err;
}

static BSP_error_t UAIR_BSP_I2C_Bus_Init_Internal(HAL_I2C_bus_t bus_instance, const struct i2c_bus_def *bus_def)
{
    bus_instance->Instance = bus_def->i2c_bus;
    bus_instance->Init.Timing = EXT_SENSOR_I2C3_TIMING; // I2C3 bus frequency config
    bus_instance->Init.OwnAddress1 = 0x00;
    bus_instance->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    bus_instance->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    bus_instance->Init.OwnAddress2 = 0x00;
    bus_instance->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    bus_instance->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(bus_instance) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    /* Enable the Analog I2C Filter */
    if (HAL_I2CEx_ConfigAnalogFilter(bus_instance, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    /** Configure Digital filter
     */
    if (HAL_I2CEx_ConfigDigitalFilter(bus_instance, 0) != HAL_OK)
    {
        return BSP_ERROR_NO_INIT;
    }

    //HAL_I2CEx_EnableFastModePlus(EXT_SENSOR_I2C3_FASTMODEPLUS);
    {
        uint8_t data[2];
    HAL_I2C_Master_Receive(bus_instance, (uint16_t)(0xAA << 1),
                                      data, 2, 1000);
    }
    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_I2C_Bus_ResumeAll(void)
{
    BSP_error_t err = BSP_ERROR_NONE;
    unsigned i;
    for (i=0;i<BSP_I2C_MAX_BUS;i++) {
        if (i2c_bus_r1[i]!=NULL) {
            err = UAIR_BSP_I2C_Bus_Resume(i2c_bus_r1[i]);
            if (err!=BSP_ERROR_NONE)
                break;
        }
    }
    return err;
}

BSP_error_t UAIR_BSP_I2C_Bus_Resume(const struct i2c_bus_def *bus_def)
{
    bus_def->sda.clock_control(1);
    bus_def->scl.clock_control(1);
    bus_def->i2c_clock_control(1);

    return BSP_ERROR_NONE;
}

/**
 * @brief Deinit the external sensors I2C3 bus.
 *
 * @return UAIR_BSP status
 */
BSP_error_t UAIR_BSP_I2C_Bus_DeInit(HAL_I2C_bus_t bus)
{
  if (HAL_I2C_DeInit(bus) != HAL_OK)
  {
    return BSP_ERROR_NO_INIT;
  }

//  HAL_I2CEx_DisableFastModePlus(EXT_SENSOR_I2C3_FASTMODEPLUS);

  return BSP_ERROR_NONE;
}

struct i2c_bus_def *UAIR_BSP_I2C_GetBusDef(BSP_I2C_busnumber_t busno)
{
    return i2c_bus_r1[(int)busno];
}
HAL_I2C_bus_t UAIR_BSP_I2C_GetHALHandle(BSP_I2C_busnumber_t busno)
{
    return &i2c_buses[busno];
}

