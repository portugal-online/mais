#ifndef WEATHER_H__
#define WEATHER_H__

#include <inttypes.h>

void weather_init(void);
void weather_fill_payload_data(uint8_t *size, uint8_t *buffer);
void weather_settime(uint32_t seconds);
uint32_t weather_gettime(void);
void weather_setdata(float temps[25], uint8_t hums[25]);

#endif
