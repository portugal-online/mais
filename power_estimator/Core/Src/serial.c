#include "stm32l1xx_hal.h"
#include "serial.h"
#include "debug.h"

#define RX_BUF_SIZE 256

extern UART_HandleTypeDef huart2;

static uint8_t rxbuf[RX_BUF_SIZE];
static uint8_t dmabuf[2];
static uint8_t wptr, rptr;

void serial_transmit(const uint8_t *buf, unsigned len)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, 10000);
}

void serial_init()
{
    wptr=0;
    rptr=0;

    HAL_UART_Receive_DMA(&huart2, dmabuf, 2);
}

bool serial_available()
{
    return (rptr!=wptr);
}

int serial_read(void)
{
    int r = -1;
    if (!serial_available())
        return r;
    r = rxbuf[rptr];
    rptr++;
    return r;
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    rxbuf[wptr] = dmabuf[0];
    wptr++;
    if (wptr==rptr)
        rptr++;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    rxbuf[wptr] = dmabuf[1];
    wptr++;
    if (wptr==rptr)
        rptr++;
}
