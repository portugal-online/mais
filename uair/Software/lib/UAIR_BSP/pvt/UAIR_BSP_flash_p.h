#ifndef UAIR_BSP_FLASH_P_H__
#define UAIR_BSP_FLASH_P_H__

#include <inttypes.h>

#define BSP_FLASH_CONFIG_NUM_PAGES (2U)

#if defined(HOSTMODE)

#define FLASH_STORAGE_SECTION /* */

#else // HOSTMODE

#define FLASH_STORAGE_SECTION __attribute__((section (".storage")))

#endif // HOSTMODE


uint8_t UAIR_BSP_flash_storage_get_config_start_page(void);
uint8_t *UAIR_BSP_flash_storage_get_config_ptr_relative(uint32_t address);
uint32_t UAIR_BSP_flash_storage_get_config_physical_address(uint32_t address);

#endif

