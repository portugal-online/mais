#ifndef CONSOLE_UART_H__
#define CONSOLE_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

void console_uart_init();
void console_uart_deinit();
int console_uart_start_dma();
int console_uart_get_char();

#ifdef __cplusplus
}
#endif


#endif
