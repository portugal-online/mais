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
 * @ingroup UAIR_BSP_FLASH
 *
 * @copyright Copyright (c) 2021 MAIS Project
 *
 */

#include "UAIR_BSP_flash.h"
#include "pvt/UAIR_BSP_flash_p.h"
#include <string.h>
#include <stm32wlxx_hal_flash.h>
#include <stm32wlxx_hal_flash_ex.h>

/**
 * @brief Return number of pages available on config area
 * @ingroup UAIR_BSP_FLASH
 *
 *
 * Return the number of pages, each \ref BSP_FLASH_PAGE_SIZE long, available
 * for configuration storage
 *
 * @return Number of pages
 */
unsigned UAIR_BSP_flash_config_area_get_page_count(void)
{
    return BSP_FLASH_CONFIG_NUM_PAGES;
}

static uint8_t UAIR_BSP_flash_get_config_start_page(void)
{
    return UAIR_BSP_flash_storage_get_config_start_page();
}

/**
 * @brief Erase page
 * @ingroup UAIR_BSP_FLASH
 *
 *
 * Erase a page identified by page \p page. This page is relative to the start of the config area
 * (starts at zero).
 *
 * If reported error is \ref BSP_ERROR_PERIPH_FAILURE then \ref BSP_error_get_last_error() can be used to obtain
 * the actual failure detected. This error will be raised on zone \ref BSP_ZONE_FLASH and type can be one of the
 * follwing:
 *  - \ref BSP_ERROR_TYPE_FLASH_UNLOCK  : Failure to unlock the flash prior to erase. Page was not erased.
 *  - \ref BSP_ERROR_TYPE_FLASH_LOCK    : Failure to lock the flash after erase, but page was erased successfully.
 *  - \ref BSP_ERROR_TYPE_FLASH_ERASE   : Erase failure occurred. Physical page address can be foung in the
 *                                   value field of the error structure.
 *
 * @param page Page to be erased. Page starts at zero
 *
 * @return \ref BSP_ERROR_NONE if page was successfully erased.
 * @return \ref BSP_ERROR_WRONG_PARAM if \p page is not in range for the config area.
 * @return \ref BSP_ERROR_PERIPH_FAILURE if flash could not be unlocked prior to erase, if it cannot be
 *         erased after programming, or if any erase error occurred.
 *
 */

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
 * @brief Read from FLASH config area
 * @ingroup UAIR_BSP_FLASH
 *
 *
 * Read from a config area address. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page). Note that reads do not wrap around, a short read will
 * occur if reading beyond the allocated area.
 *
 *
 * @param address Relative address to read from
 * @param dest Pointer to destination buffer where data will be read into
 * @param len_bytes Read length in bytes
 *
 * @return positive number of bytes read.
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
 * @brief Write to FLASH config area
 * @ingroup UAIR_BSP_FLASH
 *
 *
 * Writes 64-bit aligned data from \p data into config area at address \p address, on a total of
 * \p count_doublewords doublewords (64-bit words)
 *
 * Writes must be multiple of 64-bit. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page) minus 7 (so that last 64-bit value, 8 bytes, still lies within the
 * flash allocated area.
 *
 * If reported error is \ref BSP_ERROR_PERIPH_FAILURE then \ref BSP_error_get_last_error() can be used to obtain
 * the actual failure detected. This error will be raised on zone BSP_ZONE_FLASH and type can be one of the
 * follwing:
 *  - \ref BSP_ERROR_TYPE_FLASH_UNLOCK  : Failure to unlock the flash prior to programming. Nothing was written
 *    to flash.
 *  - \ref BSP_ERROR_TYPE_FLASH_LOCK    : Failure to lock the flash after programming, but flash programming was
 *    successful.
 *  - \ref BSP_ERROR_TYPE_FLASH_PROGRAM : Program failure occurred. Data written might not be consistent.
 *
 * @param address Relative address where to write. Needs to be 64-bit aligned.
 * @param data Pointer to data (64-bit aligned) to write
 * @param len_doublewords Number of 64-bit words to write.
 *
 * @return positive number of bytes written in case of a successful (fully or partial) read
 * @return BSP_ERROR_WRONG_PARAM if \p address is not 64-bit aligned
 * @return BSP_ERROR_WRONG_PARAM if \p address does not fall within the configuration address space
 * @return BSP_ERROR_PERIPH_FAILURE if flash could not be unlocked prior to programming, if it cannot be
 *         locked after programming, or if any programming error occurred.
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
