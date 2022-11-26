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
 * @file UAIR_BSP_i2c_p.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_I2C_P_H__
#define UAIR_BSP_I2C_P_H__

#include "UAIR_BSP_error.h"
#include "UAIR_BSP_i2c.h"
#include "HAL.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct i2c_bus_def {
    HAL_GPIODef_t sda;
    HAL_GPIODef_t scl;
    I2C_TypeDef *i2c_bus; // Hardware I2C
    HAL_clk_clock_control_fun_t i2c_clock_control;
    void (*i2c_reset)(bool is_reset);
    void (*lpm_control)(bool is_enter_lpm);
};

BSP_error_t UAIR_BSP_I2C_InitAll(void);
BSP_error_t UAIR_BSP_I2C_InitBus(BSP_I2C_busnumber_t);
BSP_error_t UAIR_BSP_I2C_Bus_Resume(const struct i2c_bus_def *bus_def);
BSP_error_t UAIR_BSP_I2C_Bus_ResumeAll(void);
BSP_error_t UAIR_BSP_I2C_Bus_DeInit(HAL_I2C_bus_t bus);
void UAIR_BSP_I2C_fault_detected(BSP_I2C_recover_action_t action);

const struct i2c_bus_def *UAIR_BSP_I2C_GetBusDef(BSP_I2C_busnumber_t busno);
HAL_I2C_bus_t UAIR_BSP_I2C_GetHALHandle(BSP_I2C_busnumber_t busno);
BSP_error_t UAIR_BSP_I2C_enter_low_power_mode(void);
BSP_error_t UAIR_BSP_I2C_exit_low_power_mode(void);
BSP_error_t UAIR_BSP_I2C_set_discharge(BSP_I2C_busnumber_t busno, bool enable_discharge);
BSP_error_t UAIR_BSP_I2C_read_sda_scl(BSP_I2C_busnumber_t busno, int *sda, int *scl);

BSP_I2C_recover_action_t UAIR_BSP_I2C_analyse_and_recover_error(BSP_I2C_busnumber_t busno);
BSP_error_t UAIR_BSP_I2C_manual_bus_release(BSP_I2C_busnumber_t busno);

#ifdef __cplusplus
}
#endif

#endif
