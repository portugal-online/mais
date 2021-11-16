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
 * @file VM3011.h
 *
 * @copyright Copyright (c) 2021 MAIS project
 *
 */

#ifndef __VM3011_H__
#define __VM3011_H__

#include "HAL.h"
#include <inttypes.h>

//#define VM3011_I2C_BUS_INSTANCE  NUCLEO_BSP_ext_sensor_i2c3
#define VM3011_DEFAULT_I2C_ADDRESS (0x61)
#define VM3011_DEFAULT_I2C_TIMEOUT 100


typedef struct {
    HAL_I2C_bus_t bus;
    uint8_t address;
    unsigned timeout;
} VM3011_t;


typedef enum
{
  VM3011_OP_SUCCESS = 0,
  VM3011_OP_FAIL_NOACK = 1,
  VM3011_OP_CONFIG_ERROR = 2,
  VM3011_OP_UNKNOWN_ERROR = 3
} VM3011_op_result_t;


#define VM3011_REG_I2C_CTRL (0x00)
#define VM3011_REG_WOS_PGA_GAIN (0x01)
#define VM3011_REG_WOS_FILTER (0x02)
#define VM3011_REG_WOS_PGA_MIN_THR (0x03)
#define VM3011_REG_WOS_PGA_MAX_THR (0x04)
#define VM3011_REG_WOS_THRESH (0x05)

#define VM3011_I2C_CTRL_WDT_ENABLE (1U<<0)
#define VM3011_I2C_CTRL_WDT_DLY_8ms (0U<<1)
#define VM3011_I2C_CTRL_WDT_DLY_16ms (1U<<1)
#define VM3011_I2C_CTRL_WDT_DLY_32ms (2U<<1)
#define VM3011_I2C_CTRL_WDT_DLY_64ms (3U<<1)
#define VM3011_I2C_CTRL_DOUT_CLEAR (1U<<4)

#define VM3011_WOS_FILTER_LPF_2KHZ (0U<<0)
#define VM3011_WOS_FILTER_LPF_4KHZ (1U<<0)
#define VM3011_WOS_FILTER_LPF_6KHZ (2U<<0)
#define VM3011_WOS_FILTER_LPF_8KHZ (3U<<0)
#define VM3011_WOS_FILTER_HPF_200HZ (0U<<2)
#define VM3011_WOS_FILTER_HPF_300HZ (1U<<2)
#define VM3011_WOS_FILTER_HPF_400HZ (2U<<2)
#define VM3011_WOS_FILTER_HPF_800HZ (3U<<2)


VM3011_op_result_t VM3011_Init(VM3011_t *vm, HAL_I2C_bus_t bus);
VM3011_op_result_t VM3011_Probe(VM3011_t *vm);
VM3011_op_result_t VM3011_Read_Threshold(VM3011_t *vm, uint8_t *threshold);

#endif
