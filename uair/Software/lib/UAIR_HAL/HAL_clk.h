#ifndef __HAL_CLH_H__
#define __HAL_CLH_H__

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

#endif
