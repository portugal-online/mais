#ifndef __UAIR_BSP_TYPES_H__
#define __UAIR_BSP_TYPES_H__

typedef enum {
    UAIR_UNKNOWN,
    UAIR_NUCLEO_REV1,
    UAIR_NUCLEO_REV2
} BSP_board_version_t;

// BSP configuration

typedef enum {
    TEMP_ACCURACY_LOW,
    TEMP_ACCURACY_MED,
    TEMP_ACCURACY_HIGH
} BSP_temp_accuracy_t;

typedef enum {
    HUM_ACCURACY_LOW,
    HUM_ACCURACY_MED,
    HUM_ACCURACY_HIGH
} BSP_hum_accuracy_t;

typedef enum {
    SENSOR_AVAILABLE,
    SENSOR_OFFLINE,
    SENSOR_FAULTY
} BSP_sensor_state_t;

typedef struct {
    /* Error handler */
    void (*bsp_error)(BSP_error_t error);
    BSP_temp_accuracy_t temp_accuracy;
    BSP_hum_accuracy_t hum_accuracy;
} BSP_config_t;

#endif