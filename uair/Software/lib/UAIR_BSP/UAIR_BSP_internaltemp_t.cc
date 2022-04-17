#include <catch2/catch.hpp>

#include "UAIR_BSP_error.h"
#include "UAIR_BSP_types.h"
#include "UAIR_BSP.h"
#include "UAIR_BSP_internaltemp.h"
#include "UAIR_BSP_powerzone.h"
#include "pvt/UAIR_BSP_powerzone_p.h"
#include "pvt/UAIR_BSP_internaltemp_p.h"
#include "stm32wlxx_hal_i2c_pvt.h"
#include "stm32wlxx_hal.h"
#include "models/shtc3.h"
#include <unistd.h>
#include "tests/uAirUnitTestFixture.hpp"

#include <limits.h>

#define RASSERT(req, x...) \
    UNSCOPED_INFO( "Checking requirement " #req ); \
    CHECK ( x )

extern struct shtc3_model *shtc3;

void check_sensor_offline()
{
    int32_t temp;
    int32_t hum;
    CHECK( BSP_internal_temp_hum_get_sensor_state() == SENSOR_OFFLINE );
    CHECK( BSP_internal_temp_hum_start_measure() == BSP_ERROR_NO_INIT );
    CHECK( BSP_internal_temp_hum_read_measure(&temp, &hum) == BSP_ERROR_NO_INIT );
}

TEST_CASE_METHOD(uAirUnitTestFixture, "Basic SHTC3 initialization","[BSP][BSP/Sensors][BSP/Sensors/InternalSensor]")
{
    BSP_error_t r;

    // Set I2C error.
    ASSERT( UAIR_BSP_powerzone_init() == BSP_ERROR_NONE );

    i2c_set_error_mode( I2C1, 0x70<<1, I2C_FAIL_PRETX, HAL_I2C_ERROR_AF );

    r = UAIR_BSP_internal_temp_hum_init();

    CHECK( r == BSP_ERROR_PERIPH_FAILURE );
    check_sensor_offline();

    i2c_set_error_mode( I2C1, 0x70<<1, I2C_FAIL_POSTTX, HAL_I2C_ERROR_AF );

    r = UAIR_BSP_internal_temp_hum_init();

    CHECK( r == BSP_ERROR_PERIPH_FAILURE );
    check_sensor_offline();

    i2c_set_error_mode( I2C1, 0x70<<1, I2C_NORMAL, 0);

    r = UAIR_BSP_internal_temp_hum_init();

    CHECK( r == BSP_ERROR_NONE );

    UAIR_BSP_powerzone_deinit();

    // At startup, sensor should not be available
    check_sensor_offline();

    ASSERT( UAIR_BSP_powerzone_init() == BSP_ERROR_NONE );

    // After powerup sensor should not be available
    check_sensor_offline();

    // Init sensor.
    r = UAIR_BSP_internal_temp_hum_init();
    CHECK( r == BSP_ERROR_NONE );

    // Check state
    CHECK( BSP_internal_temp_hum_get_sensor_state() == SENSOR_AVAILABLE );

    // Power it down
    BSP_powerzone_disable ( UAIR_POWERZONE_INTERNALI2C );

    // After power-down sensor should not be available
    check_sensor_offline();

    // Then power up
    BSP_powerzone_enable ( UAIR_POWERZONE_INTERNALI2C );

    // After powerup sensor should not be available
    check_sensor_offline();

}

TEST_CASE_METHOD(uAirUnitTestFixture, "Data capture","[BSP][BSP/Sensors][BSP/Sensors/InternalSensor]")
{
    int32_t temp;
    int32_t hum;
    BSP_error_t r;

    UAIR_BSP_powerzone_deinit();
    ASSERT( UAIR_BSP_powerzone_init() == BSP_ERROR_NONE );

    // Init sensor.
    r = UAIR_BSP_internal_temp_hum_init();
    CHECK( r == BSP_ERROR_NONE );

    CHECK( BSP_internal_temp_hum_get_sensor_state() == SENSOR_AVAILABLE );

    CHECK( BSP_internal_temp_hum_start_measure() == BSP_ERROR_NONE );

    // Currently we do not check for busy, but SHTC3 will not respond.
    CHECK( BSP_internal_temp_hum_read_measure(&temp, &hum) == BSP_ERROR_COMPONENT_FAILURE);

    usleep(BSP_internal_temp_hum_get_measure_delay_us());

    CHECK( BSP_internal_temp_hum_read_measure(&temp, &hum) == BSP_ERROR_NONE );

    CHECK( BSP_internal_temp_hum_start_measure() == BSP_ERROR_NONE );
}

