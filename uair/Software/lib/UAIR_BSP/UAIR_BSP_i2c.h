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
 * @file UAIR_BSP_i2c.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_I2C_H__
#define UAIR_BSP_I2C_H__

#include "UAIR_BSP_error.h"
#include "HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_I2C_BUS0,
    BSP_I2C_BUS1,
    BSP_I2C_BUS2,
    BSP_I2C_MAX_BUS = BSP_I2C_BUS2,
    BSP_I2C_BUS_NONE=-1
} BSP_I2C_busnumber_t;


#ifdef __cplusplus
}
#endif

#endif
