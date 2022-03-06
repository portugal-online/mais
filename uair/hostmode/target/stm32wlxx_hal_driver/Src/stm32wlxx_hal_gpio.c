#include "stm32wlxx_hal.h"
#include <stdlib.h>
#include <stdio.h>

static int pin_to_index(uint16_t pin)
{
    int z = __builtin_ctz(pin);
    if (z<0 || z>31) {
        HERROR("Cannot extract pin index from %d, z=%d\n", pin, z);
        abort();
    }
    return z;

}
static const char *portname(GPIO_TypeDef *GPIOx)
{
    if (GPIOx==GPIOA)
        return "GPIOA";
    if (GPIOx==GPIOB)
        return "GPIOB";
    if (GPIOx==GPIOC)
        return "GPIOC";
    return "GPIO?";
}

void HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    unsigned index;
    for (index=0;index<16;index++) {
        if (GPIO_Init->Pin & (1<<index)) {
            GPIOx->def[index].Mode = GPIO_Init->Mode;
        }
    }
}

void HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    unsigned index = pin_to_index(GPIO_Pin);

    if (GPIOx->def[index].ops.read) {
        return GPIOx->def[index].ops.read( GPIOx->def[index].data );
    }
    HWARN("Reading from unmapped GPIO port %s pin %d", portname(GPIOx), index);
    return 0;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    unsigned index;
    for (index=0;index<16;index++) {
        if (!(GPIO_Pin & (1<<index)))
            continue;


        switch (GPIOx->def[index].Mode) {
        case GPIO_MODE_OUTPUT_PP:
        case GPIO_MODE_OUTPUT_OD:
            break;
        default:
            HWARN("Writing %d to non-output GPIO port %s pin %d", PinState, portname(GPIOx), index);
            break;
        }

        if (GPIOx->def[index].ops.write) {
            return GPIOx->def[index].ops.write( GPIOx->def[index].data, PinState );
        } else {
            HWARN("Writing %d to unmapped GPIO port %s pin %d", PinState, portname(GPIOx), index);
        }
    }

}

void HAL_GPIO_EXTI_IRQHandler(uint16_t gpio_pin)
{
    abort();
}
