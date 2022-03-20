#ifndef UAIR_BSP_WATCHDOG_H__
#define UAIR_BSP_WATCHDOG_H__

#include "BSP.h"
#include "HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/* BSP error codes */
enum iwdg_error_e {
    BSP_ERROR_TYPE_IWDG_INIT,
    BSP_ERROR_TYPE_IWDG_KICK
};

void UAIR_BSP_watchdog_kick(void);

#ifdef __cplusplus
}
#endif

#endif
