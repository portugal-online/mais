#ifndef HLOG_H__
#define HLOG_H__

#include <string.h>
#include <stdio.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOGSTREAM stdout

#define HERROR(x...) do {  \
    fprintf(LOGSTREAM, "\033[31;1m%s: ", __FUNCTION__); \
    fprintf(LOGSTREAM, x); \
    fprintf(LOGSTREAM, ", at %s line %d\033[0m\n", __FILENAME__, __LINE__); \
    } while (0) \

#define HWARN(x...) do {  \
    fprintf(LOGSTREAM, "\033[33;1m%s: ", __FUNCTION__); \
    fprintf(LOGSTREAM, x); \
    fprintf(LOGSTREAM, ", at %s line %d\033[0m\n", __FILENAME__, __LINE__); \
    } while (0) \

#define HLOG(x...) do {  \
    fprintf(LOGSTREAM, "\033[36;1m%s: ", __FUNCTION__); \
    fprintf(LOGSTREAM, x); \
    fprintf(LOGSTREAM, ", at %s line %d\033[0m\n", __FILENAME__, __LINE__); \
    } while (0) \


#define INTERRUPT_LOG(x...) /* HLOG(x) */

#endif

