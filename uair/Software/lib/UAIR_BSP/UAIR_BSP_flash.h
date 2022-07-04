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
 * @defgroup UAIR_BSP_FLASH uAir internal FLASH interface
 * @ingroup UAIR_BSP
 *
 * uAir internal FLASH interface
 *
 *
 */

/**
 * @file UAIR_BSP_flash.h
 * 
 * @copyright Copyright (C) 2021, 2022 MAIS Project
 *
 * @ingroup UAIR_BSP_FLASH
 *
 * uAir FLASH interface header
 *
 */

#ifndef UAIR_BSP_FLASH_H__
#define UAIR_BSP_FLASH_H__

#include "UAIR_BSP_error.h"
#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_FLASH_PAGE_SIZE_BITS (11) /* 2^11 == 2048 ª*/
#define BSP_FLASH_PAGE_SIZE (1U<<BSP_FLASH_PAGE_SIZE_BITS) /* 2048 */

/* BSP error codes */
enum flash_error_e {
    BSP_ERROR_TYPE_FLASH_UNLOCK,
    BSP_ERROR_TYPE_FLASH_PROGRAM,
    BSP_ERROR_TYPE_FLASH_LOCK,
    BSP_ERROR_TYPE_FLASH_ERASE
};


/* A flash page. Max 256 pages */
typedef uint8_t flash_page_t;
/* A flash address, relative to area start. */
typedef uint32_t flash_address_t;

unsigned UAIR_BSP_flash_config_area_get_page_count(void);
BSP_error_t UAIR_BSP_flash_config_area_erase_page(flash_page_t page);
int UAIR_BSP_flash_config_area_read(flash_address_t address, uint8_t *dest, size_t len_bytes);
int UAIR_BSP_flash_config_area_write(flash_address_t address, const uint64_t *data, size_t count_doublewords);

/* Audit */
unsigned UAIR_BSP_flash_audit_area_get_page_count(void);
BSP_error_t UAIR_BSP_flash_audit_area_erase_page(flash_page_t page);
int UAIR_BSP_flash_audit_area_read(flash_address_t address, uint8_t *dest, size_t len_bytes);
int UAIR_BSP_flash_audit_area_write(flash_address_t address, const uint64_t *data, size_t count_doublewords);

#ifdef __cplusplus
}
#endif

#endif
