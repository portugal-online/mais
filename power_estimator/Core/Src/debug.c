#include "stm32l1xx_hal.h"
#include "vsnprintf.h"
#include <stdio.h>

extern UART_HandleTypeDef huart2;

void dbgprintf(const char *fmt,...)
{
    char b[256];
    va_list ap;
    va_start(ap, fmt);
    int len = __vsnprintf(b, sizeof(b), fmt, ap);
    va_end(ap);
    HAL_UART_Transmit(&huart2, (uint8_t*)b, len, 10000);
}

void dbgprintf_full(const char *fmt,...)
{
    char b[256];
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(b, sizeof(b), fmt, ap);
    va_end(ap);
    HAL_UART_Transmit(&huart2, (uint8_t*)b, len, 10000);
}

void dbgvprintf(const char *fmt, va_list ap)
{
    char b[256];
    int len = __vsnprintf(b, sizeof(b), fmt, ap);
    HAL_UART_Transmit(&huart2, (uint8_t*)b, len, 10000);
}
