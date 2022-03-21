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
 * @file UAIR_BSP_watchdog.h
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_CORE
 *
 * uAir watchdog interface header
 *
 */
#ifndef UAIR_BSP_WATCHDOG_H__
#define UAIR_BSP_WATCHDOG_H__

#include "BSP.h"
#include "HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/* BSP error codes */
enum iwdg_error_e {
    BSP_ERROR_TYPE_IWDG_INIT,
    BSP_ERROR_TYPE_IWDG_KICK
};

void UAIR_BSP_watchdog_kick(void);

#ifdef __cplusplus
}
#endif

#endif
