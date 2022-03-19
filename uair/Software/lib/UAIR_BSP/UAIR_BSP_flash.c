/**
 *  Copyright (c) 2021 MAIS Project
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
 * @file UAIR_BSP_flash.c
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP_flash.h"
#include "pvt/UAIR_BSP_flash_p.h"
#include <string.h>
#include <stm32wlxx_hal_flash.h>
#include <stm32wlxx_hal_flash_ex.h>

unsigned UAIR_BSP_flash_config_area_get_page_count(void)
{
    return BSP_FLASH_CONFIG_NUM_PAGES;
}

static uint8_t UAIR_BSP_flash_get_config_start_page(void)
{
    return UAIR_BSP_flash_storage_get_config_start_page();
}

BSP_error_t UAIR_BSP_flash_config_area_erase_page(flash_page_t page)
{
    FLASH_EraseInitTypeDef erasedef;
    HAL_StatusTypeDef err;
    uint32_t error_page;
    BSP_error_t ret;

    // Page is relative to start of config.
    if (page>=BSP_FLASH_CONFIG_NUM_PAGES)
        return BSP_ERROR_WRONG_PARAM;

    uint8_t actual_page = page + UAIR_BSP_flash_get_config_start_page();

    // Erase page

    erasedef.TypeErase = FLASH_TYPEERASE_PAGES;
    erasedef.Page = actual_page;
    erasedef.NbPages = 1;

    do {
        err = HAL_FLASH_Unlock();
        if (err!=HAL_OK) {
            BSP_error_set(ERROR_ZONE_FLASH, BSP_ERROR_TYPE_FLASH_UNLOCK, err, 0);
            ret = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        err = HAL_FLASHEx_Erase(&erasedef, &error_page);
        if (err!=HAL_OK) {
            (void)HAL_FLASH_Lock(); // Dunno what to do if we fail here
            BSP_error_set(ERROR_ZONE_FLASH, BSP_ERROR_TYPE_FLASH_ERASE, err, error_page);
            ret = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        err = HAL_FLASH_Lock();
        if (err!=HAL_OK) {
            BSP_error_set(ERROR_ZONE_FLASH, BSP_ERROR_TYPE_FLASH_LOCK, err, 0);
            ret = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        ret = BSP_ERROR_NONE;
    } while (0);
    return ret;
}

/**
 * Read from a address. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page). Note that reads do not wrap around, a short read will
 * occur if reading beyond the allocated area.
 * @return number of bytes read or a (BSP_error_t) error value.
 */
int UAIR_BSP_flash_config_area_read(flash_address_t address, uint8_t *dest, size_t len_bytes)
{
    flash_address_t last_address = BSP_FLASH_CONFIG_NUM_PAGES << BSP_FLASH_PAGE_SIZE_BITS;

    if (last_address <= address)
        return 0;

    if (len_bytes > (last_address-address)) {
        len_bytes = last_address - address;
    }

    memcpy( dest,
           UAIR_BSP_flash_storage_get_config_ptr_relative(address),
           len_bytes );

    return len_bytes;
}

/**
 * Writes must be multiple of 64-bit. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page) minus 7 (so that last 64-bit value, 8 bytes, still lies within the
 * flash allocated area.
 * @return number of bytes written or a (BSP_error_t) error value.
 */
int UAIR_BSP_flash_config_area_write(flash_address_t address, const uint64_t *data, size_t len_doublewords)
{
    int ret = 0;
    int count = 0;
    HAL_StatusTypeDef err;

    flash_address_t last_address = address - (len_doublewords * sizeof(uint64_t));

    if (!IS_ADDR_ALIGNED_64BITS(address)) {
        return BSP_ERROR_WRONG_PARAM;
    }

    if (last_address >= BSP_FLASH_CONFIG_NUM_PAGES*BSP_FLASH_PAGE_SIZE) {
        return BSP_ERROR_WRONG_PARAM;
    }

    do {

        err = HAL_FLASH_Unlock();

        if (err!=HAL_OK) {
            BSP_error_set(ERROR_ZONE_FLASH, BSP_ERROR_TYPE_FLASH_UNLOCK, err, 0);
            ret = BSP_ERROR_PERIPH_FAILURE;
            break;
        }

        do {
            err = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
                                    UAIR_BSP_flash_storage_get_config_physical_address(address),
                                    *data);
            if (err!=HAL_OK) {

                BSP_error_set(ERROR_ZONE_FLASH, BSP_ERROR_TYPE_FLASH_PROGRAM, err, 0);
                ret = BSP_ERROR_PERIPH_FAILURE;
                break;
            }
            count ++;
            address += sizeof(uint64_t);
            data++;

            ret = count;

            if (address>last_address) {
                /* Short write */
                break;
            }
        } while (--len_doublewords);

        if (err!=HAL_OK) {
            ret = BSP_ERROR_PERIPH_FAILURE;
        }

        err = HAL_FLASH_Lock();

        if (err!=HAL_OK) {
            BSP_error_set(ERROR_ZONE_FLASH, BSP_ERROR_TYPE_FLASH_LOCK, err, 0);
            ret = BSP_ERROR_PERIPH_FAILURE;
            break;
        }


        
    } while (0);

    return ret;
}
