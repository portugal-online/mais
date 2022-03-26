#ifndef HAL_FLASH_T_H__
#define HAL_FLASH_T_H__

#include <inttypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error control */
struct t_hal_flash_error_control {
    int flash_lock_error;
    int flash_unlock_error;
    int flash_program_error;
    int flash_erase_error;
    int error_page;
};

void __attribute__((constructor)) T_HAL_FLASH_init(void);
void T_HAL_FLASH_reset_locks(void);
void T_HAL_FLASH_clear_storage(uint8_t val);
void T_HAL_FLASH_check_storage_bounds(void);
uint8_t *T_HAL_FLASH_get_storage(void);

bool T_HAL_FLASH_is_flash_locked(void);
int T_HAL_FLASH_get_flash_lock_count(void);
int T_HAL_FLASH_get_flash_unlock_count(void);
bool T_HAL_FLASH_get_flash_lock_status_on_erase(void);
bool T_HAL_FLASH_get_flash_lock_status_on_program(void);
void T_HAL_FLASH_set_error_control(const struct t_hal_flash_error_control *);

uint8_t T_HAL_FLASH_get_start_page(void);
uint32_t T_HAL_FLASH_calc_physical_offset(uint32_t address);

#ifdef __cplusplus
}
#endif

#endif
