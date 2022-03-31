#ifndef HAL_CLK_H__
#define HAL_CLK_H__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*HAL_clk_clock_control_fun_t)(int);

void HAL_clk_resume_clocks(void);

void HAL_clk_GPIOA_clock_control(int enable);
void HAL_clk_GPIOB_clock_control(int enable);
void HAL_clk_GPIOC_clock_control(int enable);
void HAL_clk_I2C1_clock_control(int enable);
void HAL_clk_I2C2_clock_control(int enable);
void HAL_clk_I2C3_clock_control(int enable);
void HAL_clk_USART1_clock_control(int enable);
void HAL_clk_USART2_clock_control(int enable);
void HAL_clk_ADC_clock_control(int enable);

uint32_t HAL_clk_get_clock_status(void);

#ifdef __cplusplus
}
#endif

#endif
