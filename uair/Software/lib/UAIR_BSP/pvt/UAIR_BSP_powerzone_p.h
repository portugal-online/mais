#ifndef UAIR_BSP_POWEZONE_P_H__
#define UAIR_BSP_POWEZONE_P_H__
        
#include "BSP.h"

#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t UAIR_BSP_powerzone_init(void);
BSP_error_t UAIR_BSP_powerzone_deinit(void);
BSP_error_t UAIR_BSP_powerzone_BIT(void);
void UAIR_BSP_powerzone_set_operational(BSP_powerzone_t powerzone, bool operational);
void UAIR_BSP_powerzone_disable_internal(BSP_powerzone_t powerzone);
void UAIR_BSP_powerzone_enable_internal(BSP_powerzone_t powerzone);


#ifdef __cplusplus
}
#endif

#endif
