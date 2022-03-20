/**
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
 * @file UAIR_BSP_error.c
 * @brief UAIR BSP error routines
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_ERROR
 */

#include "UAIR_BSP_error.h"
#include "UAIR_BSP.h"

BSP_error_detail_t bsp_error_lasterror;

void BSP_error_push(BSP_error_detail_t error)
{
    bsp_error_lasterror = error;
}

/**
 * @brief Return the last BSP error details
 * @ingroup UAIR_BSP_ERROR
 *
 *
 * Return detail information about last BSP error
 *
 * @return Details of the error in a \ref BSP_error_detail_t structure
 */
BSP_error_detail_t BSP_error_get_last_error(void)
{
    return bsp_error_lasterror;
}
