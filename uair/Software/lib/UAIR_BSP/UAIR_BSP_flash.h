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

/**
 * return the number of pages, each BSP_FLASH_PAGE_SIZE long, available
 * for configuration storage
 */
unsigned UAIR_BSP_flash_config_area_get_page_count(void);


/**
 * Erase a page identified by page [page]. This page is relative to the start of the config area
 * (starts at zero).
 *
 * If reported error is BSP_ERROR_PERIPH_FAILURE then BSP_error_get_last_error() can be used to obtain
 * the actual failure detected. This error will be raised on zone BSP_ZONE_FLASH and type can be one of the
 * follwing:
 *  - BSP_ERROR_TYPE_FLASH_UNLOCK  : Failure to unlock the flash prior to erase. Page was not erased.
 *  - BSP_ERROR_TYPE_FLASH_LOCK    : Failure to lock the flash after erase, but page was erased successfully.
 *  - BSP_ERROR_TYPE_FLASH_ERASE   : Erase failure occurred. Physical page address can be foung in the
 *                                   value field of the error structure.
 *
 * @return BSP_ERROR_NONE if page was successfully erased.
 * @return BSP_ERROR_WRONG_PARAM if [page] is not in range for the config area.
 * @return BSP_ERROR_PERIPH_FAILURE if flash could not be unlocked prior to erase, if it cannot be
 *         erased after programming, or if any erase error occurred.
 *
 */
BSP_error_t UAIR_BSP_flash_config_area_erase_page(flash_page_t page);


/**
 * Read from config area at [address], placing read data in [data] on a total of maximum [len_bytes].
 * Addresses start at 0x0 and extend to the size of the config area (number of pages times size of page).
 * Note that reads do not wrap around, a short read will occur if reading beyond the allocated area.
 *
 * @return number of bytes read in case of a successful (fully or partial) read
 * @return BSP_ERROR_WRONG_PARAM if address does not fall within the configuration address space
 */
int UAIR_BSP_flash_config_area_read(flash_address_t address, uint8_t *dest, size_t len_bytes);


/**
 * Writes 64-bit aligned data from [data] into config area at address [address], on a total of
 * [count_doublewords] doublewords (64-bit words)
 *
 * Writes must be multiple of 64-bit. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page) minus 7 (so that last 64-bit value, 8 bytes, still lies within the
 * flash allocated area.
 *
 * If reported error is BSP_ERROR_PERIPH_FAILURE then BSP_error_get_last_error() can be used to obtain
 * the actual failure detected. This error will be raised on zone BSP_ZONE_FLASH and type can be one of the
 * follwing:
 *  - BSP_ERROR_TYPE_FLASH_UNLOCK  : Failure to unlock the flash prior to programming. Nothing was written
 *    to flash.
 *  - BSP_ERROR_TYPE_FLASH_LOCK    : Failure to lock the flash after programming, but flash programming was
 *    successful.
 *  - BSP_ERROR_TYPE_FLASH_PROGRAM : Program failure occurred. Data written might not be consistent.
 *
 *
 * @return number of bytes written in case of a successful (fully or partial) read
 * @return BSP_ERROR_WRONG_PARAM if [address] is not 64-bit aligned
 * @return BSP_ERROR_WRONG_PARAM if [address] does not fall within the configuration address space
 * @return BSP_ERROR_PERIPH_FAILURE if flash could not be unlocked prior to programming, if it cannot be
 *         locked after programming, or if any programming error occurred.
 */
int UAIR_BSP_flash_config_area_write(flash_address_t address, const uint64_t *data, size_t count_doublewords);

#ifdef __cplusplus
}
#endif

#endif
