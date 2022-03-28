#include <catch2/catch.hpp>

#include "UAIR_BSP_flash.h"
#include "stm32wlxx_hal_flash_t.h"
#include "stm32wlxx_hal.h"
#include <limits.h>

#define RASSERT(req, x...) \
    UNSCOPED_INFO( "Checking requirement " #req ); \
    CHECK ( x )

static bool check_contents(uint8_t *address, size_t size, uint8_t ch)
{
    while (size--) {
        if (*address!=ch)
            return false;
        address++;
    }
    return true;
}


TEST_CASE("Erase operations","[BSP][BSP/Flash]")
{
    struct t_hal_flash_error_control error_control = { 0 };

    RASSERT( BSP_FLASH_REQ_100, UAIR_BSP_flash_config_area_get_page_count() == 2);

    T_HAL_FLASH_reset_locks();

    RASSERT( BSP_FLASH_REQ_200, UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_NONE );
    RASSERT( BSP_FLASH_REQ_200, check_contents(&T_HAL_FLASH_get_storage()[0], 2048, 0xff) == true );
    RASSERT( BSP_FLASH_REQ_200, check_contents(&T_HAL_FLASH_get_storage()[2048], 2048, 0x00) == true );

    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_208, T_HAL_FLASH_get_flash_lock_count() == 1);
    RASSERT( BSP_FLASH_REQ_208, T_HAL_FLASH_get_flash_lock_status_on_erase() == false );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);

    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_clear_storage(0x00);

    T_HAL_FLASH_reset_locks();

    RASSERT( BSP_FLASH_REQ_200, UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_NONE );
    RASSERT( BSP_FLASH_REQ_200, check_contents(&T_HAL_FLASH_get_storage()[0], 2048, 0x00) == true );
    RASSERT( BSP_FLASH_REQ_200, check_contents(&T_HAL_FLASH_get_storage()[2048], 2048, 0xff) == true );

    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_208, T_HAL_FLASH_get_flash_lock_count() == 1);
    RASSERT( BSP_FLASH_REQ_208, T_HAL_FLASH_get_flash_lock_status_on_erase() == false );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_clear_storage(0xAA);

    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_201, UAIR_BSP_flash_config_area_erase_page(2) == BSP_ERROR_WRONG_PARAM );
    RASSERT( BSP_FLASH_REQ_201, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xAA )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_201, UAIR_BSP_flash_config_area_erase_page(-1) == BSP_ERROR_WRONG_PARAM );
    RASSERT( BSP_FLASH_REQ_201, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xAA )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_201, UAIR_BSP_flash_config_area_erase_page(CHAR_MAX) == BSP_ERROR_WRONG_PARAM );
    RASSERT( BSP_FLASH_REQ_201, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xAA )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_201, UAIR_BSP_flash_config_area_erase_page(CHAR_MIN) == BSP_ERROR_WRONG_PARAM );
    RASSERT( BSP_FLASH_REQ_201, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xAA )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    error_control.flash_erase_error = 1;
    T_HAL_FLASH_set_error_control(&error_control);

    T_HAL_FLASH_reset_locks();
    BSP_error_reset();
    RASSERT( BSP_FLASH_REQ_206, UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_207, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_207, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_ERASE );
    RASSERT( BSP_FLASH_REQ_207, BSP_error_get_last_error().value == T_HAL_FLASH_get_config_start_page() ); // This is physical page
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_reset_locks();
    BSP_error_reset();
    RASSERT( BSP_FLASH_REQ_206, UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_207, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_207, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_ERASE );
    RASSERT( BSP_FLASH_REQ_207, BSP_error_get_last_error().value == T_HAL_FLASH_get_config_start_page()+1 ); // This is physical page
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    // Lock/unlock
    error_control.flash_erase_error = 0;
    error_control.flash_lock_error = 1;
    T_HAL_FLASH_set_error_control(&error_control);

    T_HAL_FLASH_reset_locks();
    BSP_error_reset();
    RASSERT( BSP_FLASH_REQ_204, UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_205, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_205, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_LOCK );
    RASSERT( BSP_FLASH_REQ_204, check_contents(&T_HAL_FLASH_get_storage()[0], 2048, 0xFF )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_208, T_HAL_FLASH_get_flash_lock_status_on_erase() == false );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == false);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_reset_locks();
    BSP_error_reset();
    RASSERT( BSP_FLASH_REQ_204, UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_205, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_205, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_LOCK );
    RASSERT( BSP_FLASH_REQ_204, check_contents(&T_HAL_FLASH_get_storage()[2048], 2048, 0xFF )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_208, T_HAL_FLASH_get_flash_lock_status_on_erase() == false );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == false);
    T_HAL_FLASH_check_storage_bounds();

    error_control.flash_lock_error = 0;
    error_control.flash_unlock_error = 1;
    T_HAL_FLASH_set_error_control(&error_control);

    T_HAL_FLASH_clear_storage(0xAA);

    T_HAL_FLASH_reset_locks();
    BSP_error_reset();
    RASSERT( BSP_FLASH_REQ_202, UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_203, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_203, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_UNLOCK );
    RASSERT( BSP_FLASH_REQ_202, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xAA )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    T_HAL_FLASH_reset_locks();
    BSP_error_reset();
    RASSERT( BSP_FLASH_REQ_202, UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_203, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_203, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_UNLOCK );
    RASSERT( BSP_FLASH_REQ_202, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xAA )==true );
    RASSERT( BSP_FLASH_REQ_209, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    error_control.flash_unlock_error = 0;
    T_HAL_FLASH_set_error_control(&error_control);
}

TEST_CASE("Write operations","[BSP][BSP/Flash]")
{
    int i;
    uint64_t data[2];
    uint64_t *pstorage64 = (uint64_t*)T_HAL_FLASH_get_storage();
    struct t_hal_flash_error_control error_control = { 0 };

    // Erase everything
    T_HAL_FLASH_reset_locks();
    ASSERT( UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_NONE );
    ASSERT( UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_NONE );
    ASSERT( pstorage64[0] == 0xFFFFFFFFFFFFFFFFULL);

    // Alignment check
    data[0] = 0xAAAAAAAA55555555ULL;
    data[1] = 0xBEBEBEBECACACACAULL;

    for (i=1; i<7; i++) {
        T_HAL_FLASH_reset_locks();
        RASSERT( BSP_FLASH_REQ_401, UAIR_BSP_flash_config_area_write(0x00000000 + i, data, 1) == BSP_ERROR_WRONG_PARAM );
        RASSERT( BSP_FLASH_REQ_401, check_contents(&T_HAL_FLASH_get_storage()[0], 4096, 0xFF )==true );
    }

    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_400, UAIR_BSP_flash_config_area_write(0x00000000, data, 2) == 2 );
    RASSERT( BSP_FLASH_REQ_400, pstorage64[0] ==  0xAAAAAAAA55555555ULL);
    RASSERT( BSP_FLASH_REQ_400, pstorage64[1] ==  0xBEBEBEBECACACACAULL);

    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_get_flash_lock_count() == 1);
    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_lock_status_on_program() == false );
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    // write last 2 addresses
    
    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_400, UAIR_BSP_flash_config_area_write(0x00000FF0, data, 2) == 2 );
    RASSERT( BSP_FLASH_REQ_400, pstorage64[0] ==  0xAAAAAAAA55555555ULL);
    RASSERT( BSP_FLASH_REQ_400, pstorage64[1] ==  0xBEBEBEBECACACACAULL);
    RASSERT( BSP_FLASH_REQ_400, pstorage64[510] ==  0xAAAAAAAA55555555ULL);
    RASSERT( BSP_FLASH_REQ_400, pstorage64[511] ==  0xBEBEBEBECACACACAULL);

    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_get_flash_lock_count() == 1);
    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_lock_status_on_program() == false );
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    RASSERT( BSP_FLASH_REQ_410, UAIR_BSP_flash_config_area_write(0xFFFFFFFF, data, 1) == BSP_ERROR_WRONG_PARAM );
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    RASSERT( BSP_FLASH_REQ_410, UAIR_BSP_flash_config_area_write(0x00001000, data, 1) == BSP_ERROR_WRONG_PARAM );
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();

    // write last address, overflow
    ASSERT( UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_NONE );
    ASSERT( UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_NONE );
    
    T_HAL_FLASH_reset_locks();
    RASSERT( BSP_FLASH_REQ_402, UAIR_BSP_flash_config_area_write(0x00000FF8, data, 2) == 1 );
    RASSERT( BSP_FLASH_REQ_402, pstorage64[511] ==  0xAAAAAAAA55555555ULL);

    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_unlock_count() == 1);
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_get_flash_lock_count() == 1);
    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_lock_status_on_program() == false );
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_is_flash_locked() == true);
    T_HAL_FLASH_check_storage_bounds();


    //BSP_FLASH_REQ_403

    
    T_HAL_FLASH_reset_locks();
    ASSERT( UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_NONE );
    ASSERT( UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_NONE );

    error_control.flash_unlock_error = 1;
    T_HAL_FLASH_set_error_control(&error_control);

    BSP_error_reset();

    RASSERT( BSP_FLASH_REQ_403, UAIR_BSP_flash_config_area_write(0x00000000, data, 1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_403, T_HAL_FLASH_is_flash_locked() == true);
    RASSERT( BSP_FLASH_REQ_403, pstorage64[0] ==  0xFFFFFFFFFFFFFFFFULL);
    RASSERT( BSP_FLASH_REQ_406, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_406, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_UNLOCK );

    BSP_error_reset();

    RASSERT( BSP_FLASH_REQ_403, UAIR_BSP_flash_config_area_write(0x00000FF8, data, 1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_403, T_HAL_FLASH_is_flash_locked() == true);
    RASSERT( BSP_FLASH_REQ_403, pstorage64[511] ==  0xFFFFFFFFFFFFFFFFULL);
    RASSERT( BSP_FLASH_REQ_406, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_406, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_UNLOCK );

    error_control.flash_unlock_error = 0;
    T_HAL_FLASH_set_error_control(&error_control);

    T_HAL_FLASH_check_storage_bounds();

    // 404

    T_HAL_FLASH_reset_locks();
    ASSERT( UAIR_BSP_flash_config_area_erase_page(0) == BSP_ERROR_NONE );
    ASSERT( UAIR_BSP_flash_config_area_erase_page(1) == BSP_ERROR_NONE );

    error_control.flash_lock_error = 1;
    T_HAL_FLASH_set_error_control(&error_control);
    BSP_error_reset();

    RASSERT( BSP_FLASH_REQ_404, UAIR_BSP_flash_config_area_write(0x00000000, data, 1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_404, T_HAL_FLASH_is_flash_locked() == false);
    RASSERT( BSP_FLASH_REQ_404, pstorage64[0] ==  0xAAAAAAAA55555555ULL);
    RASSERT( BSP_FLASH_REQ_405, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_405, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_LOCK );

    BSP_error_reset();

    RASSERT( BSP_FLASH_REQ_404, UAIR_BSP_flash_config_area_write(0x00000FF8, data, 1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_404, T_HAL_FLASH_is_flash_locked() == false);
    RASSERT( BSP_FLASH_REQ_404, pstorage64[511] ==  0xAAAAAAAA55555555ULL);
    RASSERT( BSP_FLASH_REQ_405, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_405, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_LOCK );

    error_control.flash_lock_error = 0;
    T_HAL_FLASH_set_error_control(&error_control);

    // 407/411
    T_HAL_FLASH_reset_locks();
    BSP_error_reset();

    error_control.flash_program_error = 1;
    T_HAL_FLASH_set_error_control(&error_control);

    RASSERT( BSP_FLASH_REQ_407, UAIR_BSP_flash_config_area_write(0x00000FF8, data, 1) == BSP_ERROR_PERIPH_FAILURE );
    RASSERT( BSP_FLASH_REQ_411, BSP_error_get_last_error().zone == ERROR_ZONE_FLASH );
    RASSERT( BSP_FLASH_REQ_411, BSP_error_get_last_error().type == BSP_ERROR_TYPE_FLASH_PROGRAM );
    RASSERT( BSP_FLASH_REQ_409, T_HAL_FLASH_is_flash_locked() == true);
    RASSERT( BSP_FLASH_REQ_408, T_HAL_FLASH_get_flash_lock_status_on_program() == false );

    error_control.flash_program_error = 0;
    T_HAL_FLASH_set_error_control(&error_control);

    T_HAL_FLASH_check_storage_bounds();

}