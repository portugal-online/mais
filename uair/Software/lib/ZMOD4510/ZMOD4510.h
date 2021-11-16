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
 * @file ZMOD4510.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef __ZMOD4510_H__
#define __ZMOD4510_H__

#include "HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  ZMOD4510_OP_SUCCESS = 0,
  ZMOD4510_OP_FAIL_NOACK = 1,
  ZMOD4510_OP_DEVICE_ERROR = 2,
  ZMOD4510_OP_UNKNOWN_ERROR = 3
} ZMOD4510_op_result_t;


struct ZMOD4510;
typedef struct ZMOD4510 ZMOD4510_t;

ZMOD4510_op_result_t ZMOD4510_Init(ZMOD4510_t *zmod, HAL_I2C_bus_t bus, HAL_GPIO_t reset_gpio);
ZMOD4510_op_result_t ZMOD4510_Probe(ZMOD4510_t *zmod);


#ifdef __cplusplus
}
#endif

#endif
