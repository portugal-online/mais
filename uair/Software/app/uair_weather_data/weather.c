#include "weather.h"
#include "BSP.h"
#include "uartrx.h"
#include "UAIR_rtc.h"
#include "lora_app.h"

float temp_data[48] = {
    7.7, 8.2,
    8.7, 9.0,
    9.4, 9.5,
    9.6, 9.7,
    9.8, 9.7,
    9.6, 9.6,
    9.5, 9.5,
    9.4, 9.6,
    9.7, 10.2,
    10.6, 10.8,
    12.0, 12.6,
    13.4, 13.6,
    13.8, 14.2,
    14.5, 14.3,
    14.1, 14.1,
    14.1, 13.5,
    12.9, 12.2,
    11.4, 11.0,
    10.5, 10.5,
    10.4, 10.0,
    9.6, 9.3,
    9.0, 8.6,
    8.2, 8.0,
    7.8, 7.7
};

uint8_t hum_data[48] = {
    96, 90,
    87, 81,
    76, 75,
    73, 72,
    72, 73,
    74, 74,
    75, 75,
    76, 76,
    77, 77,
    76, 73,
    71, 69,
    66, 64,
    63, 61,
    59, 62,
    64, 66,
    67, 70,
    73, 77,
    81, 85,
    88, 88,
    89, 89,
    89, 90,
    91, 93,
    95, 95,
    96, 96
};

uint8_t oaq_data[48];
static int32_t rtc_offset;
static bool time_set = false;
static int packets_sent = 0;
static RNG_HandleTypeDef rng;

static int weather_init_rng()
{
    rng.Instance = RNG;
    rng.Init.ClockErrorDetection = 0;

    HAL_StatusTypeDef s = HAL_RNG_Init(&rng);

    return s;
}

static uint32_t weather_random()
{
    uint32_t rand;
    (void)HAL_RNG_GenerateRandomNumber(&rng, &rand);
    return rand;
}

void weather_init()
{
    if (uartrx_init()!=0) {
        APP_PPRINTF("Cannot start UART\r\n");
        return;
    }
    if (weather_init_rng()!=0) {
        APP_PPRINTF("Cannot start RNG\r\n");
        return;
    }


    APP_PPRINTF("System initialized\r\n");
}


static void weather_start()
{
    LoRaWAN_Join();
}

struct weather_data {
    float temp;
    uint8_t hum;
    float max_temp;
    uint8_t max_hum;
    uint8_t avg_sound_level;
    uint8_t max_sound_level;
    uint16_t avg_oaq;
    uint16_t max_oaq;
    float avg_int_temp;
    float max_int_temp;
    uint8_t max_int_hum;
};
static struct weather_data weather_data;

static void weather_dump_weather_values(const struct weather_data *data)
{
    APP_PPRINTF("Computed weather values: \r\n");
    APP_PPRINTF(" Avg Ext. temp  : %.02fC\r\n", data->temp);
    APP_PPRINTF(" Max Ext. temp  : %.02fC\r\n", data->max_temp);
    APP_PPRINTF(" Avg Ext. hum   : %d%%\r\n", data->hum);
    APP_PPRINTF(" Max Ext. hum   : %d%%\r\n", data->max_hum);
    APP_PPRINTF(" Avg sound level: %d\r\n", data->avg_sound_level);
    APP_PPRINTF(" Max sound level: %d\r\n", data->max_sound_level);
    APP_PPRINTF(" Avg OAQ        : %d\r\n", data->avg_oaq);
    APP_PPRINTF(" Max OAQ        : %d\r\n", data->max_oaq);
    APP_PPRINTF(" Avg Int. temp  : %.02fC\r\n", data->avg_int_temp);
    APP_PPRINTF(" Max Int. temp  : %.02fC\r\n", data->max_int_temp);
    APP_PPRINTF(" Max Int. hum   : %d%%\r\n", data->max_int_hum);
}

static void weather_prepare_data(uint8_t time)
{
    weather_data.temp = temp_data[time];
    weather_data.hum = hum_data[time];
    // Generate max/min temp. Max 20%
    weather_data.max_temp = temp_data[time] * (1.0 + (float)(weather_random()&0xff)/(25500/2));

    // Generate max/min hum. Max 12%
    weather_data.max_hum = ((uint32_t)hum_data[time] * (256+(weather_random()&0x1F))) >> 8;
    if (weather_data.max_hum>100)
        weather_data.max_hum=100;

    // Generate sound level
    weather_data.avg_sound_level = weather_random() % 24; // 0-23
    weather_data.max_sound_level = weather_data.avg_sound_level + (weather_random() & 7); // Up to 31

    // Generate OAQ. Since we have no data, use humidity*2 + random
    weather_data.avg_oaq = (int32_t)(hum_data[time]<<1) + ((int8_t)(weather_random() & 0xff))/2; // +-128

    weather_data.max_oaq = weather_data.avg_oaq + (weather_random() & 15);

    // Internal temp

    weather_data.avg_int_temp = 25.0 + ((int8_t)(weather_random() & 0xff))/32;
    weather_data.max_int_temp = weather_data.avg_int_temp + (weather_random() & 7);

    weather_data.max_int_hum = 50 + ((int8_t)(weather_random() & 0xff))/16;
    weather_dump_weather_values(&weather_data);

}

static uint8_t weather_encode_temp(float temp)
{
    int32_t rtemp = ((int32_t)(temp * 4)) + 47;
    APP_PPRINTF("Encode temp %f %d\r\n", temp, rtemp);

    if (rtemp<0)
        rtemp = 0;

    if (rtemp>255)
        rtemp = 255;

    return (uint8_t)(rtemp & 0xff);

}

static uint8_t weather_encode_hum(uint8_t hum)
{
    return hum & 0x7F;
}

static uint8_t weather_restart_detected()
{
    return (weather_random() & 0xFF) == 0xFF;
}

static uint8_t weather_batt_level()
{
    return 98 - (packets_sent/(48*4)); // 1% every 4 days. ~1y batt
}

static uint8_t errors_logged = 1;

static uint8_t weather_errors_logged()
{
    if ((weather_random() & 0xFFF) == 0xFFF) {
        if (errors_logged!=255)
            errors_logged++;
    }
    return errors_logged;
}

static int weather_decode_payload(const uint8_t *payload, unsigned size);

static void weather_fill_payload0(uint8_t *size, uint8_t *buffer, const struct weather_data *data)
{
    uint8_t bptr = 0;

    buffer[bptr++] = 0x3C | // Type 0, health all ones.
        ((data->avg_oaq & 0x100) >> 7) |
        ((data->max_oaq & 0x100)) >> 8;

    buffer[bptr++] = weather_encode_temp(data->temp);
    buffer[bptr++] = ((weather_encode_hum(data->hum))<<1) | ((data->max_sound_level>>4) & 0x1);
    buffer[bptr++] = data->avg_oaq & 0xff;
    buffer[bptr++] = data->max_oaq & 0xff;
    buffer[bptr++] = ((data->max_sound_level& 0xF)<<4) | ((data->avg_sound_level& 0xF));
    buffer[bptr++] = weather_encode_temp(data->max_int_temp);
    buffer[bptr++] = ((weather_encode_hum(data->max_int_hum))<<1) | ((data->avg_sound_level>>4) & 0x1);

    *size = bptr;
}

static void weather_fill_payload1(uint8_t *size, uint8_t *buffer, const struct weather_data *data)
{
    uint8_t bptr;
    weather_fill_payload0(&bptr, buffer, data);
    buffer[0] &= ~0xC0;
    buffer[0] |= 0x40; // Payload type 1

    buffer[bptr++] = (weather_restart_detected()&0x1<<7) | (weather_batt_level() & 0x7F);
    buffer[bptr++] = weather_errors_logged();
    *size = bptr;
}

void weather_fill_payload_data(uint8_t *size, uint8_t *buffer)
{
    uint32_t time = weather_gettime();
    int payload_type = 0;

    // We are only interested in 30-minute slots.
    time = time / (30*60);
    // And in daily offsets
    time %= 48; // 48 half-hours per day

    if ((packets_sent % 48)==1) {
        payload_type = 1;
    }
    APP_PPRINTF("Time slot %d\r\n", time);

    weather_prepare_data(time);

    if (payload_type==0) {
        weather_fill_payload0(size, buffer, &weather_data);
    } else {
        weather_fill_payload1(size, buffer, &weather_data);
    }
    {
        int i;
        APP_PPRINTF("Payload: [");
        for (i=0;i<*size;i++) {
            APP_PPRINTF(" %02x", buffer[i]);
        }
        APP_PPRINTF(" ]\r\n");
    }
    weather_decode_payload(buffer, *size);

    packets_sent ++;
}

void weather_settime(uint32_t seconds)
{
    uint16_t subSeconds;
    uint32_t time = UAIR_RTC_GetTime(&subSeconds);
    rtc_offset = seconds - time;
    if (!time_set) {
        time_set = true;
        weather_start();
    }
}

uint32_t weather_gettime()
{
    uint16_t subSeconds;
    uint32_t time = UAIR_RTC_GetTime(&subSeconds);
    return time + rtc_offset;
}

void weather_setdata(float temps[25], uint8_t hums[25])
{
    int i;
    int source_index = 0;

    for (i=0; i<48; i+=2) {

        temp_data[i] = temps[source_index];
        hum_data[i] = hums[source_index];
        temp_data[i+1] = (temps[source_index] + temps[source_index+1])/2;
        hum_data[i+1] = (hums[source_index] + hums[source_index+1])>>1;
        source_index++;
    }
}

static float weather_decode_temp(uint8_t temp)
{
    return ((float)((int)temp-47))/4;
}

static int weather_decode_payload(const uint8_t *payload, unsigned size)
{
    if (size<8)
        return -1;
    // Check version
    uint8_t version = payload[0] >> 6;

    if (version!=0 && version!=1) {
        return -1;
    }
    // Parse common fields
    uint8_t health = (payload[0] >> 2)& 0xF;
    uint16_t avg_oaq = (((int16_t)payload[0] >> 1)&1) << 8;
    uint16_t max_oaq = (((int16_t)payload[0])&1) << 8;
    uint8_t avg_temp = payload[1];
    uint8_t avg_hum = payload[2]>>1;
    uint8_t max_sound = (payload[2]&1)<<4;
    avg_oaq |= payload[3];
    max_oaq |= payload[4];
    max_sound |= (payload[5]>>4);
    uint8_t avg_sound = (payload[7]&1)<<4;
    avg_sound |= (payload[5] & 0xF);
    uint8_t max_int_temp = payload[6];
    uint8_t max_int_hum = payload[7]>>1;

    APP_PPRINTF("Decoded payload (type %d)\r\n", version);
    APP_PPRINTF(" Health status   : %01x\r\n", health);
    APP_PPRINTF(" Avg. ext. temp  : %.02f\r\n", weather_decode_temp(avg_temp));
    APP_PPRINTF(" Avg. ext. hum   : %d%%\r\n", avg_hum);
    APP_PPRINTF(" Max sound level : %d\r\n", max_sound);
    APP_PPRINTF(" Avg sound level : %d\r\n", avg_sound);
    APP_PPRINTF(" Max OAQ         : %d\r\n", max_oaq);
    APP_PPRINTF(" EPA OAQ         : %d\r\n", avg_oaq);
    APP_PPRINTF(" Max. int. temp  : %.02f\r\n", weather_decode_temp(max_int_temp));
    APP_PPRINTF(" Max. int. hum   : %d%%\r\n", max_int_hum);
    if (version==1) {
        uint8_t restart = payload[8] & 0x80;
        uint8_t bat = payload[8] & 0x7F;
        uint8_t errors = payload[9];
        APP_PPRINTF(" Restart         : %s\r\n", restart? "YES": "NO");
        APP_PPRINTF(" Battery level   : %d%%\r\n", bat);
        APP_PPRINTF(" Errors logged   : %d\r\n", errors);
    }
    return 0;
}
