#include "system_linux.h"
#include "hw_rtc.h"
#include "hw_interrupts.h"
#include "cqueue.hpp"
#include <thread>
#include <atomic>
#include <sys/signal.h>
#include <unistd.h>
#include <inttypes.h>

// RTC Engine

// RTC thread.

std::atomic<uint32_t> counter = 0xFFFFFFFF;
std::atomic<uint32_t> alarma = 0xFFFFFFFF;
std::atomic<bool> alarma_enabled;
std::atomic<bool> rtc_run = true;

static std::thread rtc_thread;

uint32_t rtc_engine_get_counter()
{
    return counter;
}

void rtc_engine_enable()
{
    counter = 0xFFFFFFFF;
    alarma_enabled = false;
    rtc_run = true;
}

void rtc_engine_raise_alarma()
{
    raise_interrupt(58);
}


uint32_t rtc_engine_get_second_counter()
{
    return 0;
}

void rtc_thread_runner(void)
{
    while (1) {
        usleep(30*32);
        if (rtc_run) {
            uint32_t old = counter--;
            if (old==0) {
                // Trigger ssr
            }

            if (alarma_enabled && old==alarma) {
                rtc_engine_raise_alarma();
            }
        };
    };
};

void rtc_engine_init()
{
    alarma_enabled = false;
    rtc_thread = std::thread(rtc_thread_runner);
}

void rtc_engine_set_alarm_a(uint32_t counter)
{
    alarma = counter;
    alarma_enabled = true;
}

void rtc_engine_set_alarm_a_enable(int enabled)
{
    alarma_enabled = enabled==0?false:true;
}
