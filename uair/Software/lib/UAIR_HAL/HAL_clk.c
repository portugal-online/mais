#include "HAL_clk.h"
#include "stm32wlxx_hal.h"

static uint32_t clockstatus = 0;

#define CLOCKSTATUS_GPIOA (1<<0)
#define CLOCKSTATUS_GPIOB (1<<1)
#define CLOCKSTATUS_GPIOC (1<<2)
#define CLOCKSTATUS_I2C1  (1<<3)
#define CLOCKSTATUS_I2C2  (1<<4)
#define CLOCKSTATUS_I2C3  (1<<5)
#define CLOCKSTATUS_USART1 (1<<6)
#define CLOCKSTATUS_USART2 (1<<7)
#define CLOCKSTATUS_CRC    (1<<8)
#define CLOCKSTATUS_ADC    (1<<9)

static void HAL_clk_updatestatus(unsigned type, int enable)
{
    if (enable) {
        clockstatus |= type;
    } else {
        clockstatus &= ~type;
    }
}


void HAL_clk_resume_clocks(void)
{
    HAL_clk_GPIOA_clock_control(clockstatus & CLOCKSTATUS_GPIOA);
    HAL_clk_GPIOB_clock_control(clockstatus & CLOCKSTATUS_GPIOB);
    HAL_clk_GPIOC_clock_control(clockstatus & CLOCKSTATUS_GPIOC);
    HAL_clk_I2C1_clock_control(clockstatus & CLOCKSTATUS_I2C1);
    HAL_clk_I2C2_clock_control(clockstatus & CLOCKSTATUS_I2C2);
    HAL_clk_I2C3_clock_control(clockstatus & CLOCKSTATUS_I2C3);
    HAL_clk_USART1_clock_control(clockstatus & CLOCKSTATUS_USART1);
    HAL_clk_USART2_clock_control(clockstatus & CLOCKSTATUS_USART2);
<<<<<<< HEAD
    HAL_clk_ADC_clock_control(clockstatus & CLOCKSTATUS_ADC);
=======
    HAL_clk_CRC_clock_control(clockstatus & CLOCKSTATUS_CRC);
>>>>>>> alvie-commissioning
}

uint32_t HAL_clk_get_clock_status(void)
{
    return clockstatus;
}

void HAL_clk_GPIOA_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_GPIOA,enable);
    if (enable)
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else
        __HAL_RCC_GPIOA_CLK_DISABLE();
}

void HAL_clk_GPIOB_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_GPIOB,enable);
    if (enable)
        __HAL_RCC_GPIOB_CLK_ENABLE();
    else
        __HAL_RCC_GPIOB_CLK_DISABLE();
}
void HAL_clk_GPIOC_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_GPIOC,enable);
    if (enable)
        __HAL_RCC_GPIOC_CLK_ENABLE();
    else
        __HAL_RCC_GPIOC_CLK_DISABLE();
}

void HAL_clk_I2C1_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_I2C1,enable);
    if (enable)
        __HAL_RCC_I2C1_CLK_ENABLE();
    else
        __HAL_RCC_I2C1_CLK_DISABLE();
}

void HAL_clk_I2C2_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_I2C2,enable);
    if (enable)
        __HAL_RCC_I2C2_CLK_ENABLE();
    else
        __HAL_RCC_I2C2_CLK_DISABLE();
}

void HAL_clk_I2C3_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_I2C3,enable);
    if (enable)
        __HAL_RCC_I2C3_CLK_ENABLE();
    else
        __HAL_RCC_I2C3_CLK_DISABLE();
}

void HAL_clk_USART1_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_USART1,enable);
    if (enable)
        __HAL_RCC_USART1_CLK_ENABLE();
    else
        __HAL_RCC_USART1_CLK_DISABLE();
}

void HAL_clk_USART2_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_USART2,enable);

    if (enable)
        __HAL_RCC_USART2_CLK_ENABLE();
    else
        __HAL_RCC_USART2_CLK_DISABLE();
}

void HAL_clk_CRC_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_CRC,enable);

    if (enable)
        __HAL_RCC_CRC_CLK_ENABLE();
    else
        __HAL_RCC_CRC_CLK_DISABLE();
}

void HAL_clk_ADC_clock_control(int enable)
{
    HAL_clk_updatestatus(CLOCKSTATUS_ADC,enable);

    if (enable)
        __HAL_RCC_ADC_CLK_ENABLE();
    else
        __HAL_RCC_ADC_CLK_DISABLE();
}
