/*
 * Copyright (C) 2021, 2022 MAIS Project
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
 * @defgroup UAIR_BSP_SENSOR_MICROPHONE uAir Microphone Sensor interface
 * @ingroup UAIR_BSP_SENSORS
 *
 * uAir interfacing to microphone
 *
 *
 * The microphone is currently operating in what the manufacturer calls ZPL (Zero-Power Listening).
 * The microphone data (audio) is not captured, instead the microphone is placed in an automatic gain control
 * loop, where the internal gain increases when the sound level is weak, and decreases when the sound level is
 * stronger.
 *
 * By reading the current gain, we can infer in a very low-power mode whether the ambient noise is low or high.
 *
 * A gain of 31 (maximum) is reported when the audio pressure (level) is low, i.e., quiet.
 * A gain of 0 (minimum) is reported when the audio pressure (level) is high, i.e., loud.
 *
 * Usage of the sensor should be as follows:
 *
 * - Call \ref BSP_microphone_read_gain() to read current microphone gain.
 *
 */

/**
 * @file UAIR_BSP_microphone.h
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_SENSOR_MICROPHONE
 *
 * uAir interfacing to microphone header
 *
 */

#ifndef UAIR_BSP_MICROPHONE_H__
#define UAIR_BSP_MICROPHONE_H__

#include "UAIR_BSP_types.h"
#include "UAIR_BSP_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MICROPHONE_MAX_GAIN (31U)

BSP_error_t BSP_microphone_read_gain(uint8_t *gain);
BSP_sensor_state_t BSP_microphone_get_sensor_state(void);

#ifdef __cplusplus
}
#endif

#endif
