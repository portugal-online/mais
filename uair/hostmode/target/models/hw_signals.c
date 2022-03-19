#include "hw_signals.h"

#include <sys/signal.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

void hw_setup_signals( void (*handler)(int) )
{
    struct sigaction act;

    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);


    act.sa_flags = 0;
#ifdef __linux__
    act.sa_restorer = NULL;
#endif

    if (sigaction(SIGUSR1, &act, NULL)<0)
        abort();
}


void hw_activate_signals(void)
{
    sigset_t s;
    sigemptyset(&s);
    sigaddset(&s, SIGUSR1 );
    sigprocmask(SIG_UNBLOCK, &s, NULL);
}
