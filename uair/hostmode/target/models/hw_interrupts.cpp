#include "hw_interrupts.h"
#include "cqueue.hpp"
#include <thread>
#include <atomic>
#include <unistd.h>
#include <inttypes.h>
#include "hw_signals.h"
#include <pthread.h>
#include <signal.h>
#include <cassert>
#include <atomic>
#include "hlog.h"

static CQueue<int> interrupt_queue;
static CQueue<int> pending_queue;

static std::thread interrupt_thread;
static pthread_t main_thread_id;

static std::recursive_mutex intmutex;

static std::atomic<bool> interrupts_enabled = true;

extern "C" void interrupt(int signal);

extern "C" void __disable_irq_impl()
{
    std::unique_lock<std::recursive_mutex> lock(intmutex);

    interrupts_enabled = false;
}

extern "C" void __enable_irq_impl()
{
    std::unique_lock<std::recursive_mutex> lock(intmutex);

    bool old = interrupts_enabled.exchange( true );

    if (!old)
    {
        if (!pending_queue.empty())
            pthread_kill(pthread_self(), SIGUSR1);
    }

}

extern "C" int __get_PRIMASK()
{
    return interrupts_enabled? 0x1 : 0x0;
}

extern "C" void __set_PRIMASK_impl(int mask)
{
    if (mask) {
        bool old = interrupts_enabled.exchange( true );
        if (!old) {
            INTERRUPT_LOG("Re-enabling interrupts");
            if (!pending_queue.empty())
                pthread_kill(pthread_self(), SIGUSR1);
        }
    } else {
        interrupts_enabled = false;
    }
}


void interrupt_signal_handler(int)
{
    // We arrive here upon wakeup (SIGUSR1) or post-interrupt.
    INTERRUPT_LOG("CPU wakeup, int enabled=%d", interrupts_enabled?1:0);
    if (interrupts_enabled)
    {
        int line = pending_queue.dequeue();
        interrupt(line);
    }
}


static void interrupt_thread_runner()
{
    while (1) {
        int line = interrupt_queue.dequeue();
        if (line<0)
        {
            return;
        }
        else
        {
            pending_queue.enqueue(line);
            INTERRUPT_LOG("Waking up");
            pthread_kill(main_thread_id, SIGUSR1);
        }
    }
}


extern "C" void deinit_interrupts()
{
    raise_interrupt(-1);
    interrupt_thread.join();
}

extern "C" void init_interrupts()
{
    hw_setup_signals(&interrupt_signal_handler);

    main_thread_id = pthread_self();

    interrupt_thread = std::thread(&interrupt_thread_runner);

    hw_activate_signals();
}

extern "C" void raise_interrupt(int line)
{
    interrupt_queue.enqueue(line);
}

