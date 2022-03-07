#include "uartrx.h"
#include "BSP.h"
#include "app_conf.h"
#include "stm32_seq.h"
#include <stdlib.h>
#include "weather.h"
#include <sys/time.h>
#include "UAIR_rtc.h"

static uint8_t uartrxbuf[2];
static uint8_t line[1024];
static char procline[1024];
static uint8_t *lptr;

#define UARTRX_SEND(str...) APP_PPRINTF(str "\r\n")

// Time in seconds since midnight
static bool uartrx_settime(const char *time)
{
    unsigned seconds;
    char *end;
    APP_PPRINTF("Convert '%s'\r\n", time);
    seconds = strtoul(time, &end, 10);
    if (end==NULL || *end!='\0')
        return false;
    weather_settime(seconds);
    return true;
}

static bool uartrx_gettime()
{
    time_t time = weather_gettime();

    struct tm *t = localtime(&time);
    APP_PPRINTF("Local time now %d:%d:%d\r\n", t->tm_hour, t->tm_min, t->tm_sec);

    return true;
}

static bool uartrx_setweather(char *cmd)
{
    char *toks[49];

    float temps[25];
    uint8_t hums[25];

    // 24 tokens temp, 24 tokens humidity
    int i = 0;
    toks[i] = strtok(cmd,",");
    do {
        i++;
        toks[i] = strtok(NULL,",");
    } while (toks[i]);
    if (i!=48) {
        APP_PPRINTF("Not enough parameters\r\n");
        return false;
    }
    for (i=0; i<24; i++) {
        char *end;
        temps[i] = strtof(toks[i], &end);
        if ((end==NULL) || (*end!='\0')) {
            APP_PPRINTF("Malformed float '%s'", toks[i]);
            return false;
        }
    }

    for (i=0; i<24; i++) {
        char *end;
        hums[i] = strtoul(toks[i+24], &end, 10);
        if ((end==NULL) || (*end!='\0')) {
            APP_PPRINTF("Malformed int '%s'", toks[i+24]);
            return false;
        }
    }

    temps[24] = temps[0];
    hums[24] = hums[0];

    weather_setdata(temps, hums);

    return true;
}

static void uartrx_atcmd(char *cmd)
{
    bool ok = false;
    do {
        if (*cmd=='\0') {
            ok = true;
            break;
        }
        if (strncmp(cmd,"+TIME=",6)==0) {
            ok = uartrx_settime(&cmd[6]);
            break;
        }
        if (strcmp(cmd,"+TIME")==0) {
            ok = uartrx_gettime();
            break;
        }
        if (strncmp(cmd,"+WEATHER=",9 )==0) {
            ok = uartrx_setweather(&cmd[9]);
            break;
        }
    } while (0);

    UARTRX_SEND(ok?"OK\r\n":"ERROR\r\n");
}

static void uartrx_command(void)
{
    if (procline[0]=='A' && procline[1]=='T') {
        uartrx_atcmd(&procline[2]);
    }
}
int uartrx_init(void)
{

    UART_HandleTypeDef *usart = BSP_get_debug_usart_handle();
    HAL_StatusTypeDef s;

    lptr = &line[0];

    UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_CMD), UTIL_SEQ_RFU, uartrx_command);

    s = HAL_UART_Receive_DMA(usart,
                             uartrxbuf,
                             2);


    return s;
}

static void uartrx_char(const char c)
{
    *lptr = c;
    if (c=='\r' || c=='\n') {
        *lptr = '\0';
        memcpy(procline, line, (lptr-line)+1);
        UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_CMD), CFG_SEQ_Prio_0);

        lptr = &line[0];
    } else {
        if (lptr < &line[sizeof(line)]-1) {
            lptr++;
        }
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *husart)
{
    uartrx_char(uartrxbuf[0]);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *husart)
{
    uartrx_char(uartrxbuf[1]);
}


