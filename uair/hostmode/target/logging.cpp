#include <unordered_map>
#include "hlog.h"
#include <stdarg.h>
#include <vector>
#include <algorithm>
#include <string>

struct mycomp
{
    inline bool operator() (const char *a, const char *b)
    {
        return strcmp(a,b) <0 ? true:false;
    }
};


static log_level_t log_all_zones = LEVEL_DEBUG;

typedef std::unordered_map<std::string, log_level_t> zone_map_t;

static zone_map_t *log_zones = nullptr;

static FILE *host_log_file = stdout;
static bool silent_progress = false;


extern "C" {

    void get_zones(char *dest, size_t max)
    {
        char *p = dest;
        *p = '\0';

        std::vector<std::string> zones;

        for (auto i: *log_zones) {
            zones.push_back(i.first);
        }

        std::sort(zones.begin(), zones.end());

        for (auto i: zones) {
            if (p!=dest) {
                *p++=' ';
            }
            p = stpcpy(p, i.c_str());
        }
    }

    log_level_t string_to_loglevel(const char *c)
    {
        if (strcmp(c,"debug")==0)
            return LEVEL_DEBUG;
        if (strcmp(c,"warn")==0)
            return LEVEL_WARN;
        if (strcmp(c,"error")==0)
            return LEVEL_ERROR;
        return LEVEL_NONE;
    }

    void set_log_level(log_level_t level)
    {
        log_all_zones = level;
    }

    int set_zone_log_level(log_zone_t z, log_level_t level)
    {
        zone_map_t::iterator i;

        i = log_zones->find(z);

        if (i==log_zones->end()) {
            fprintf(stderr,"Cannot find zone %s\n", z);
            return -1;
        }

        i->second = level;
        return 0;
    }


    void zone_register(const char *zone)
    {
        if (nullptr==log_zones) {
            log_zones = new zone_map_t();
        }
        (*log_zones)[zone] = LEVEL_NONE;
    }

    static bool zone_log(log_zone_t zone, log_level_t level)
    {
        zone_map_t::const_iterator i = log_zones->find(zone);

        if (i != log_zones->end())
        {
            if (level >= i->second)
            {
                return true;
            }
        }
        return false;
    }
    static int get_color(log_level_t level)
    {
        int color = 36;
        switch (level)
        {
        case LEVEL_ERROR:
            color = 31;
            break;
        case LEVEL_WARN:
            color = 33;
            break;
        case LEVEL_PROGRESS:
            color = 32;
            break;
        default:
            break;
        }
        return color;
    }

    static void do_output(FILE *out, int color, log_zone_t zone, const char *fun, const char *filename, int line, const char *fmt, va_list ap)
    {
        fprintf(out, "\033[%d;1m%s %s: ", color, zone, fun);
        vfprintf(out, fmt, ap);
        if (filename && (filename[0] != '\0'))
        {
            fprintf(out, ", at %s line %d\033[0m\n", filename, line);
        }
        else
        {
            fprintf(out, "\033[0m\n");
        }
    }

    void do_log(log_zone_t zone, log_level_t level, const char *fun, const char *filename, int line, const char *fmt, ...)
    {
        if (
            ((level==LEVEL_ERROR) || (level==LEVEL_PROGRESS))
            || (level >= log_all_zones)
            || zone_log(zone, level) )
        {
            va_list ap;
            int color = get_color(level);

            va_start(ap, fmt);
            do_output(host_log_file, color, zone, fun, filename, line, fmt, ap);
            va_end(ap);

            if (level == LEVEL_PROGRESS)
            {
                if (!silent_progress)
                {
                    va_start(ap, fmt);
                    do_output(stdout, color, zone, fun, filename, line, fmt, ap);
                    va_end(ap);
                }
            }

            va_end(ap);

        }

    }

    FILE *get_host_log_file()
    {
        return host_log_file;
    }

    void set_host_log_file(FILE *f)
    {
        host_log_file =f;
    }
}
