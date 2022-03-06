#ifndef TIMEUTILS_H__
#define TIMEUTILS_H__

#include <sys/time.h>

static inline int timeval_subtract (struct timeval *result, const struct timeval *x, const struct timeval *y_const)
{
    struct timeval ay;
    ay.tv_sec = y_const->tv_sec;
    ay.tv_usec = y_const->tv_usec;

    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < ay.tv_usec) {
        int nsec = (ay.tv_usec - x->tv_usec) / 1000000 + 1;
        ay.tv_usec -= 1000000 * nsec;
        ay.tv_sec += nsec;
    }
    if (x->tv_usec - ay.tv_usec > 1000000) {
        int nsec = (x->tv_usec - ay.tv_usec) / 1000000;
        ay.tv_usec += 1000000 * nsec;
        ay.tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - ay.tv_sec;
    result->tv_usec = x->tv_usec - ay.tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < ay.tv_sec;
}

static inline void timeval_normalise(struct timeval *tv)
{
    while (tv->tv_usec<-1000000) {
        tv->tv_usec+=1000000;
        tv->tv_sec--;
    }
    while (tv->tv_usec>1000000) {
        tv->tv_usec-=1000000;
        tv->tv_sec++;
    }
}

#endif
