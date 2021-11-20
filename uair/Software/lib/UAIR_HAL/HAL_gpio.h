#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include <inttypes.h>
#include "stm32wlxx_hal.h"
#include "HAL_clk.h"

typedef struct  {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint16_t af;
    HAL_clk_clock_control_fun_t clock_control;
} HAL_GPIODef_t;

typedef HAL_GPIODef_t *HAL_GPIO_t;

HAL_StatusTypeDef HAL_GPIO_configure_output_pp(const HAL_GPIODef_t *gpio);
HAL_StatusTypeDef HAL_GPIO_configure_output_od(const HAL_GPIODef_t *gpio);
HAL_StatusTypeDef HAL_GPIO_configure_input(const HAL_GPIODef_t *gpio);
HAL_StatusTypeDef HAL_GPIO_set(const HAL_GPIODef_t *gpio, int value);
int HAL_GPIO_get(const HAL_GPIODef_t *gpio);

static inline HAL_StatusTypeDef HAL_GPIO_write(const HAL_GPIODef_t *gpio, int value)
{
    return HAL_GPIO_set(gpio, value);
}
static inline int HAL_GPIO_read(const HAL_GPIODef_t *gpio) {
    return HAL_GPIO_get(gpio);
}


#endif
