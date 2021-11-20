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
#include "stm32wlxx_hal_i2c.h"

static void i2c3_reset(int is_reset)
{
    if (is_reset)
        __HAL_RCC_I2C3_FORCE_RESET();
    else
        __HAL_RCC_I2C3_RELEASE_RESET();
}

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
    .i2c_clock_control = HAL_clk_I2C3_clock_control,
    .i2c_reset = i2c3_reset
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

enum i2c_bus_idle_e {
    I2C_BUS_IDLE=0,
    I2C_BUS_STUCK_SDA=1,
    I2C_BUS_STUCK_SCL=2,
    I2C_BUS_STUCK_BOTH=3
};

static enum i2c_bus_idle_e UAIR_BSP_I2C_is_bus_idle(struct i2c_bus_def *busdef)
{
    int sda_state = HAL_GPIO_read(&busdef->sda);
    if (sda_state<0)
        return sda_state;
    int scl_state = HAL_GPIO_read(&busdef->scl);
    if (scl_state<0)
        return scl_state;
    if (sda_state!=GPIO_PIN_SET) {
        if (scl_state!=GPIO_PIN_SET) {
            return I2C_BUS_STUCK_BOTH;
        }
        return I2C_BUS_STUCK_SDA;
    }
    if (scl_state!=GPIO_PIN_SET) {
        return I2C_BUS_STUCK_SCL;
    }
    return I2C_BUS_IDLE;
}

static BSP_I2C_recover_action_t UAIR_BSP_I2C_handle_hal_error(BSP_I2C_busnumber_t busno, int32_t state, int32_t errorcode)
{
    if (state==HAL_I2C_STATE_READY) {
        /* Looks like a temporary failure */
        if (errorcode & HAL_I2C_ERROR_BERR) {
            return BSP_I2C_RECOVER_RESET_BUS; // Force bus reset
        }
        if (errorcode & HAL_I2C_ERROR_ARLO) {
            return BSP_I2C_RECOVER_RESET_ALL; // This should not happen. We are only bus master
        }
        if (errorcode & HAL_I2C_ERROR_AF) {
            return BSP_I2C_RECOVER_RETRY; // Missed ACK.
        }
        return BSP_I2C_RECOVER_RESET_ALL; // TBD: we probably want this to be fatal
    } else {
        return BSP_I2C_RECOVER_RESET_BUS; // Force bus reset
    }
}

static BSP_I2C_recover_action_t UAIR_BSP_I2C_handle_stuck(BSP_I2C_busnumber_t busno)
{
    BSP_error_t err;
    BSP_I2C_recover_action_t ret;

    // One of the lines is stuck.

    // First, teardown BUS and change the lines to input mode.
    HAL_I2C_bus_t handle = UAIR_BSP_I2C_GetHALHandle(busno);
    struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);

    err= HAL_I2C_DeInit(handle);

    if (err!=HAL_OK) {
        BSP_TRACE("Cannot de-init I2C");
        return BSP_I2C_RECOVER_FATAL_ERROR;
    }

    HAL_GPIO_configure_input(&busdef->sda);
    HAL_GPIO_configure_input(&busdef->scl);

    int sda_state = HAL_GPIO_read(&busdef->sda);
    int scl_state = HAL_GPIO_read(&busdef->scl);

    err = UAIR_BSP_I2C_Bus_Init_Internal(handle, busdef);
    if (err!=HAL_OK) {
        BSP_TRACE("Cannot re-init I2C");
        return BSP_I2C_RECOVER_FATAL_ERROR;
    }

    int new_sda_state = HAL_GPIO_read(&busdef->sda);
    int new_scl_state = HAL_GPIO_read(&busdef->scl);

    // If we were not stuck after disabling I2C controller,
    // then re-init bus should be sufficient.

    if ((sda_state==GPIO_PIN_SET) && (scl_state==GPIO_PIN_SET)) {
        // Check if we are now operational
        if ((new_sda_state!=GPIO_PIN_SET) || (new_scl_state!=GPIO_PIN_SET)) {
            // Cannot recover
            BSP_TRACE("Cannot recover from internal stuck pins");
            ret = BSP_I2C_RECOVER_FATAL_ERROR;
        } else {
            ret = BSP_I2C_RECOVER_RETRY; // Retry
        }
    } else {
        // Device is stuck.
        // This requires powering down/resetting device;
        BSP_TRACE("Device is holding lines down, needs reset");
        ret = BSP_I2C_RECOVER_RESET_DEVICE;
    }
    return ret;

}

BSP_I2C_recover_action_t UAIR_BSP_I2C_analyse_and_recover_error(BSP_I2C_busnumber_t busno)
{
    BSP_I2C_recover_action_t ret = BSP_I2C_RECOVER_RETRY;
    struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);
    HAL_I2C_bus_t handle = UAIR_BSP_I2C_GetHALHandle(busno);

    // First, check if bus is idle.
    enum i2c_bus_idle_e busidle = UAIR_BSP_I2C_is_bus_idle(busdef);
    do {
        if (busidle<0) {
            // Ups, this should not happen
            BSP_TRACE("Cannot determine I2C bus state (busno %d)", busno);
            ret = BSP_I2C_RECOVER_FATAL_ERROR;
            break;
        }

        if (busidle!=I2C_BUS_IDLE) {
            BSP_TRACE("I2C busno %d lines stuck (%s)", busno,
                      busidle==I2C_BUS_STUCK_BOTH?"both":busidle==I2C_BUS_STUCK_SDA?"sda":"scl");
            // Lines are stuck.
            ret = UAIR_BSP_I2C_handle_stuck(busno);
            if (ret==BSP_I2C_RECOVER_RETRY) {
                break;
            }
        } else {
            // Bus looks normal. Check reported error in I2C bus
            int32_t state = handle->State;
            int32_t i2c_error = handle->ErrorCode;
            BSP_TRACE("I2C busno %d handler state=%d, errorcode 0x%08x", busno, state, i2c_error);
            ret = UAIR_BSP_I2C_handle_hal_error(busno, state, i2c_error);
            if (ret==BSP_I2C_RECOVER_RETRY) {
                break;
            }
        }
    } while (0);
    return ret;
}

