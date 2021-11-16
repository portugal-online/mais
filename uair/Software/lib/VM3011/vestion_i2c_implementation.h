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
 * @file vestion_i2c_implementation.c
 *
 * @copyright Copyright (c) 2021 MAIS project
 *
 */

#include "VM3011.h"
#include "BSP.h"
#include "UAIR_tracer.h"

static inline int vm3011_write_register(VM3011_t *vm, uint8_t reg, uint8_t val)
{
    int r =HAL_I2C_Mem_Write(vm->bus,
                             (vm->address<<1),
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &val, 1,
                             vm->timeout);
    if (r!=0) {
        APP_PRINTF("%s: error %d (%d)\r\n", __FUNCTION__, r, vm->bus->ErrorCode);
    }
    return r;
}

static int32_t vm3011_read_register(VM3011_t * vm,uint8_t reg, uint8_t *dest)
{
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Read(vm->bus,
                              (vm->address<<1),
                              reg,
                              I2C_MEMADD_SIZE_8BIT, dest, 1,vm->timeout);
    if (status!=0) {
        APP_PRINTF("%s: error %d (%d)\r\n", __FUNCTION__, status, vm->bus->ErrorCode);
    }

    return status;
}

