/*
 *  Copyright (C) 2021 MAIS Project
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
 * @defgroup UAIR_BSP_POWERZONE uAir BSP Power Zones
 * @ingroup UAIR_BSP
 * @brief uAir Power Zones
 *
 * The uAir hardware operates using powerzones. A powerzone is a hardware partition
 * usually made up of sensors, that can be selectively and individually powered.
 *
 * There are currently 3 powerzones:
 *
 * - \ref UAIR_POWERZONE_INTERNALI2C controls power to the internal temperature and humidity sensor.
 * - \ref UAIR_POWERZONE_AMBIENTSENS controls power to the ambient sensor and the external temperature and humidity sensor.
 * - \ref UAIR_POWERZONE_MICROPHONE controls power to microphone.
 *
 * Powerzone BIT checks verify if the actual power switch works by attempting to measure the power
 * after changing the power setting. Currently the BIT checks are only internal to the BSP and not directly controllable
 * by the application.
 */

/**
 * @file UAIR_BSP_powerzone.h
 * 
 * @copyright Copyright (C) 2021 MAIS Project
 *
 * @ingroup UAIR_BSP_POWERZONE
 *
 */

#ifndef UAIR_BSP_POWEZONE_H__
#define UAIR_BSP_POWEZONE_H__

/* Power control */

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup UAIR_BSP_POWERZONE
 Powerzone specific BSP error. Used to populate the \ref BSP_error_detail_t type field.
 */
enum powerzone_error_e {
    BSP_ERROR_TYPE_POWERZONE_ZONE_STILL_POWERED, /*!< Powerzone still powered after removing power */
    BSP_ERROR_TYPE_POWERZONE_ZONE_NO_POWER       /*!< Powerzone not powered after enabling power */
};

/** @ingroup UAIR_BSP_POWERZONE
 Powerzones definition */
typedef enum {
    UAIR_POWERZONE_INTERNALI2C = 0,   /*!< Internal I2C bus devices */
    UAIR_POWERZONE_MICROPHONE,        /*!< Microphone */
    UAIR_POWERZONE_AMBIENTSENS,       /*!< Ambient and external temp./hum sensor */
    UAIR_POWERZONE_MAX = UAIR_POWERZONE_AMBIENTSENS,
    UAIR_POWERZONE_NONE = -1
} BSP_powerzone_t;

/**
 * @brief Enable power to a specific powerzone
 * @ingroup UAIR_BSP_POWERZONE
 *
 *
 * If the powerzone has discharging circuitry, it will be disabled.
 *
 * \return BSP_ERROR_NO_INIT if the powerzone is not initialized or available.
 * \return BSP_ERROR_NONE if the powerzone was enabled successfuly
 */
BSP_error_t BSP_powerzone_enable(BSP_powerzone_t powerzone);

/**
 * @brief Disable power to a specific powerzone
 * @ingroup UAIR_BSP_POWERZONE
 *
 *
 * If the powerzone has discharging circuitry, it will be enabled.
 *
 * \return BSP_ERROR_NO_INIT if the powerzone is not initialized or available.
 * \return BSP_ERROR_NONE if the powerzone was disabled successfuly
 */
BSP_error_t BSP_powerzone_disable(BSP_powerzone_t powerzone);

#ifdef __cplusplus
}
#endif

#endif
