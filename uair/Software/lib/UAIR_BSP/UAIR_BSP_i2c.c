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
#include <stdbool.h>

static void i2c1_reset(bool is_reset);
static void i2c2_reset(bool is_reset);
static void i2c3_reset(bool is_reset);

static void i2c1_lpm(bool is_enter_lpm);
static void i2c2_lpm(bool is_enter_lpm);

static const struct i2c_bus_def i2c3 = {
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
    .i2c_reset = i2c3_reset,
    .lpm_control = NULL /* Nothing needed for LPM */
};

static const struct i2c_bus_def i2c1 = {
    .sda = {
        .port = GPIOA,
        .pin = GPIO_PIN_10,
        .af = GPIO_AF4_I2C1,
        .clock_control = HAL_clk_GPIOA_clock_control
    },
    .scl = {
        .port = GPIOB,
        .pin = GPIO_PIN_8,
        .af = GPIO_AF4_I2C1,
        .clock_control = HAL_clk_GPIOB_clock_control
    },
    .i2c_bus = I2C1,
    .i2c_clock_control = HAL_clk_I2C1_clock_control,
    .i2c_reset = i2c1_reset,
    .lpm_control = i2c1_lpm
};

static const struct i2c_bus_def i2c2 = {
    .sda = {
        .port = GPIOA,
        .pin = GPIO_PIN_12,
        .af = GPIO_AF4_I2C2,
        .clock_control = HAL_clk_GPIOA_clock_control
    },
    .scl = {
        .port = GPIOA,
        .pin = GPIO_PIN_11,
        .af = GPIO_AF4_I2C2,
        .clock_control = HAL_clk_GPIOA_clock_control
    },
    .i2c_bus = I2C2,
    .i2c_clock_control = HAL_clk_I2C2_clock_control,
    .i2c_reset = i2c2_reset,
    .lpm_control = i2c2_lpm
};



static const struct i2c_bus_def *i2c_bus_r1[] = {
    &i2c3,
    NULL,
    NULL,
};

static const struct i2c_bus_def *i2c_bus_r2[] = {
    &i2c1,
    &i2c2,
    &i2c3
};

static uint8_t i2c_bus_initialised = 0;

// Bus instances

static I2C_HandleTypeDef i2c_buses[1+BSP_I2C_MAX_BUS];

static BSP_error_t UAIR_BSP_I2C_Bus_Init_Internal(HAL_I2C_bus_t bus_instance, const struct i2c_bus_def *bus_def);

BSP_error_t UAIR_BSP_I2C_InitAll()
{
    BSP_error_t err;
    unsigned i;
    do {
        for (i=0; i<=BSP_I2C_MAX_BUS;i++) {
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

    const struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);
    if (busdef==NULL) {
        err = BSP_ERROR_WRONG_PARAM;
    } else {
        err = UAIR_BSP_I2C_Bus_Init_Internal(&i2c_buses[busno], busdef);
        if (err==BSP_ERROR_NONE) {
            i2c_bus_initialised |= (1<<busno);
        }
    }
    return err;
}

static BSP_error_t UAIR_BSP_I2C_low_power_mode(bool is_enter_lpm)
{
    int i;
    for (i=0; i<=BSP_I2C_MAX_BUS;i++) {
        const struct i2c_bus_def *bus_def = UAIR_BSP_I2C_GetBusDef(i);
        if (bus_def->lpm_control)
            bus_def->lpm_control(is_enter_lpm);
    }
    return BSP_ERROR_NONE;
}

BSP_error_t UAIR_BSP_I2C_enter_low_power_mode(void)
{
    return UAIR_BSP_I2C_low_power_mode(true);
}

BSP_error_t UAIR_BSP_I2C_exit_low_power_mode(void)
{
    return UAIR_BSP_I2C_low_power_mode(false);
}

static BSP_error_t UAIR_BSP_I2C_Bus_Init_Internal(HAL_I2C_bus_t bus_instance, const struct i2c_bus_def *bus_def)
{
    bus_instance->Instance = bus_def->i2c_bus;
    if (UAIR_HAL_is_lowpower()) {
        bus_instance->Init.Timing = I2C_SPEED_SYSCLK2_100KHZ;
    } else {
        bus_instance->Init.Timing = I2C_SPEED_SYSCLK24_100KHZ;
    }
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
    for (i=0;i<=BSP_I2C_MAX_BUS;i++) {
        const struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(i);
        if (busdef != NULL) {
            err = UAIR_BSP_I2C_Bus_Resume(busdef);
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

const struct i2c_bus_def *UAIR_BSP_I2C_GetBusDef(BSP_I2C_busnumber_t busno)
{
    const struct i2c_bus_def **buses = NULL;
    switch (BSP_get_board_version()) {
    case UAIR_NUCLEO_REV1:
        buses = &i2c_bus_r1[0];
        break;
    case UAIR_NUCLEO_REV2:
        buses = &i2c_bus_r2[0];
        break;
    default:
        buses = NULL;
        break;
    }
    if (!buses)
        return NULL;
    return buses[(int)busno];
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

static enum i2c_bus_idle_e UAIR_BSP_I2C_is_bus_idle(const struct i2c_bus_def *busdef)
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
    const struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);

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
    const struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);
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

        int32_t state = handle->State;
        int32_t i2c_error = handle->ErrorCode;
        BSP_TRACE("I2C busno %d handler state=%d, errorcode 0x%08x", busno, state, i2c_error);

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
            ret = UAIR_BSP_I2C_handle_hal_error(busno, state, i2c_error);
            if (ret==BSP_I2C_RECOVER_RETRY) {
                break;
            }
        }
    } while (0);
    return ret;
}

static void i2c1_reset(bool is_reset)
{
    if (is_reset)
        __HAL_RCC_I2C1_FORCE_RESET();
    else
        __HAL_RCC_I2C1_RELEASE_RESET();
}

static void i2c2_reset(bool is_reset)
{
    if (is_reset)
        __HAL_RCC_I2C2_FORCE_RESET();
    else
        __HAL_RCC_I2C2_RELEASE_RESET();
}

static void i2c3_reset(bool is_reset)
{
    if (is_reset)
        __HAL_RCC_I2C3_FORCE_RESET();
    else
        __HAL_RCC_I2C3_RELEASE_RESET();
}

struct saved_i2c_context
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t OAR1;
    uint32_t OAR2;
    uint32_t TIMINGR;
    uint32_t TIMEOUTR;
};

static struct saved_i2c_context i2c1_saved_context;
static struct saved_i2c_context i2c2_saved_context;

static void UAIR_BSP_I2C_save_i2c_context(I2C_TypeDef *handle, struct saved_i2c_context *ctx)
{
    ctx->CR1 = handle->CR1;
    ctx->CR2 = handle->CR2;
    ctx->OAR1 = handle->OAR1;
    ctx->OAR2 = handle->OAR2;
    ctx->TIMINGR = handle->TIMINGR;
    ctx->TIMEOUTR = handle->TIMEOUTR;
}

static void UAIR_BSP_I2C_restore_i2c_context(const struct saved_i2c_context *ctx, I2C_TypeDef *handle)
{
    handle->TIMEOUTR = ctx->TIMEOUTR;
    handle->TIMINGR = ctx->TIMINGR;
    handle->OAR2 = ctx->OAR2;
    handle->OAR1 = ctx->OAR1;
    handle->CR2 = ctx->CR2;
    handle->CR1 = ctx->CR1;
}

static void i2c1_lpm(bool is_enter_lpm)
{
    if (is_enter_lpm) {
        UAIR_BSP_I2C_save_i2c_context(I2C1, &i2c1_saved_context);
    } else {
        UAIR_BSP_I2C_restore_i2c_context(&i2c1_saved_context, I2C1);
    }
}

static void i2c2_lpm(bool is_enter_lpm)
{
    if (is_enter_lpm) {
        UAIR_BSP_I2C_save_i2c_context(I2C2, &i2c2_saved_context);
    } else {
        UAIR_BSP_I2C_restore_i2c_context(&i2c2_saved_context, I2C2);
    }
}

BSP_error_t UAIR_BSP_I2C_set_discharge(BSP_I2C_busnumber_t busno, bool enable_discharge)
{
    BSP_error_t err = BSP_ERROR_NONE;

    const struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);

    if (busdef!=NULL) {
        // TBD: check if we need to temporarly disable I2C bus. use i2c_bus_initialised to confirm
        if (enable_discharge) {
            HAL_GPIO_configure_output_od(&busdef->sda);
            HAL_GPIO_configure_output_od(&busdef->scl);
            HAL_GPIO_write(&busdef->sda, 0);
            HAL_GPIO_write(&busdef->scl, 0);
        } else {
            HAL_GPIO_write(&busdef->sda, 1);
            HAL_GPIO_write(&busdef->scl, 1);
            HAL_GPIO_configure_af_od(&busdef->sda);
            HAL_GPIO_configure_af_od(&busdef->scl);
        }
        // Wait for bus to drain.

    } else {
        BSP_TRACE("Invalid bus specified (%d)");
        err = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    return err;

}

BSP_error_t UAIR_BSP_I2C_read_sda_scl(BSP_I2C_busnumber_t busno, int *sda, int *scl)
{
    BSP_error_t ret;

    const struct i2c_bus_def *busdef = UAIR_BSP_I2C_GetBusDef(busno);

    if (busdef==NULL) {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
    } else {
        *sda =  HAL_GPIO_read(&busdef->sda);
        *scl = HAL_GPIO_read(&busdef->scl);
        BSP_TRACE("Bus %d: sda=%d scl=%d", busno, *sda, *scl);
        ret = BSP_ERROR_NONE;
    }
    return ret;
}
