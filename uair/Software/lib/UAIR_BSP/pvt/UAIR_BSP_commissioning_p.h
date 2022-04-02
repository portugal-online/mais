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
 * @file UAIR_BSP_commissioning_p.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_COMMISSIONING_P_H__
#define UAIR_BSP_COMMISSIONING_P_H__

#include <inttypes.h>
#include "UAIR_BSP_error.h"

#if defined(HOSTMODE)

#define COMMISSIONING_STORAGE_SECTION /* */

#else // HOSTMODE

#define COMMISSIONING_STORAGE_SECTION __attribute__((section (".commissioning")))

#endif // HOSTMODE


BSP_error_t UAIR_BSP_commissioning_init(void);
BSP_error_t UAIR_BSP_commissioning_get_device_eui(uint8_t target[8]);

#endif

