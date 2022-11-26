#ifndef UAIR_BSP_AIR_QUALITY_P_H__
#define UAIR_BSP_AIR_QUALITY_P_H__

#include "UAIR_BSP_error.h"
#include "ZMOD4510.h"


#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t UAIR_BSP_air_quality_init(void);
void UAIR_BSP_air_quality_deinit(void);
ZMOD4510_t *UAIR_BSP_air_quality_get_zmod(void);
void UAIR_BSP_air_quality_powerzone_changed(void *userdata, const powerstate_t state);

#ifdef __cplusplus
}
#endif

#endif
