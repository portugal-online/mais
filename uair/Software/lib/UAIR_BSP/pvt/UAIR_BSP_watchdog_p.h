#ifndef UAIR_BSP_WATCHDOG_H__
#define UAIR_BSP_WATCHDOG_H__

#include "UAIR_BSP_watchdog.h"

#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t UAIR_BSP_watchdog_init(uint32_t seconds);

#ifdef __cplusplus
}
#endif

#endif
