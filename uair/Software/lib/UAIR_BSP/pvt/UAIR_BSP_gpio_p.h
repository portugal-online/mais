#ifndef UAIR_BSP_GPIO_P_H__
#define UAIR_BSP_GPIO_P_H__

#include "UAIR_BSP_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t UAIR_BSP_LED_Init(Led_TypeDef Led);
int32_t UAIR_BSP_LED_DeInit(Led_TypeDef Led);

#ifdef __cplusplus
}
#endif

#endif
