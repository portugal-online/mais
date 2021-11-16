#include "BSP.h"
#include "HS300X.h"

#include "UAIR_BSP_externaltemp.h"
#include "pvt/UAIR_BSP_externaltemp_p.h"
#include "UAIR_BSP_i2c.h"
#include "pvt/UAIR_BSP_i2c_p.h"


static HS300X_t hs300x = {0};

enum {
    HS300X_NOT_INIT,
    HS300X_IDLE,
    HS300X_MEASURE
} hs300x_state = HS300X_NOT_INIT;

static inline HS300X_accuracy_t UAIR_BSP_BSP_temp_accuracy_to_hs300x(const BSP_temp_accuracy_t t)
{
    HS300X_accuracy_t acc;
    switch (t) {
    case TEMP_ACCURACY_LOW:
        acc = HS300X_ACCURACY_8BIT;
        break;
    case TEMP_ACCURACY_MED:
        acc = HS300X_ACCURACY_10BIT;
        break;
    case TEMP_ACCURACY_HIGH:
        acc = HS300X_ACCURACY_12BIT;
        break;
    default:
        acc = HS300X_ACCURACY_8BIT;
        break;
    }
    return acc;
}

static inline HS300X_accuracy_t UAIR_BSP_BSP_hum_accuracy_to_hs300x(const BSP_hum_accuracy_t t)
{
    HS300X_accuracy_t acc;
    switch (t) {
    case HUM_ACCURACY_LOW:
        acc = HS300X_ACCURACY_8BIT;
        break;
    case HUM_ACCURACY_MED:
        acc = HS300X_ACCURACY_10BIT;
        break;
    case HUM_ACCURACY_HIGH:
        acc = HS300X_ACCURACY_12BIT;
        break;
    default:
        acc = HS300X_ACCURACY_8BIT;
        break;
    }
    return acc;

}

int UAIR_BSP_external_temp_hum_init(BSP_temp_accuracy_t temp_acc, BSP_hum_accuracy_t hum_acc)
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
            powerzone = UAIR_POWERZONE_AMBIENTSENS;
            break;
        case UAIR_NUCLEO_REV2:
            busno = BSP_I2C_BUS1;
            powerzone = UAIR_POWERZONE_AMBIENTSENS;
            break;
        default:
            busno = BSP_I2C_BUS0;
            break;
        }

        // Power down

        if (UAIR_POWERZONE_NONE != powerzone) {
            if (BSP_powerzone_disable(powerzone)!=BSP_ERROR_NONE) {
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
        // Power up

        BSP_TRACE("Initializing HS300X sensor");
        if (HS300X_init(&hs300x, bus) != HAL_OK) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }

        BSP_TRACE("Probing HS300X sensor");

        if (UAIR_POWERZONE_NONE != powerzone) {
            if (BSP_powerzone_enable(powerzone)!=BSP_ERROR_NONE) {
                err = BSP_ERROR_BUS_FAILURE;
                break;
            }
        }

        HS300X_accuracy_t hs_temp_acc = UAIR_BSP_BSP_temp_accuracy_to_hs300x(temp_acc);
        HS300X_accuracy_t hs_hum_acc = UAIR_BSP_BSP_hum_accuracy_to_hs300x(hum_acc);

        // Probe
        if (HS300X_probe(&hs300x, hs_hum_acc, hs_temp_acc)!=HAL_OK) {
            err = BSP_ERROR_PERIPH_FAILURE;
            break;
        }
        BSP_TRACE("HS300X sensor detected and initialised (%08x)", HS300X_get_probed_serial(&hs300x));

        hs300x_state = HS300X_IDLE;
        err = BSP_ERROR_NONE;

    } while (0);
    return err;
}

BSP_error_t BSP_external_temp_hum_start_measure(void)
{
    BSP_error_t ret;
    do {
        if (hs300x_state==HS300X_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (hs300x_state==HS300X_MEASURE) {
            ret = BSP_ERROR_BUSY;
            break;
        }

        if (HS300X_start_measurement(&hs300x)!=0) {
            BSP_TRACE("Cannot start measure on HS300X");
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }

        hs300x_state = HS300X_MEASURE;
        ret = BSP_ERROR_NONE;

    } while (0);

    return ret;
}

BSP_error_t BSP_external_temp_hum_read_measure(int32_t *temp, int32_t *hum)
{
    BSP_error_t ret;
    int stale;
    do {
        if (hs300x_state==HS300X_NOT_INIT) {
            ret = BSP_ERROR_NO_INIT;
            break;
        }
        if (hs300x_state==HS300X_IDLE) {
            ret = BSP_ERROR_BUSY;
            break;
        }
        if (HS300X_read_measurement(&hs300x, temp, hum, &stale)!=0) {
            ret = BSP_ERROR_COMPONENT_FAILURE;
            break;
        }

        hs300x_state = HS300X_IDLE;
        if (stale)
            ret = BSP_ERROR_BUSY;
        else
            ret = BSP_ERROR_NONE;
    } while (0);
    return ret;
}

unsigned int BSP_external_temp_hum_get_measure_delay_us(void)
{
    return HS300X_time_for_measurement_us(hs300x.temp_acc, hs300x.hum_acc);

}
