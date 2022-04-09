#include "hw_lptim.h"
#include "hw_interrupts.h"
#include <atomic>
#include <cassert>
#include <thread>
#include <unistd.h>
#include "hlog.h"

static unsigned int tick = 0;

static std::atomic<uint16_t> counter(0xFFFF);
static std::atomic<uint16_t> period;
static std::atomic<bool> lptim_int_enabled(false);
static std::atomic<bool> lptim_run(false);

volatile bool lptim_exit = false;

static std::thread lptim_thread;

#define RESOLUTION_DEGRADE (10)

void lptim_engine_raise_interrupt()
{
    raise_interrupt(55);
}


void lptim_thread_runner(void)
{
    while (!lptim_exit) {
        usleep(tick);

        if (lptim_run) {
            uint32_t old = counter--;
            if (old==0) {
                // Trigger ssr
                counter = period.load();
                HLOG("LPTim underflow int=%d period=%uus", lptim_int_enabled ? 1:0, counter.load()*tick);
                if (lptim_int_enabled) {
                    lptim_engine_raise_interrupt();
                    lptim_run = false; // stop.
                }
            }
        };
    };
};

void lptim_engine_init(uint32_t divider)
{
    tick = (divider/2) * RESOLUTION_DEGRADE;  // 2MHz

    if (! lptim_thread.joinable())
    {
        lptim_thread = std::thread( &lptim_thread_runner );
    }
}

void lptim_engine_deinit(void)
{
    lptim_run = false;
    lptim_exit = true;
    if (lptim_thread.joinable())
    {
        lptim_thread.join();
    }
}

void lptim_engine_enable(void)
{
}
void lptim_engine_start_it(uint32_t Period)
{
    assert(lptim_run == false);
    lptim_int_enabled = true;

    Period/=RESOLUTION_DEGRADE;
    if (Period==0)
        Period=1;

    period = Period;
    counter = Period;

    lptim_run = true;
}

void lptim_engine_stop_it(void)
{
    lptim_int_enabled = false;
    lptim_run = false;
}
