#include "HAL_gpio.h"

HAL_StatusTypeDef HAL_GPIO_configure_output_pp(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_configure_output_od(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_configure_af_od(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_AF_OD;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Alternate = gpio->af;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_configure_input(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_configure_input_pu(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_configure_input_analog(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_ANALOG;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_configure_interrupt_falling_edge(const HAL_GPIODef_t *gpio)
{
    GPIO_InitTypeDef gpio_init_structure = {0};

    gpio_init_structure.Pin = gpio->pin;
    gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(gpio->port, &gpio_init_structure);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_GPIO_set(const HAL_GPIODef_t *gpio, int value)
{
    HAL_GPIO_WritePin(gpio->port, gpio->pin, value);
    return HAL_OK;
}

int HAL_GPIO_get(const HAL_GPIODef_t *gpio)
{
    return HAL_GPIO_ReadPin(gpio->port, gpio->pin);
}
