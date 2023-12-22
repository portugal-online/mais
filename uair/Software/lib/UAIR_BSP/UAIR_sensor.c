#include "UAIR_sensor.h"
#include "UAIR_BSP_i2c.h"
#include "UAIR_BSP.h"

#define SENSOR_MAX_FAIL_COUNT (3)

extern UAIR_sensor_ops_t internal_sensor_ops;

bool UAIR_sensor_reset_bus(UAIR_sensor_t *sensor)
{
    HAL_I2C_bus_t bus = UAIR_BSP_I2C_GetHALHandle( sensor->ops->get_bus() );

    int err;

    if (NULL==bus) {
        return false;
    }
    // Re-init, re-init
    err = HAL_I2C_DeInit(bus);
    if (err != HAL_OK)
        return false;

    err= HAL_I2C_Init(bus);
    if (err != HAL_OK)
        return false;
    return true;
}

bool UAIR_sensor_reset_device_bus(UAIR_sensor_t *sensor, bool reset_bus)
{
    BSP_powerzone_ref(sensor->ops->get_powerzone());

    sensor->ops->deinit();

    if (sensor->ops->reset) {
        // Do not disable powerzone.
        sensor->ops->reset();
    } else {
        // Otherwise we need to power cycle
        UAIR_BSP_powerzone_cycle(sensor->ops->get_powerzone());
    }
    // Reset bus if required
    if (reset_bus)
    {
        if (!UAIR_sensor_reset_bus(sensor))
            return false;
    }
    sensor->ops->init();

    BSP_powerzone_unref(sensor->ops->get_powerzone());

    return true;
}

bool UAIR_sensor_reset_device(UAIR_sensor_t *sensor)
{
    return UAIR_sensor_reset_device_bus(sensor, false);
}

bool UAIR_sensor_reset_all(UAIR_sensor_t *sensor)
{
    return UAIR_sensor_reset_device_bus(sensor, true);
}

bool UAIR_sensor_manual_bus_release(UAIR_sensor_t *sensor)
{
    return (UAIR_BSP_I2C_manual_bus_release(sensor->ops->get_bus()) == BSP_ERROR_NONE);
}

bool UAIR_sensor_fatal(UAIR_sensor_t *sensor)
{
    sensor->ops->set_faulty();
    return true;
}

void UAIR_sensor_fault_detected(UAIR_sensor_t *sensor)
{
    bool actionok = false;

    if (sensor->failcount < SENSOR_MAX_FAIL_COUNT)
        sensor->failcount++;

    BSP_I2C_recover_action_t action = UAIR_BSP_I2C_analyse_and_recover_error( sensor->ops->get_bus() );

    BSP_TRACE("Sensor failcount: %d", sensor->failcount);

    if (sensor->failcount >= SENSOR_MAX_FAIL_COUNT)
    {
        switch (sensor->last_action)
        {
        case BSP_I2C_RECOVER_RETRY:
            action = BSP_I2C_RECOVER_MANUAL_BUS_RELEASE; // Not responding
            break;

        case BSP_I2C_RECOVER_MANUAL_BUS_RELEASE:
            action = BSP_I2C_RECOVER_RESET_DEVICE;
            break;

        case BSP_I2C_RECOVER_RESET_DEVICE:
            action = BSP_I2C_RECOVER_RESET_BUS;
            break;

        case BSP_I2C_RECOVER_RESET_BUS:
            action = BSP_I2C_RECOVER_RESET_ALL; 
            break;

        case BSP_I2C_RECOVER_RESET_ALL:
            action = BSP_I2C_RECOVER_FATAL_ERROR;
            break;

        case BSP_I2C_RECOVER_FATAL_ERROR:
            break;
        }
    }

    switch (action)
    {
    case BSP_I2C_RECOVER_RETRY:
        BSP_TRACE("Action: retry communication");
        actionok = true;
        break;
    case BSP_I2C_RECOVER_RESET_BUS:
        BSP_TRACE("Action: reset bus");
        actionok = UAIR_sensor_reset_bus(sensor);
        break;
    case BSP_I2C_RECOVER_RESET_DEVICE:
        BSP_TRACE("Action: reset device");
        actionok = UAIR_sensor_reset_device(sensor);
        break;
    case BSP_I2C_RECOVER_RESET_ALL:
        BSP_TRACE("Action: reset all");
        actionok = UAIR_sensor_reset_all(sensor);
        break;
    case BSP_I2C_RECOVER_MANUAL_BUS_RELEASE:
        BSP_TRACE("Action: manual bus release");
        actionok = UAIR_sensor_manual_bus_release(sensor);
        break;
    case BSP_I2C_RECOVER_FATAL_ERROR:
        BSP_TRACE("Action: fatal error");
        actionok = UAIR_sensor_fatal(sensor);
        break;
    }

    sensor->last_action = action;

    if (!actionok) {
        UAIR_sensor_fatal(sensor);
    }
}

void UAIR_sensor_ok(UAIR_sensor_t *sensor)
{
    if (sensor->failcount>0) {
        BSP_TRACE("Sensor OK");
    }
    sensor->failcount = 0;
}

