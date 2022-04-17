#include "system_linux.h"
#include "hw_rtc.h"
#include "hw_interrupts.h"
#include "cqueue.hpp"
#include <thread>
#include <atomic>
#include <sys/signal.h>
#include <unistd.h>
#include <inttypes.h>
#include "hlog.h"
#include "csignal.hpp"

DECLARE_LOG_TAG(RTC)
#define TAG "RTC"

// RTC Engine

// RTC thread.

static std::atomic<uint32_t> counter(0xFFFFFFFF);
static std::atomic<uint32_t> alarma(0xFFFFFFFF);
static std::atomic<bool> alarma_enabled(false);
static std::atomic<bool> rtc_run(true);
static volatile bool rtc_exit = false;
static std::thread rtc_thread;
static std::thread progress_thread;
static CSignal<uint32_t, uint32_t> timerupdated;

CSignal<uint32_t, uint32_t> &rtc_timer_signal()
{
    return timerupdated;
}


extern "C" float get_speedup();

uint32_t rtc_engine_get_counter()
{
    return counter;
}

uint32_t rtc_engine_get_ticks()
{
    return 0xFFFFFFFFF-counter;
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

void progress_thread_runner(void)
{
    while (!rtc_exit) {
        uint32_t ticks = 0xFFFFFFFF - counter;
        uint64_t elapsed = ticks;

        // Convert ticks to DHMS
//        elapsed *= 1000;
        elapsed >>=10;

        unsigned days = elapsed / 86400;
        unsigned hr = (elapsed/3600) % 24;
        unsigned min= ((elapsed/60))%60;
        unsigned sec= (elapsed%60);

        do_log(TAG, LEVEL_PROGRESS, "","", __LINE__, "Elapsed time: ticks=%lu %dd %02dh:%02dm%02ds", ticks, days,hr,min,sec);

        usleep(1000000);
    }
}


void rtc_thread_runner(void)
{
    uint32_t delta = get_speedup(); // TBD: optimize

    while (!rtc_exit) {
        usleep(1024);
        if (rtc_run) {
            uint32_t old = counter;

            if (counter>delta) {
                counter-=delta;
            } else {
                counter = 0;
            }

            timerupdated.emit(0xFFFFFFFF-old, 0xFFFFFFFF-counter);

            if (old==0) {
                // Trigger ssr
                abort();
            }

            if (alarma_enabled && (old>=alarma) && (counter<=alarma)) {
                rtc_engine_raise_alarma();
            }
        };
    };
};

void rtc_engine_init()
{
    if (rtc_thread.joinable())
    {
        HERROR(TAG, "RTC engine already started!");
        abort();
    }

    counter = 0xFFFFFFFF;
    alarma = 0xFFFFFFFF;
    alarma_enabled = false;
    rtc_run = true;
    rtc_exit = false;

    rtc_thread = std::thread(rtc_thread_runner);
}

void rtc_engine_set_alarm_a(uint32_t s_counter)
{
    alarma = s_counter;
#if 0
    do_log("RTC", LEVEL_PROGRESS, "","", __LINE__, "Alarm %08x now %08x delta %d", s_counter, counter.load(),
           counter.load() - s_counter);
#endif
    alarma_enabled = true;
}

void rtc_engine_set_alarm_a_enable(int enabled)
{
    alarma_enabled = enabled==0?false:true;
}

void rtc_engine_deinit()
{
    if (rtc_thread.joinable())
    {
        rtc_exit = true;
        rtc_thread.join();
    }
}

void rtc_enable_progress()
{
    if (!progress_thread.joinable())
    {
        progress_thread = std::thread(progress_thread_runner);
    }
}
