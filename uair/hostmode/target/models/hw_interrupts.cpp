#include "hw_interrupts.h"
#include "cqueue.hpp"
#include <thread>
#include <atomic>
#include <unistd.h>
#include <inttypes.h>
#include "hw_signals.h"
#include <pthread.h>
#include <signal.h>

static CQueue<int> interrupt_queue;
static CQueue<int> pending_queue;
static std::thread interrupt_thread;
static pthread_t main_thread_id;

extern "C" void interrupt(int signal);

void interrupt_signal_handler(int)
{
    int line = pending_queue.dequeue();
    interrupt(line);
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

