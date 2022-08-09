#ifndef LTC2451_H__
#define LTC2451_H__

void ltc2415_set_callback( void (*complete)(int value) );
int ltc2415_read(void);

#endif
