/*
 * Copyright © 2021 The Things Industries B.V.
 * Copyright © 2021 MAIS Project
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
 * @defgroup UAIR_BSP_ERROR BSP error handling
 * @ingroup UAIR_BSP_CORE
 */

/**
 * @file UAIR_bsp_error.h
 * @ingroup UAIR_BSP_ERROR
 * @brief BSP Error definitions
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_ERROR_H__
#define UAIR_BSP_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef int BSP_error_t;

#define BSP_ERROR_NONE                         0     /* No error */
#define BSP_ERROR_NO_INIT                     -1
#define BSP_ERROR_WRONG_PARAM                 -2
#define BSP_ERROR_BUSY                        -3
#define BSP_ERROR_PERIPH_FAILURE              -4
#define BSP_ERROR_COMPONENT_FAILURE           -5
#define BSP_ERROR_UNKNOWN_FAILURE             -6
#define BSP_ERROR_UNKNOWN_COMPONENT           -7
#define BSP_ERROR_BUS_FAILURE                 -8
#define BSP_ERROR_CLOCK_FAILURE               -9
#define BSP_ERROR_MSP_FAILURE                 -10
#define BSP_ERROR_FEATURE_NOT_SUPPORTED       -11
#define BSP_ERROR_CALIBRATING                 -12

/**
 * @ingroup UAIR_BSP_ERROR
 * @brief Error zones
 */
typedef enum {
    ERROR_ZONE_POWERZONE,          /*!< Powerzone error. See \ref UAIR_BSP_POWERZONE */
    ERROR_ZONE_INTERNALTEMP,       /*!< Internal temperature/humidity sensor error */
    ERROR_ZONE_EXTERNALTEMP,       /*!< External temperature/humidity sensor error */
    ERROR_ZONE_AMBIENTSENSOR,      /*!< Ambient sensor error */
    ERROR_ZONE_MICROPHONE,         /*!< Microphone error */
    ERROR_ZONE_FLASH,              /*!< Internal FLASH error */
    ERROR_ZONE_IWDG,               /*!< Independent Watchdog error */
    ERROR_ZONE_BSP,                /*!< Generic BSP error */
} BSP_error_zone_t;


/**
 * @ingroup UAIR_BSP_ERROR
 * @brief Error details
 */
typedef struct {
    BSP_error_zone_t zone:8;       /*!< Zone where error ocurred */
    uint8_t type;                  /*!< Error type. */
    uint8_t index;                 /*!< Index that caused the error. */
    uint8_t value;                 /*!< Error value. */
} BSP_error_detail_t;

static inline void BSP_error_set(BSP_error_zone_t zone, uint8_t type, uint8_t index, uint8_t value);

void BSP_error_push(BSP_error_detail_t error);
BSP_error_detail_t BSP_error_get_last_error(void);

static inline void BSP_error_set(BSP_error_zone_t zone, uint8_t type, uint8_t index, uint8_t value)
{
    BSP_error_detail_t detail;
    detail.zone = zone;
    detail.type = type;
    detail.index = index;
    detail.value = value;
    BSP_error_push(detail);
}

#ifdef UNITTESTS
void BSP_error_reset(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BSP_ERROR_H */
