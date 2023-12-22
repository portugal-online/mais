#ifndef UAIR_SENSOR_H__
#define UAIR_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <UAIR_BSP_error.h>
#include <UAIR_BSP_powerzone.h>
#include <UAIR_BSP_i2c.h>
#include <pvt/UAIR_BSP_i2c_p.h>

struct UAIR_sensor;

typedef struct
{
    BSP_error_t (*init)(void);
    void (*deinit)(void);
    void (*reset)(void);
    BSP_powerzone_t (*get_powerzone)(void);
    BSP_I2C_busnumber_t (*get_bus)(void);
    void (*set_faulty)(void);
} UAIR_sensor_ops_t;

typedef struct UAIR_sensor
{
    const UAIR_sensor_ops_t *ops;
    uint8_t failcount;
    BSP_I2C_recover_action_t last_action;
} UAIR_sensor_t;

void UAIR_sensor_fault_detected(UAIR_sensor_t *sensor);
void UAIR_sensor_ok(UAIR_sensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif
