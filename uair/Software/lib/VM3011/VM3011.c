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
 * @file VM3011.c
 *
 * @copyright Copyright (c) 2021 MAIS project
 *
 */
#include "VM3011.h"
#include "vestion_i2c_implementation.h"

static VM3011_op_result_t VM3011_Reset_DOUT(VM3011_t *vm)
{
    VM3011_op_result_t r = vm3011_write_register_readout_mask(vm,
                                                              VM3011_REG_I2C_CTRL,
                                                              VM3011_I2C_CTRL_WDT_ENABLE |
                                                              VM3011_I2C_CTRL_WDT_DLY_8ms |
                                                              VM3011_I2C_CTRL_DOUT_CLEAR,

                                                              VM3011_I2C_CTRL_WDT_ENABLE_MASK | VM3011_I2C_CTRL_WDT_DLY_MASK
                                                             );
    return r;
}

VM3011_op_result_t VM3011_Init(VM3011_t *vm, HAL_I2C_bus_t bus)
{
    vm->bus = bus;
    vm->address = VM3011_DEFAULT_I2C_ADDRESS;
    vm->timeout = VM3011_DEFAULT_I2C_TIMEOUT;
    return VM3011_OP_SUCCESS;
}


VM3011_op_result_t VM3011_Probe(VM3011_t *vm)
{
    VM3011_op_result_t r;

    r = VM3011_Reset_DOUT(vm);

    if (r != VM3011_OP_SUCCESS)
        return r;

    r = vm3011_write_register_readout_mask(vm,
                                           VM3011_REG_WOS_FILTER,
                                           VM3011_WOS_FILTER_LPF_2KHZ | VM3011_WOS_FILTER_HPF_800HZ,
                                           VM3011_WOS_FILTER_LPF_MASK | VM3011_WOS_FILTER_HPF_MASK
                                          );

    if (r != VM3011_OP_SUCCESS)
        return r;

    r = vm3011_write_register_readout_mask(vm, VM3011_REG_WOS_PGA_MIN_THR, 0x00, 0x1F);

    if (r != VM3011_OP_SUCCESS)
        return r;

    r = vm3011_write_register_readout_mask(vm, VM3011_REG_WOS_PGA_MAX_THR, 0x1F, 0x1F);

    if (r != VM3011_OP_SUCCESS)
        return r;

    r = vm3011_write_register_readout_mask(vm, VM3011_REG_WOS_THRESH, 0x07, 0x07);

    return r;
}

VM3011_op_result_t VM3011_Read_Threshold(VM3011_t *vm, uint8_t *threshold)
{
    return vm3011_read_register(vm, VM3011_REG_WOS_PGA_GAIN, threshold);
}
