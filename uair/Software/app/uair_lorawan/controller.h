/** Copyright © 2022 MAIS
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
 * @file controller.h
 *
 *
 */

#ifndef UAIR_CONTROLLER_H__
#define UAIR_CONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "io/UAIR_io_config.h"

void UAIR_controller_start(void);

uint8_t UAIR_policy_set(uair_io_context_keys id, uint8_t value);


#ifdef __cplusplus
}
#endif

#endif /*__LORA_APP_H__*/
