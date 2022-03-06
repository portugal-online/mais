#include "hw_interrupts.h"
#include "cqueue.hpp"
#include <thread>
#include <atomic>
#include <sys/signal.h>
#include <unistd.h>
#include <inttypes.h>

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
        pending_queue.enqueue(line);
        pthread_kill(main_thread_id, SIGUSR1);
    }
}



extern "C" void init_interrupts()
{
    struct sigaction act;
    sigset_t s;


    act.sa_handler = &interrupt_signal_handler;
    sigemptyset(&act.sa_mask);


    //sigaddset(&act.sa_mask, SIGUSR1 );
    act.sa_flags = 0;//SA_RESETHAND;
    act.sa_restorer = NULL;

    if (sigaction(SIGUSR1, &act, NULL)<0)
        abort();
#if 0
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1 );
    sigprocmask(SIG_BLOCK, &s, NULL);
#endif
    //pthread_sigmask();
    main_thread_id = pthread_self();

    interrupt_thread = std::thread(&interrupt_thread_runner);

    sigemptyset(&s);
    sigaddset(&s, SIGUSR1 );
    sigprocmask(SIG_UNBLOCK, &s, NULL);

}

extern "C" void raise_interrupt(int line)
{
    interrupt_queue.enqueue(line);
}

