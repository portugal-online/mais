#include "console_uart.h"
#include <thread>
#include "cqueue.hpp"
#include <termios.h>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include "models/hw_interrupts.h"
#include "models/hw_dma.h"
#include "stm32wlxx_ll_dmamux.h"

static struct termios g_startup_termios;
std::thread g_uart_thread;

static CQueue<int> g_control_queue;
static CQueue<uint8_t> g_data_queue;

std::atomic<bool> g_enabled;

static void restore_termios()
{
    tcsetattr(0, TCSANOW, &g_startup_termios);
}

void console_uart_init()
{
    struct termios i;
    tcgetattr(0, &g_startup_termios);
    memcpy(&i, &g_startup_termios, sizeof(i));

    i.c_lflag &= ~ICANON;

    atexit( &restore_termios );

    tcsetattr(0, TCSANOW, &i);
}

void console_uart_deinit()
{
    if (g_uart_thread.joinable())
    {
        restore_termios();
        g_control_queue.enqueue(-1);
        g_uart_thread.join();
    }
}

static void uart_thread_runner(int dmaline)
{
    uint8_t thischar;
    struct timeval tv;
    fd_set rfs;

    while (1) {
        int i = g_control_queue.dequeue();
        if (i<0)
            return;

        // Read
        bool exitloop = false;
        do {
            tv.tv_sec = 0;
            tv.tv_usec = 500000;
            FD_ZERO(&rfs);
            FD_SET(0, &rfs);

            switch (select(1, &rfs, NULL, NULL, &tv)) {
            case -1:
                return;
            case 0:
                if (!g_control_queue.empty()) {
                    exitloop = true;
                }
                break;
            default:
                read(0, &thischar, 1);
                g_data_queue.enqueue(thischar);
                dma_notify(dmaline);
                break;
            }
        } while (!exitloop);
    }
}

int console_uart_get_char()
{

    return g_data_queue.dequeue();
}

int console_uart_start_dma()
{
    if (!g_uart_thread.joinable()) {
        console_uart_init();
        g_uart_thread = std::thread( &uart_thread_runner, LL_DMAMUX_REQ_USART2_RX );
    }
    g_control_queue.enqueue(1);
    return 0;
}

