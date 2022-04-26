/** Copyright (C) 2022 MAIS
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
 * @file uair_payloads.h
 *
 *
 */

#ifndef UAIR_PAYLOADS_H__
#define UAIR_PAYLOADS_H__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

struct generic_payload
{
    uint8_t rsvd:6;
    uint8_t payload_type:2;
} __attribute__((packed));

struct payload_type0
{
    uint8_t max_oaq_msb:1;
    uint8_t epa_oaq_msb:1;
    uint8_t health_oaq:1;
    uint8_t health_microphone:1;
    uint8_t health_ext_temp_hum:1;
    uint8_t health_int_temp_hum:1;    
    uint8_t payload_type:2;

    uint8_t avg_ext_temp;

    uint8_t max_sound_level_msb:1;
    uint8_t avg_ext_hum:7;

    uint8_t epa_oaq_lsb;

    uint8_t max_oaq_lsb;

    uint8_t avg_sound_level_lsb:4;
    uint8_t max_sound_level_lsb:4;

    uint8_t max_int_temp;

    uint8_t avg_sound_level_msb:1;
    uint8_t max_int_hum:7;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif
