#include "BSP.h"
#include "SHTC3.h"

#include "UAIR_BSP_internaltemp.h"
#include "pvt/UAIR_BSP_internaltemp_p.h"
#include "UAIR_BSP_i2c.h"
#include "pvt/UAIR_BSP_i2c_p.h"

/* SHTC3 temp. sensor */
static SHTC3_t shtc3 = {0};

enum {
    SHTC3_NOT_INIT,
    SHTC3_IDLE,
    SHTC3_MEASURE
} shtc3_state = SHTC3_NOT_INIT;

int UAIR_BSP_internal_temp_hum_init()
{
    HAL_I2C_bus_t bus;
    BSP_I2C_busnumber_t busno;
    
    // TBD: board variations
    BSP_powerzone_t powerzone = UAIR_POWERZONE_NONE;
    BSP_error_t err = BSP_ERROR_NO_INIT;

    do {
        switch (BSP_get_board_version()) {
        case UAIR_NUCLEO_REV1:
            busno = BSP_I2C_BUS0;
            /* Although we have no powerzone, we need to power up
             the I2C pullups */
            powerzone = UAIR_POWERZONE_INTERNALI2C;
            break;
        case UAIR_NUCLEO_REV2:
            busno = BSP_I2C_BUS0;
            powerzone = UAIR_POWERZONE_INTERNALI2C;
            break;
        default:
            bus = NULL;
            break;
        }

        // Power up if required

        if (UAIR_POWERZONE_NONE != powerzone) {
            if (BSP_powerzone_enable(powerzone)!=BSP_ERROR_NONE) {
                err = BSP_ERROR_BUS_FAILURE;
                break;
            }
        }
        /* Initialise bus */

        err = UAIR_BSP_I2C_InitBus(busno);

        if (err!=BSP_ERROR_NONE)
            break;

        bus = UAIR_BSP_I2C_GetHALHandle(busno);

        if (NULL==bus) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }

        BSP_TRACE("Initializing SHTC3 sensor");
        if (SHTC3_init(&shtc3, bus, 200) != SHTC3_STATUS_OK) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }

        BSP_TRACE("Probing SHTC3 sensor");
        // Probe
        if (SHTC3_probe(&shtc3)!=SHTC3_STATUS_OK) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        BSP_TRACE("SHTC3 sensor detected and initialised (%08x)", SHTC3_get_probed_serial(&shtc3));

        shtc3_state = SHTC3_IDLE;
        err = BSP_ERROR_NONE;

    } while (0);
    return err;
}

BSP_error_t BSP_internal_temp_hum_start_measure(void)
{
    BSP_error_t ret;
    BSP_TRACE("Starting internal temp measure");
    do {
        if (shtc3_state==SHTC3_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (shtc3_state==SHTC3_MEASURE) {
            ret = BSP_ERROR_BUSY;
            break;
        }

        // start measure
        if (SHTC3_wake_up(&shtc3)!=0) {
            BSP_TRACE("Cannot wakeup SHTC3");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }

        HAL_Delay(1);

        if (SHTC3_measure(&shtc3)!=0) {
            BSP_TRACE("Cannot start measure on SHTC3");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }

        shtc3_state = SHTC3_MEASURE;
        ret = BSP_ERROR_NONE;

    } while (0);

    return ret;
}

BSP_error_t BSP_internal_temp_hum_read_measure(int32_t *temp, int32_t *hum)
{
    BSP_error_t ret;
    BSP_TRACE("Reading internal temp measure");
    do {
        if (shtc3_state==SHTC3_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (shtc3_state==SHTC3_IDLE) {
            ret = BSP_ERROR_BUSY;
            break;
        }
        if (SHTC3_read(&shtc3, temp, hum)!=0) {
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }
        if (SHTC3_sleep(&shtc3)!=0) {
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }
        shtc3_state = SHTC3_IDLE;
        ret = BSP_ERROR_NONE;
    } while (0);
    return ret;
}

unsigned int BSP_internal_temp_hum_get_measure_delay_us(void)
{
    return 12100;  // 12.1ms max in normal mode.
}
