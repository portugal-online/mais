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
 * @file UAIR_bsp_error.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 */

#ifndef UAIR_BSP_ERROR_H
#define UAIR_BSP_ERROR_H

#ifdef __cplusplus
 extern "C" {
#endif

typedef int BSP_error_t;

/* Common Error codes */
#define BSP_ERROR_NONE                         0
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

#ifdef __cplusplus
}
#endif

#endif /* BSP_ERROR_H */
