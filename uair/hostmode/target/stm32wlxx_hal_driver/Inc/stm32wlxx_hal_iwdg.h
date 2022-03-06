#ifndef STM32WLXX_HAL_IWDG_H__
#define STM32WLXX_HAL_IWDG_H__

#include "hal_types.h"
// Host-mode
#define IWDG_PRESCALER_256 (1)
#define IWDG_WINR_WIN (0)

typedef struct
{
    uint32_t Prescaler;
    uint32_t Reload;
    uint32_t Window;
} IWDG_InitTypeDef;

typedef struct {
} IWDG_TypeDef;

typedef struct
{
    IWDG_TypeDef *Instance;
    IWDG_InitTypeDef Init;

} IWDG_HandleTypeDef;

HAL_StatusTypeDef HAL_IWDG_Init(IWDG_HandleTypeDef*);
HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef*);

extern IWDG_TypeDef *IWDG;


#endif
