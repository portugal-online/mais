/*
 Based on information from
 http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
 */

#ifndef __CRC16_H__
#define __CRC16_H__

#include <inttypes.h>

typedef uint16_t crc16_t;

static inline void crc16__update(crc16_t *crc, uint8_t data)
{
    data ^= (*crc)&0xff;
    data ^= data << 4;
    (*crc) = ((((uint16_t)data << 8) | (((*crc)>>8)&0xff)) ^ (uint8_t)(data >> 4)
              ^ ((uint16_t)data << 3));
}

static inline void crc16__reset(crc16_t *crc)
{
    *crc = 0xffff;
}

#endif
