#include "pvt/UAIR_BSP_flash_p.h"
#include "UAIR_BSP_flash.h"

#if defined (HOSTMODE)

#include "stm32wlxx_hal_flash_t.h"

extern uint8_t config_storage[];
extern uint8_t audit_storage[];

uint8_t UAIR_BSP_flash_storage_get_config_start_page(void)
{
    return T_HAL_FLASH_get_config_start_page();
}

uint8_t *UAIR_BSP_flash_storage_get_config_ptr_relative(uint32_t address)
{
    return T_HAL_FLASH_get_config_ptr_relative(address);
}

uint32_t UAIR_BSP_flash_storage_get_config_physical_address(uint32_t address)
{
    return T_HAL_FLASH_calc_physical_offset(address + T_HAL_FLASH_get_config_start_page() * BSP_FLASH_PAGE_SIZE);
}

uint8_t UAIR_BSP_flash_storage_get_audit_start_page(void)
{
    return T_HAL_FLASH_get_audit_start_page();
}

uint8_t *UAIR_BSP_flash_storage_get_audit_ptr_relative(uint32_t address)
{
    return T_HAL_FLASH_get_audit_ptr_relative(address);
}

uint32_t UAIR_BSP_flash_storage_get_audit_physical_address(uint32_t address)
{
    return T_HAL_FLASH_calc_physical_offset(address + T_HAL_FLASH_get_audit_start_page() * BSP_FLASH_PAGE_SIZE);
}

#else // HOSTMODE

static uint8_t FLASH_STORAGE_SECTION config_storage[BSP_FLASH_PAGE_SIZE * BSP_FLASH_CONFIG_NUM_PAGES];
static uint8_t FLASH_STORAGE_SECTION audit_storage[BSP_FLASH_PAGE_SIZE * BSP_FLASH_AUDIT_NUM_PAGES];

extern int _rom_start;

uint8_t UAIR_BSP_flash_storage_get_config_start_page(void)
{
    // cppcheck-suppress comparePointers ; The pointers refer to flash memory areas.
    unsigned offset = (unsigned)(&config_storage[0] - (uint8_t*)&_rom_start);
    return (uint8_t)(offset >> BSP_FLASH_PAGE_SIZE_BITS) & 0xff;
}

uint8_t *UAIR_BSP_flash_storage_get_config_ptr_relative(uint32_t address)
{
    return &config_storage[address];
}

uint32_t UAIR_BSP_flash_storage_get_config_physical_address(uint32_t address)
{
    return (uint32_t)&config_storage[address];
}

uint8_t UAIR_BSP_flash_storage_get_audit_start_page(void)
{
    // cppcheck-suppress comparePointers ; The pointers refer to flash memory areas.
    unsigned offset = (unsigned)(&audit_storage[0] - (uint8_t*)&_rom_start);
    return (uint8_t)(offset >> BSP_FLASH_PAGE_SIZE_BITS) & 0xff;
}

uint8_t *UAIR_BSP_flash_storage_get_audit_ptr_relative(uint32_t address)
{
    return &audit_storage[address];
}

uint32_t UAIR_BSP_flash_storage_get_audit_physical_address(uint32_t address)
{
    return (uint32_t)&audit_storage[address];
}

#endif // HOSTMODE
