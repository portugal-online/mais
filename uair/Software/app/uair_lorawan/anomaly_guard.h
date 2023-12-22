
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
 * @file anomaly_guard.h
 *
 *
 */


#ifndef UAIR_ANOMALY_GUARD_H__
#define UAIR_ANOMALY_GUARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "io/UAIR_io_audit.h"

#define UAIR_POL_INT_TEMP_AS_EXT 1
#define UAIR_POL_INT_HUMD_AS_EXT 2
#define UAIR_POL_INT_TEMP_FOR_OAQ 3
#define UAIR_POL_INT_HUMD_FOR_OAQ 4
#define UAIR_POL_NO_TEMP 5
#define UAIR_POL_NO_HUMD 6
#define UAIR_POL_NO_SOUND 7
#define UAIR_POL_NO_OAQ 8

void UAIR_anomaly_process(uair_io_context_audit_errors audit, uint8_t *policy);

#ifdef __cplusplus
}
#endif

#endif
