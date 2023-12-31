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

/* Power zone state */
typedef enum {
    POWER_ON,
    POWER_OFF,
    POWER_INCONSISTENT
} powerstate_t;

typedef void (*powerzone_notify_callback_t)(void *userdata, const powerstate_t state);

/**
 * @brief Enable power to a specific powerzone
 * @ingroup UAIR_BSP_POWERZONE
 *
 *
 * If the powerzone has discharging circuitry, it will be disabled.
 *
 * \return BSP_ERROR_NO_INIT if the powerzone is not initialized or available.
 * \return BSP_ERROR_NONE if the powerzone was enabled successfuly
 * \return BSP_ERROR_BUS_FAILURE if the powerzone is faulty
 */
BSP_error_t BSP_powerzone_ref(BSP_powerzone_t powerzone);

/**
 * @brief Disable power to a specific powerzone
 * @ingroup UAIR_BSP_POWERZONE
 *
 *
 * If the powerzone has discharging circuitry, it will be enabled. Note that disabling a 
 * faulty powerzone does not yield an error. The powerzone will only be effectively
 * disabled if no other sensors are connected to the powerzone
 *
 * \return BSP_ERROR_NO_INIT if the powerzone is not initialized or available.
 * \return BSP_ERROR_NONE if the powerzone was disabled successfuly
 */
BSP_error_t BSP_powerzone_unref(BSP_powerzone_t powerzone);

/**
 * @brief Attach a callback to a powerzone
 * @ingroup UAIR_BSP_POWERZONE
 *
 *
 * Attach a callback to the powerzone. Whenever the power zone changes state,
 * the callback will be called with \p userdata as the user argument.
 *
 * If the power is removed, the callback is called prior to removing power.
 * If the power is applied, the callback is called after applying power.
 *
 * The parameter power to the \p callback callback will be either POWER_OFF or POWER_ON.
 *
 * \return BSP_ERROR_BUSY if a callback is already registered
 * \return BSP_ERROR_NONE if the callback was registered successfully
 */
BSP_error_t BSP_powerzone_attach_callback(BSP_powerzone_t powerzone, powerzone_notify_callback_t callback, void *userdata);

BSP_error_t BSP_powerzone_detach_callback(BSP_powerzone_t powerzone);

BSP_error_t UAIR_BSP_powerzone_cycle(BSP_powerzone_t powerzone);

#ifdef __cplusplus
}
#endif

#endif
