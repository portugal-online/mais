#ifndef __UAIR_BSP_POWEZONE_H__
#define __UAIR_BSP_POWEZONE_H__

/* Power control */

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

#endif