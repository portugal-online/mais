#ifndef UAIR_BSP_POWEZONE_P_H__
#define UAIR_BSP_POWEZONE_P_H__
        
#include "BSP.h"

#ifdef __cplusplus
extern "C" {
#endif

BSP_error_t UAIR_BSP_powerzone_init(void);
BSP_error_t UAIR_BSP_powerzone_deinit(void);
BSP_error_t UAIR_BSP_powerzone_BIT(void);

#ifdef __cplusplus
}
#endif

#endif
