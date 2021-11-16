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
 * @file UAIR_bsp_internaltemp.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#include "BSP.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned int BSP_internal_temp_hum_get_measure_delay_us(void);
BSP_error_t BSP_internal_temp_hum_start_measure(void);
/* Units

 */
BSP_error_t BSP_internal_temp_hum_read_measure(int32_t *temp, int32_t *hum);

#ifdef __cplusplus
}

#endif
