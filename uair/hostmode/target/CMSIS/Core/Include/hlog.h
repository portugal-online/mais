#ifndef HLOG_H__
#define HLOG_H__

#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LEVEL_DEBUG,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_PROGRESS,
    LEVEL_NONE
} log_level_t;

typedef const char *log_zone_t;

extern void do_log(log_zone_t zone, log_level_t level, const char *fun, const char *filename, int line, const char *fmt, ...);

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOGSTREAM stdout

#define HERROR(zone, x...) do_log( zone, LEVEL_ERROR, __FUNCTION__, __FILENAME__, __LINE__, x)

#define HWARN(zone, x...) do_log( zone, LEVEL_WARN, __FUNCTION__, __FILENAME__, __LINE__, x)

#define HLOG(zone, x...) do_log( zone, LEVEL_DEBUG, __FUNCTION__, __FILENAME__, __LINE__, x)


#define INTERRUPT_LOG(x...) /* HLOG(x) */

#define CONSTRUCTOR(rtn,lvl)                                       \
        __attribute__((constructor(lvl))) void _STI__##lvl##__##rtn (void)

extern void zone_register(const char *zone);
log_level_t string_to_loglevel(const char *c);
void set_log_level(log_level_t level);
void get_zones(char *dest, size_t max);
int set_zone_log_level(log_zone_t z, log_level_t level);


#define DECLARE_LOG_TAG(x) \
    CONSTRUCTOR(x, 101) { \
    zone_register( #x ) ; \
    }

#ifdef __cplusplus
}
#endif

#endif

