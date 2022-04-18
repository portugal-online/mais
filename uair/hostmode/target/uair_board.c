#include "stm32wlxx_hal_def.h"
#include <stdlib.h>
#include <stdio.h>
#include "stm32wlxx_hal_i2c_pvt.h"
#include "models/hs300x.h"
#include "models/shtc3.h"
#include "models/zmod4510.h"
#include "models/vm3011.h"
#include "models/hw_rtc.h"
#include "models/hw_lptim.h"
#include "models/hw_interrupts.h"
#include "hw_radio.h" // From lorawan
#include "system_linux.h"
#include "models/console_uart.h"
#include <getopt.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

uint32_t i2c1_power = 0;
uint32_t i2c2_power = 0;
uint32_t i2c3_power = 0;

struct hs300x_model *hs300x;
struct shtc3_model *shtc3;;
struct zmod4510_model *zmod4510;
struct vm3011_model *vm3011;

// Configuration

float arg_internal_temp = 28.21F;
float arg_internal_hum = 53.20F;
float arg_external_temp = 25.43F;
float arg_external_hum = 53.20F;
float speedup = 1.0F;

int arg_mic = 31;
int arg_oaq_samples = -1;
int arg_oaq = -1;

float cycle_cos = -0.5F;
float cycle_angle= M_PI_2;
float delta_angle = M_PI/256.0;
const char *logfile = NULL;


void i2c1_power_control_write(void *user, int val)
{
    i2c1_power = !!val;
    if (!i2c1_power)
        shtc3_powerdown(shtc3);
    else
        shtc3_powerup(shtc3);
}

void i2c2_power_control_write(void *user, int val)
{
    i2c2_power = !!val;
}

void i2c3_power_control_write(void *user, int val)
{
    i2c3_power = !!val;
    if (!i2c3_power) {
        hs300x_powerdown(hs300x);
        zmod4510_powerdown(zmod4510);
    }
    else {
        hs300x_powerup(hs300x);
        zmod4510_powerup(zmod4510);
    }
}

int i2c1_scl_read(void *user)
{
    return i2c1_power;
}

int i2c1_sda_read(void *user)
{
    return i2c1_power;
}

int i2c2_scl_read(void *user)
{
    return i2c2_power;
}

int i2c2_sda_read(void *user)
{
    return i2c2_power;
}

int i2c3_scl_read(void *user)
{
    return i2c3_power;
}

int i2c3_sda_read(void *user)
{
    return i2c3_power;
}

int board_ver_read(void *user)
{
    // R2
    return 0;
}

void gpio_write_ignore(void *user, int value)
{
}

void zmod4510_gpio_reset(void *user, int value)
{
}

float get_generic_float_value(float setting, float min, float max, float error_amplitude)
{
    if (!isinf(setting)) {
        return setting;
    }

    float half_amplitude = (max-min)/2.0;

    float v = min + half_amplitude + (half_amplitude * cycle_cos);

    float half_error_amplitude = error_amplitude / 2;

    v += (error_amplitude * ((random()&0xFFFF)/65536.0F)) - half_error_amplitude;
    return v;
}

float get_internal_temp()
{
    return get_generic_float_value(arg_internal_temp, -4.0F, +42.0F, +2.0F);
}

float get_external_temp()
{
    return get_generic_float_value(arg_external_temp, -4.0F, +42.0F, +2.0F);
}

float get_internal_hum()
{
    float h =get_generic_float_value(arg_internal_hum, 2.0F, +98.0F, +2.0F);
    if (h<0.0)
        h=0.0;
    if (h>100.0)
        h=100.0;
    return h;
}

float get_external_hum()
{
    float h =get_generic_float_value(arg_external_hum, 2.0F, +98.0F, +2.0F);
    if (h<0.0)
        h=0.0;
    if (h>100.0)
        h=100.0;
    return h;
}


static const struct option long_options[] = {
    {"inttemp",     required_argument, 0,  0 },
    {"inthum",      required_argument, 0,  0 },
    {"exttemp",     required_argument, 0,  0 },
    {"exthum",      required_argument, 0,  0 },
    {"mic",         required_argument, 0,  0 },
    {"oaq_samples", required_argument, 0,  0 },
    {"oaq",         required_argument, 0,  0 },
    {"random",      no_argument, 0,  0 },
    {"period",      required_argument, 0,  0 },
    {"help",      no_argument, 0,  0 },
    {"speedup",      required_argument, 0,  0 },
    {"logfile",      required_argument, 0,  0 },
    {"loglevel",      required_argument, 0,  0 },
    {"enable-progress",      no_argument, 0,  0 },
    {"logzone",      required_argument, 0,  0 },
    {0,         0,                 0,  0 }
};

static void help()
{
    char z[512];
    get_zones(z, sizeof(z)-1);

    fputs("\n"
          "Host-mode options: \n"
          "\n"
          "\t--inttemp=random\tUse random for internal temperature\n"
          "\t--inttemp=<float>\tUse specific internal temperature\n"
          "\t--exttemp=random\tUse random for external temperature\n"
          "\t--exttemp=<float>\tUse specific external temperature\n"
          "\t--inthum=random\t\tUse random for internal humidity\n"
          "\t--inthum=<float>\tUse specific internal humidity\n"
          "\t--exthum=random\t\tUse random for external humidity\n"
          "\t--exthum=<float>\tUse specific external humidity\n"
          "\t--random\t\tUse random whenever possible\n"
          "\t--period=<float>\tUse specified random period increment. Default: 0.01227.\n"
          "\n"
          "\t--speedup=<float>\tSpeed up time by this factor\n"
          "\n"
          "\t--logfile=<logfile>\tStore app output in logfile\n"
          "\t--loglevel=<level>\tSet default loglevel for hostmode. level can be debug, warn or error\n"
          "\t--logzone\n"
          "\n"
          "\t--enable-progress\tShow periodic RTC progress\n"
          "\n"
          "The random period is the angle increment for the cosine generator. Defaults to PI/256.\n"
          "Initial angle is PI/2.\n"
          "\n"
          , stdout);
    printf("Available log zones: %s\n\n", z);
    exit (-1);
}

static float parse_float_or_random(const char *what, float min, float max)
{
    char *endp;
    if (strcmp(what,"random")==0)
        return INFINITY;

    float r = strtof(what, &endp);
    if (!endp || (*endp)!='\0') {
        fprintf(stderr, "Invalid value '%s'\n", what);
        exit(-1);
    }
    if (r<min)
        r=min;
    if (r>max)
        r=max;
    return r;
}

static float parse_float(const char *what, float min, float max)
{
    char *endp;
    float r = strtof(what, &endp);
    if (!endp || (*endp)!='\0') {
        fprintf(stderr, "Invalid value '%s'\n", what);
        exit(-1);
    }
    if (r<min)
        r=min;
    if (r>max)
        r=max;
    return r;
}

static int parse_int_or_random(const char *what, int min, int max)
{
    char *endp;

    if (strcmp(what,"random")==0)
        return INT_MIN;

    long r = strtol(what, &endp, 0);
    if (!endp || (*endp)=='\0') {
        fprintf(stderr, "Invalid value '%s'\n", what);
        exit(-1);
    }
    if (r<min)
        r=min;
    if (r>max)
        r=max;
    return r;
}

static int parse_zone(char *c)
{
    log_level_t level = LEVEL_DEBUG;

    char *z = strtok(c,":");
    char *mod = strtok(NULL,":");
    if (mod) {
        level = string_to_loglevel(mod);
        if (level==LEVEL_NONE) {
            fprintf(stderr,"Invalid log level %s\n", mod);
            return -1;
        }
    }
    return set_zone_log_level(z, level);
}

static int parse_log(char *c)
{
    char *zones[16];
    int i = 0;

    zones[i++] = strtok(c, ",");
    while ((zones[i]=strtok(NULL,""))) {
        i++;
    }

    for (i=0; zones[i]; i++) {
        if (parse_zone(zones[i])<0)  {
            fprintf(stderr, "Error handling zone '%s'", zones[i]);
            return -1;
        }
    }
    return 0;
}

static int parse_int(const char *what, int min, int max)
{
    char *endp;

    long r = strtol(what, &endp, 0);
    if (!endp || (*endp)=='\0') {
        fprintf(stderr, "Invalid value '%s'\n", what);
        exit(-1);
    }
    if (r<min)
        r=min;
    if (r>max)
        r=max;
    return r;
}

void bsp_set_hostmode_arguments(int argc, char **argv)
{
    srand((unsigned int)time(NULL));

    log_level_t loglevel;

    while (1)
    {
        int option_index = 0;

        int c = getopt_long_only(argc, argv, "", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0:
            switch (option_index)
            {
            case 0: arg_internal_temp = parse_float_or_random(optarg, -20.0F, +80.0F); break;
            case 1: arg_internal_hum = parse_float_or_random(optarg, 0.0F, 100.0F); break;
            case 2: arg_external_temp = parse_float_or_random(optarg, -20.0F, +80.0F); break;
            case 3: arg_external_hum = parse_float_or_random(optarg, 0.0F, 100.F); break;
            case 4: arg_mic = parse_int_or_random(optarg,0,31); break;
            case 5: arg_oaq_samples = parse_int(optarg, 0, 10000); break;
            case 6: arg_oaq = parse_int(optarg, 0, 500); break;
            case 7: // All random
                arg_internal_temp = INFINITY;
                arg_internal_hum = INFINITY;
                arg_external_temp = INFINITY;
                arg_external_hum = INFINITY;
                break;
            case 8: delta_angle = parse_float(optarg, 0.0F, M_PI); break;
            case 9:
                help();
                break;
            case 10:
                speedup = parse_float(optarg, 1.0F, 200.0F); break;
                break;
            case 11:
                logfile = optarg;
                break;
            case 12:
                loglevel = string_to_loglevel(optarg);
                if (loglevel==LEVEL_NONE) {
                    fprintf(stderr,"Invalid log level %s\n\n", optarg);
                    help();
                }
                else
                {
                    set_log_level(loglevel);
                }
                break;
            case 13:
                rtc_enable_progress();
                break;
            case 14:
                if (parse_log(optarg)!=0)
                    help();
                break;
            }
            break;
        default:
            exit (-1);
        }
    }
}

#ifndef UNITTESTS
static volatile bool sensor_data_thread_exit = false;
static pthread_t sensor_data_thread;

void *sensor_data_thread_runner(void*)
{
    while (!sensor_data_thread_exit) {

        cycle_angle += delta_angle;
        if (cycle_angle> 2*M_PI)
            cycle_angle = 0.0;
        cycle_cos = cos(cycle_angle);

        float exttemp = get_external_temp();
        float exthum = get_external_hum();
        float inttemp = get_internal_temp();
        float inthum = get_internal_hum();

        //printf("*** SET %f %f %f %f\n", exttemp, exthum, inttemp, inthum);

        hs300x_set_temperature(hs300x, exttemp);
        hs300x_set_humidity(hs300x, exthum);

        shtc3_set_temperature(shtc3, inttemp);
        shtc3_set_humidity(shtc3, inthum);

        usleep(1500000/speedup);
    }
    return NULL;
}
#endif

void bsp_preinit()
{
    // Set up output filedes for UART
    if (logfile) {
        USART2->filedes = fopen(logfile, "a");
        if (NULL == USART2->filedes)
        {
            fprintf(stderr,"Cannot open %s: %s", logfile, strerror(errno));
            abort();
        }
        setlinebuf( USART2->filedes );
    } else {
        if (USART2->filedes == NULL)
            USART2->filedes = stdout;
    }

    init_interrupts();

    // Register GPIO handlers

    GPIOA->def[4].ops.read = board_ver_read;

    GPIOB->def[2].ops.write = i2c2_power_control_write; // Mic/I2C2
    GPIOA->def[6].ops.write = i2c3_power_control_write; // Sens/I2C3
    GPIOC->def[2].ops.write = i2c1_power_control_write; //

    GPIOA->def[12].ops.read = i2c2_scl_read;   // Mic
    GPIOA->def[12].ops.write = gpio_write_ignore;
    GPIOA->def[11].ops.read = i2c2_sda_read;
    GPIOA->def[11].ops.write = gpio_write_ignore;

    GPIOB->def[8].ops.read = i2c1_scl_read;
    GPIOB->def[8].ops.write = gpio_write_ignore;
    GPIOA->def[10].ops.read = i2c1_sda_read;
    GPIOA->def[10].ops.write = gpio_write_ignore;

    GPIOC->def[0].ops.read = i2c3_scl_read;
    GPIOC->def[0].ops.write = gpio_write_ignore;

    GPIOC->def[1].ops.read = i2c3_sda_read;
    GPIOC->def[1].ops.write = gpio_write_ignore;

    GPIOC->def[13].ops.write = zmod4510_gpio_reset;

    // Debug/Misc pins
    GPIOA->def[7].ops.write = gpio_write_ignore;
    GPIOA->def[8].ops.write = gpio_write_ignore;
    GPIOA->def[9].ops.write = gpio_write_ignore;
    GPIOB->def[13].ops.write = gpio_write_ignore; // Microphone SCK (ZPL)

    if (hs300x == NULL)
    {
        hs300x = hs300x_model_new();

        i2c_register_device(I2C3, 0x44, &hs300x_ops, hs300x);
    }

    if (shtc3 == NULL)
    {
        shtc3 = shtc3_model_new();

        i2c_register_device(I2C1, 0x70, &shtc3_ops, shtc3);
    }
    if (zmod4510 == NULL)
    {
        zmod4510 = zmod4510_model_new();

        i2c_register_device(I2C3, 0x33, &zmod4510_ops, zmod4510);
    }

    if (vm3011 == NULL)
    {
        vm3011 = vm3011_model_new();

        i2c_register_device(I2C2, 0xC2>>1, &vm3011_ops, vm3011);
    }

    // Set sane defaults
    hs300x_set_temperature(hs300x, 25.43F);
    hs300x_set_humidity(hs300x, 65.30F);

    shtc3_set_temperature(shtc3, 28.21F);
    shtc3_set_humidity(shtc3, 53.20F);

    vm3011_set_gain(vm3011, 31);

    rtc_engine_init();

#ifndef UNITTESTS
    pthread_create(&sensor_data_thread, NULL, &sensor_data_thread_runner, NULL);
#endif
}

static void sensor_data_deinit()
{
#ifndef UNITTESTS
    sensor_data_thread_exit = true;
#endif
}

FILE *uart2_get_filedes()
{
    return USART2->filedes;
}

void uart2_set_filedes(FILE *f)
{
    USART2->filedes = f;
    setlinebuf( USART2->filedes );
}

extern void iwdg_deinit(); // Move to somewhere else

void bsp_deinit()
{
    iwdg_deinit();
    sensor_data_deinit();
    hw_radio_deinit();
    lptim_engine_deinit();
    rtc_engine_deinit();
    deinit_interrupts();
    console_uart_deinit();
}

float get_speedup()
{
    return speedup;
}

void set_speedup(float f)
{
    speedup = f;
}
