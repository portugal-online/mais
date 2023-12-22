#include "hw_lptim.h"
#include "hw_interrupts.h"
#include <atomic>
#include <cassert>
#include <thread>
#include <unistd.h>
#include "hlog.h"

DECLARE_LOG_TAG(LPTIM)
#define TAG "LPTIM"

static unsigned int tick = 0;
static std::atomic<uint16_t> counter;
static std::atomic<uint16_t> period;
static std::atomic<bool> lptim_int_enabled;
static std::atomic<bool> lptim_run;
volatile bool lptim_exit = false;

static std::thread lptim_thread;

#define RESOLUTION_DEGRADE (10)

extern "C" float get_speedup();

void lptim_engine_raise_interrupt()
{
    raise_interrupt(55);
}

void lptim_thread_runner(void)
{
    uint32_t delta = get_speedup();

    while (!lptim_exit) {
        usleep(tick);

        if (lptim_run) {
            if (counter == 0) {

                // Trigger ssr
                counter = period.load();
                HLOG(TAG, "LPTim underflow int=%d period=%uus", lptim_int_enabled ? 1:0, counter.load()*tick);
                if (lptim_int_enabled) {
                    lptim_engine_raise_interrupt();
                    lptim_run = false; // stop.
                }
            } else {
                if (counter>delta) {
                    counter -= delta;
                } else {
                    counter = 0;
                }
            }
        };
    };
};

void lptim_engine_init(uint32_t divider)
{
    tick = (divider/2) * RESOLUTION_DEGRADE;  // 2MHz

    counter = 0xFFFF;
    period = 0;
    lptim_int_enabled = false;
    lptim_run = false;
    lptim_exit = false;


    if (! lptim_thread.joinable())
    {
        HLOG(TAG, "Starting thread");
        lptim_thread = std::thread( &lptim_thread_runner );
    }
    else
    {
        HWARN(TAG, "Not starting LPTIM, already joinable?");
    }
}

void lptim_engine_deinit(void)
{
    if (lptim_thread.joinable())
    {
        lptim_run = false;
        lptim_exit = true;
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
        Period=get_speedup();

    period = Period;
    counter = Period;

    lptim_run = true;
}

void lptim_engine_stop_it(void)
{
    lptim_int_enabled = false;
    lptim_run = false;
}
