/** Copyright © 2021 The Things Industries B.V.
 *  Copyright © 2021 MAIS Project
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
 * @file UAIR_bsp.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_H__
#define UAIR_BSP_H__

#include "UAIR_BSP_conf.h"
#include "stm32wlxx_hal.h"
#include "UAIR_BSP_radio.h"
#include "UAIR_BSP_gpio.h"
#include "UAIR_BSP_serial.h"
#include "UAIR_BSP_clk_timer.h"
#include "UAIR_BSP_microphone.h"
#include "UAIR_BSP_bm.h"
#include "UAIR_BSP_lpm.h"
#include "UAIR_BSP_internaltemp.h"
#include "UAIR_BSP_externaltemp.h"
#include "UAIR_BSP_powerzone.h"
#include "UAIR_BSP_air_quality.h"
#include "UAIR_BSP_error.h"
#include "UAIR_BSP_types.h"
#include "UAIR_tracer.h"
#include "UAIR_lpm.h"

#ifdef __cplusplus
extern "C" {
#endif

void BSP_get_default_config(BSP_config_t *dest);
BSP_error_t BSP_init(const BSP_config_t *config);
BSP_board_version_t BSP_get_board_version(void);

/* Microphone */
BSP_error_t BSP_microphone_read_gain(uint8_t *gain);
void  __attribute__((noreturn)) BSP_FATAL(void);

#define BSP_TRACE(x...) do { \
    APP_PRINTF("BSP: %s:%d : ", __FUNCTION__, __LINE__); \
    APP_PRINTF(x); \
    APP_PRINTF("\r\n"); \
    } while (0)

#define BSP_STOP_FOR_POWER_CALCULATION(x...) \
    do { \
    UAIR_LPM_EnterLowPower(); \
    } while (1)


#ifdef __cplusplus
}
#endif

#endif /* UAIR_BSP_H */
