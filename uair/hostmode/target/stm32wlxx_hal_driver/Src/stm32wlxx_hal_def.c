#include "stm32wlxx_hal_def.h"

RTC_TypeDef _rtc = {0};

IWDG_TypeDef _iwdg = {0};

GPIO_TypeDef _gpioa = {0};
GPIO_TypeDef _gpiob = {0};
GPIO_TypeDef _gpioc = {0};

I2C_TypeDef _I2C1 = {0};
I2C_TypeDef _I2C2 = {0};
I2C_TypeDef _I2C3 = {0};

LPTIM_TypeDef _LPTIM1 = {0};
LPTIM_TypeDef _LPTIM2 = {0};
LPTIM_TypeDef _LPTIM3 = {0};

TIM_TypeDef _TIM1 = {0};

ADC_TypeDef _ADC = {0};

SPI_TypeDef _SPI1 = {0};
SPI_TypeDef _SPI2 = {0};

USART_TypeDef _usart1 = {0};
USART_TypeDef _usart2 = {0};

RNG_TypeDef _rng1 = {0};

DMA_TypeDef _dma1 = {0};

DMA_Channel_TypeDef _dma1channels[8] = {
    { .id = 0, .interrupt = -1 },
    { .id = 1, .interrupt = 27 },
    { .id = 2, .interrupt = 28 },
    { .id = 3, .interrupt = 29 },
    { .id = 4, .interrupt = 30 },
    { .id = 5, .interrupt = 31 },
    { .id = 6, .interrupt = 32 },
    { .id = 7, .interrupt = 33 }
};

CRC_TypeDef _crc1 = {0};

void LL_EXTI_EnableIT_0_31(uint32_t v)
{
}
void LL_EXTI_EnableIT_32_63(uint32_t v)
{
}

