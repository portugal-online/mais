#include "shtc3.h"
#include <stdlib.h>
#include <stdbool.h>
#include "timeutils.h"
#include <assert.h>
#include <math.h>

struct shtc3_model
{
    bool powered;
    bool sleeping;
    uint8_t rxbuf[16];
    uint16_t temp;
    uint16_t hum;
    uint8_t serialptr;
    uint16_t serial[2];
    void (*sampling_callback)(void *user, struct shtc3_model*);
    void *sampling_callback_user;
};

#define SHTC3_CMD_SLEEP 0xB098
#define SHTC3_CMD_WAKEUP 0x3517
#define SHTC3_CMD_READID 0xEFC8

#define SHTC3_CMD_NORMAL_READTFIRST 0x7866
#define SHTC3_CMD_NORMAL_READHFIRST 0x58E0
#define SHTC3_CMD_LOWPOWER_READTFIRST 0x609C
#define SHTC3_CMD_LOWPOWER_READHFIRST 0x401A
#define SHTC3_CMD_SOFTRESET 0x805D

#define SHTC3_CMD_CAPTURE_SERIAL 0xC595
#define SHTC3_CMD_READ_SERIAL 0xC7F7

#define CRC8_POLYNOMIAL 0x31
#define CRC8_INIT 0xFF


uint8_t shtc3_generate_crc(const uint8_t* data, uint16_t count) {
    uint16_t current_byte;
    uint8_t crc = CRC8_INIT;
    uint8_t crc_bit;

    /* calculates 8-Bit checksum with given polynomial */
    for (current_byte = 0; current_byte < count; ++current_byte) {
        crc ^= (data[current_byte]);
        for (crc_bit = 8; crc_bit > 0; --crc_bit) {
            if (crc & 0x80)
                crc = (crc << 1) ^ CRC8_POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

static void shtc3_put_u16(struct shtc3_model*m, uint16_t value)
{
    m->rxbuf[0] = value>>8;
    m->rxbuf[1] = value;
    m->rxbuf[2] = shtc3_generate_crc(m->rxbuf, 2);
}

static void shtc3_put_u16_u16(struct shtc3_model*m, uint16_t value1, uint16_t value2)
{
    m->rxbuf[0] = value1>>8;
    m->rxbuf[1] = value1;
    m->rxbuf[2] = shtc3_generate_crc(m->rxbuf, 2);

    m->rxbuf[3] = value2>>8;
    m->rxbuf[4] = value2;
    m->rxbuf[5] = shtc3_generate_crc(&m->rxbuf[3], 2);
}


int shtc3_master_transmit(void *data, const uint8_t *pData, uint16_t Size)
{
    struct shtc3_model *m = (struct shtc3_model *)data;
    uint8_t crc;

    if (Size<2) {
        HERROR("Transmit not supported of size %d", Size);
        return -1;
    }
    uint16_t cmd = ((uint16_t)pData[0]<<8) | pData[1];
    switch (cmd) {
    case SHTC3_CMD_SLEEP:
        assert(!m->sleeping);
        break;
    case SHTC3_CMD_WAKEUP:
        m->sleeping = false;
        break;
    case SHTC3_CMD_CAPTURE_SERIAL:
        assert(Size==5);
        crc = shtc3_generate_crc(&pData[2], 2);
        if (crc!=pData[4]) {
            HERROR("CRC error");
            abort();
        }
        m->serialptr = 0;
        HLOG("Request serial read");
        break;

    case SHTC3_CMD_READ_SERIAL:
        switch (m->serialptr) {
        case 0:
            shtc3_put_u16(m, m->serial[0]);
            m->serialptr++;
            break;
        case 1:
            shtc3_put_u16(m, m->serial[1]);
            m->serialptr++;
            break;
        default:
            return -1;
        }
        break;
    case SHTC3_CMD_NORMAL_READTFIRST:

        if (m->sampling_callback!=NULL)
            m->sampling_callback(m->sampling_callback_user, m);

        shtc3_put_u16_u16(m, m->temp, m->hum);
        break;
    default:
        HERROR("Unknown command 0x%04x", cmd);
        abort();
        break;
    }
    return 0;
}

int shtc3_master_receive(void *data, uint8_t *pData, uint16_t Size)
{
    struct shtc3_model *m = (struct shtc3_model *)data;
    memcpy(pData, m->rxbuf, MIN(Size, sizeof(m->rxbuf)));
    return 0;
}

int shtc3_master_mem_write(void *,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size)
{
    HERROR("Mem writes not supported");
    return -1;
}

int shtc3_master_mem_read(void *,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size)
{
    HERROR("Mem reads not supported");
    return -1;
}

struct i2c_device_ops shtc3_ops = {
    &shtc3_master_transmit,
    &shtc3_master_receive,
    &shtc3_master_mem_write,
    &shtc3_master_mem_read
};

struct shtc3_model *shtc3_model_new()
{
    struct shtc3_model *m = (struct shtc3_model *) malloc(sizeof(struct shtc3_model));
    m->powered  = false;
    m->sleeping  = false;

    m->serialptr = 2 ; // out of bounds

    m->serial[0] = 0xDEAD;
    m->serial[1] = 0xBEEF;


    return m;
}

void shtc3_powerdown(struct shtc3_model *m)
{
    HLOG("Powered down");
    m->powered = false;
}

void shtc3_powerup(struct shtc3_model *m)
{
    HLOG("Powered up");
    m->powered = true;
}

void shtc3_set_temperature(struct shtc3_model *m, float temp_c)
{
    m->temp = round(374.49143F*temp_c + 16852.114F);
}

void shtc3_set_humidity(struct shtc3_model *m, float hum_percent)
{
    if (hum_percent<0.0F)
        hum_percent=0.0F;
    if (hum_percent>100.0F)
        hum_percent=100.0F;

    m->hum = hum_percent*655.36;
}

void shtc3_set_sampling_callback(struct shtc3_model *m, void (*callback)(void *user, struct shtc3_model*), void*user)
{
    m->sampling_callback = callback;
    m->sampling_callback_user = user;
}

