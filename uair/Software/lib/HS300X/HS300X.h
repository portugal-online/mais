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
 * @file HS300X.h
 *
 * @copyright Copyright (c) 2021 MAIS project
 *
 */

#ifndef HS300X_H__
#define HS300X_H__

#include "HAL.h"
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HS300X_ACCURACY_8BIT = 0,
    HS300X_ACCURACY_10BIT = 1,
    HS300X_ACCURACY_12BIT = 2,
    HS300X_ACCURACY_14BIT = 3,
    HS300X_ACCURACY_NONE = -1
} HS300X_accuracy_t;

struct HS300X {
    HAL_I2C_bus_t bus;
    uint8_t address;
    unsigned i2c_timeout;
    uint32_t serial;
#ifndef HS300X_NO_CHECK_TIMING
    HS300X_accuracy_t temp_acc;
    HS300X_accuracy_t hum_acc;
    uint64_t meas_start;
#endif
};

typedef struct HS300X HS300X_t;

int HS300X_init(HS300X_t *hs, HAL_I2C_bus_t bus);

HAL_StatusTypeDef HS300X_probe(HS300X_t *hs,
                               HS300X_accuracy_t hum_accuracy,
                               HS300X_accuracy_t temp_accuracy);


HAL_StatusTypeDef HS300X_start_measurement(HS300X_t *hs);
HAL_StatusTypeDef HS300X_read_measurement(HS300X_t *hs, int32_t *temp_millicentigrade, int32_t *hum_millipercent, int *stale);
uint32_t HS300X_get_probed_serial(HS300X_t *hs);
unsigned HS300X_time_for_measurement_us(const HS300X_accuracy_t temp_acc, const HS300X_accuracy_t hum_acc);

/* Adv. API */
HAL_StatusTypeDef HS300X_read_register(HS300X_t *hs, uint8_t reg, uint16_t *value);

#ifdef __cplusplus
}
#endif

#endif
