#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_def.h"
#include "stm32wlxx_hal_flash.h"
#include "stm32wlxx_hal_flash_t.h"
#include "stm32wlxx_hal_flash_ex.h"
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MEM_CANARY (0xBD)

extern uint8_t config_storage[];
extern uint8_t _rom_start[];
extern uint8_t _rom_end[];
extern uint8_t _flash_end[];

// Comes from STM HAL.
FLASH_ProcessTypeDef pFlash;

static bool flash_locked = true;
static int flash_lock_count = 0;
static int flash_unlock_count = 0;
static bool flash_lock_status_on_erase;
static bool flash_lock_status_on_program;

#define FLASH_VIRTUAL_ADDR (0x80000000U)
#define FLASH_VIRTUAL_MASK (0xFFFF0000U)

static struct t_hal_flash_error_control error_control = {0};

uint8_t T_HAL_FLASH_get_start_page(void)
{
    size_t delta = &config_storage[0] - &_rom_start[0];
    return delta / FLASH_PAGE_SIZE;
}

uint32_t T_HAL_FLASH_calc_physical_offset(uint32_t address)
{
    return FLASH_VIRTUAL_ADDR + address;
}



HAL_StatusTypeDef HAL_FLASH_Lock()
{
    if (error_control.flash_lock_error) {
        return HAL_ERROR;
    }
    flash_locked = true;
    flash_lock_count++;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Unlock()
{
    if (error_control.flash_unlock_error) {
        return HAL_ERROR;
    }
    flash_locked = false;
    flash_unlock_count++;
    return HAL_OK;
}


HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *PageError)
{
    ASSERT( pEraseInit->TypeErase == FLASH_TYPEERASE_PAGES );

    flash_lock_status_on_erase = flash_locked;

    if (error_control.flash_erase_error) {
        // FlashEx_Erase does not populate error
        //pFlash.ErrorCode = error_control.flash_erase_error;
        *PageError = pEraseInit->Page;
        return HAL_ERROR;
    }

    ASSERT (pEraseInit->Page >= 4 );
    ASSERT (pEraseInit->Page <= 5 );
    ASSERT (pEraseInit->NbPages == 1);

    *PageError = 0xFFFFFFFFU;

    uint8_t *ptr = &_rom_start[pEraseInit->Page * 2048];
    memset(ptr, 0xff, 2048);


    return HAL_OK;
}

HAL_StatusTypeDef  HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
    ASSERT(TypeProgram==FLASH_TYPEPROGRAM_DOUBLEWORD);
    ASSERT((Address & 0x7)==0x0); /* 64-bit aligned */
    ASSERT((Address & FLASH_VIRTUAL_MASK) == FLASH_VIRTUAL_ADDR);

    Address &= ~FLASH_VIRTUAL_MASK;//ASSERT(Address >= (uint32_t)config_storage);

    flash_lock_status_on_program = flash_locked;

    if (error_control.flash_program_error) {
        pFlash.ErrorCode = error_control.flash_program_error;
        return HAL_ERROR;
    }
    uint64_t *p = (uint64_t*)&config_storage[Address];

    *p = (*p) & Data; // Only zeroes can be programmed


    return HAL_OK;
}

static bool check_contents(uint8_t *address, size_t size, uint8_t ch)
{
    while (size--) {
        if (*address!=ch)
            return false;
        address++;
    }
    return true;
}

void T_HAL_FLASH_reset_locks(void)
{
    flash_lock_status_on_erase = true;
    flash_lock_status_on_program = true;
    flash_lock_count = 0;
    flash_unlock_count = 0;
    flash_locked = true;
}

void T_HAL_FLASH_clear_storage(uint8_t val)
{
    memset(config_storage, val, 4096);
}

void T_HAL_FLASH_check_storage_bounds(void)
{
    ASSERT( check_contents(_rom_start, 8192, MEM_CANARY ));
    ASSERT( check_contents(_rom_end, 8192, MEM_CANARY ));
}

bool T_HAL_FLASH_is_flash_locked(void)
{
    return flash_locked;
}

int T_HAL_FLASH_get_flash_lock_count(void)
{
    return flash_lock_count;
}

int T_HAL_FLASH_get_flash_unlock_count(void)
{
    return flash_unlock_count;
}

bool T_HAL_FLASH_get_flash_lock_status_on_erase(void)
{
    return flash_lock_status_on_erase;
}

bool T_HAL_FLASH_get_flash_lock_status_on_program(void)
{
    return flash_lock_status_on_program;
}


void T_HAL_FLASH_init()
{
    memset(&_rom_start, MEM_CANARY, 8192);
    memset(&_rom_end, MEM_CANARY, 8192);
}

uint8_t* T_HAL_FLASH_get_storage(void)
{
    return config_storage;
}

void T_HAL_FLASH_set_error_control(const struct t_hal_flash_error_control *s)
{
    memcpy(&error_control, s, sizeof(error_control));
}
