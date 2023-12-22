
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
 * @file anomaly_guard.c
 *
 *
 */

#include "io/UAIR_io_audit.h"

void UAIR_anomaly_process(uair_io_context_audit_errors audit, uint8_t *policy) {
    // To Be Continued. policy needs to be checked by caller
    if (audit >= 1 && audit <= 10) if (!((*policy >> 1) & 1)) *policy |= 1 << 1;
    if (audit >= 11 && audit <= 20) if (!((*policy >> 2) & 1)) *policy |= 1 << 2;
}