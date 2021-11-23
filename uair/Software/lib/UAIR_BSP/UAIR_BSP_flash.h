#ifndef UAIR_BSP_FLASH_H__
#define UAIR_BSP_FLASH_H__

#include "UAIR_BSP_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_FLASH_SECTOR_SIZE_BITS (11) /* 2^11 == 2048 ª*/
#define BSP_FLASH_SECTOR_SIZE (1U<<BSP_FLASH_SECTOR_SIZE_BITS) /* 2048 */

/* A flash page. Max 256 pages */
typedef uint8_t flash_page_t;
/* A flash address, relative to area start. */
typedef uint32_t flash_address_t;

/**
 * return the number of pages, each BSP_FLASH_SECTOR_SIZE long, available
 * for configuration storage
 */
unsigned UAIR_BSP_flash_config_area_get_size_pages(void);


/**
 * Erase a page
 * @return TBD
 */
BSP_error_t UAIR_BSP_flash_config_area_erase_page(flash_page_t page);

/**
 * Read from a address. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page). Note that reads do not wrap around, a short read will
 * occur if reading beyond the allocated area.
 * @return number of bytes read or a (BSP_error_t) error value.
 */
int UAIR_BSP_flash_config_area_read(flash_address_t address, uint8_t *dest, size_t len_bytes);

/**
 * Writes must be multiple of 64-bit. Addresses start at 0x0 and extend to the size of the config area
 * (number of pages times size of page) minus 7 (so that last 64-bit value, 8 bytes, still lies within the
 * flash allocated area.
 * @return number of bytes read or a (BSP_error_t) error value.
 */
int UAIR_BSP_flash_config_area_write(flash_address_t address, const uint64_t *, size_t len_doublewords);

#ifdef __cplusplus
}
#endif

#endif
