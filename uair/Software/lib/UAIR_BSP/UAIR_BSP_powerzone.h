#ifndef UAIR_BSP_POWEZONE_H__
#define UAIR_BSP_POWEZONE_H__

/* Power control */

#ifdef __cplusplus
extern "C" {
#endif

/* BSP errors */
enum powerzone_error_e {
    BSP_ERROR_TYPE_POWERZONE_ZONE_STILL_POWERED,
    BSP_ERROR_TYPE_POWERZONE_ZONE_NO_POWER
};

// Power zones
typedef enum {
    UAIR_POWERZONE_INTERNALI2C = 0,
    UAIR_POWERZONE_MICROPHONE,
    UAIR_POWERZONE_AMBIENTSENS,
    UAIR_POWERZONE_MAX = UAIR_POWERZONE_AMBIENTSENS,
    UAIR_POWERZONE_NONE = -1
} BSP_powerzone_t;

BSP_error_t BSP_powerzone_enable(BSP_powerzone_t powerzone);
BSP_error_t BSP_powerzone_disable(BSP_powerzone_t powerzone);

#ifdef __cplusplus
}
#endif

#endif
