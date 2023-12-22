#ifndef SERIAL_H__
#define SERIAL_H__

#include <stdbool.h>

void serial_transmit(const uint8_t *buf, unsigned len);
void serial_init(void);
bool serial_available(void);
int serial_read(void);

#endif
