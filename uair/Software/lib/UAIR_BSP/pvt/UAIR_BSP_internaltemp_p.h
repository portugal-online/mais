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
 * @file UAIR_BSP_internaltemp_p.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_INTERNALTEMP_P_H__
#define UAIR_BSP_INTERNALTEMP_P_H__

#include "UAIR_BSP_error.h"

#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t UAIR_BSP_internal_temp_hum_init(void);
void UAIR_BSP_internal_temp_hum_powerzone_changed(void *userdata, const powerstate_t state);

#ifdef __cplusplus
}
#endif

#endif
