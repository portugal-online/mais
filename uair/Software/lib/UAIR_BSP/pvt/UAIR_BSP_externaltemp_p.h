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
 * @file NUCLEO_BSP_internaltemp_p.h
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_EXTERNALTEMP_P_H__
#define UAIR_BSP_EXTERNALTEMP_P_H__


#ifdef __cplusplus
extern "C" {
#endif

void UAIR_BSP_external_temp_set_defaults(BSP_temp_accuracy_t temp_acc, BSP_hum_accuracy_t hum_acc);
BSP_error_t UAIR_BSP_external_temp_hum_init(void);
void UAIR_BSP_external_temp_hum_deinit(void);
void UAIR_BSP_external_temp_hum_set_faulty(void);


#ifdef __cplusplus
}
#endif

#endif
