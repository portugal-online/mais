#include "shtc3.h"
#include <stdlib.h>
#include <stdbool.h>
#include "timeutils.h"
#include <assert.h>
#include <math.h>

#define SHTC3_WAKEUP_TIME_US (240)

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
    i2c_status_t (*command_handler)(uint16_t command, const uint8_t *data, uint16_t len);
    struct timeval measurement_start;
    int measurement_delay;
    struct timeval wakeup_start;
};

void shtc3_set_command_handler( struct shtc3_model *model, i2c_status_t (*handler)(uint16_t command, const uint8_t *data, uint16_t len) )
{
    model->command_handler = handler;

}

static bool shtc3_is_sleeping(struct shtc3_model *m)
{
    if (m->sleeping)
        return true;

    if (time_elapsed_since_exceeds_us(&m->wakeup_start, SHTC3_WAKEUP_TIME_US))
        return false;

    return true;
}

static bool shtc3_is_measuring( struct shtc3_model *model )
{
    if (shtc3_is_sleeping(model))
        return false;

    if (model->measurement_delay<0)
        return false;

    if (time_elapsed_since_exceeds_us(&model->measurement_start, model->measurement_delay))
        return false;

    return true;
}


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


i2c_status_t shtc3_master_transmit(void *data, const uint8_t *pData, uint16_t Size)
{
    struct shtc3_model *m = (struct shtc3_model *)data;
    uint8_t crc;
    i2c_status_t ret = I2C_NORMAL;

    if (Size<2) {
        HERROR("Transmit not supported of size %d", Size);
        return HAL_I2C_ERROR_AF;
    }

    uint16_t cmd = ((uint16_t)pData[0]<<8) | pData[1];


    switch (cmd)
    {
    case SHTC3_CMD_SLEEP:
        if (m->sleeping) {
            HERROR("Received SLEEP command while sleeping");
            ret = HAL_I2C_ERROR_AF;
            break;
        }
        m->sleeping = true;
        break;

    case SHTC3_CMD_WAKEUP:

        m->sleeping = false;
        m->measurement_delay = -1;
        gettimeofday(&m->wakeup_start, NULL);

        break;

    case SHTC3_CMD_CAPTURE_SERIAL:
        if (shtc3_is_measuring(m) || shtc3_is_sleeping(m))
        {
            HERROR("Capturing serial while not ready: measure %s sleep %s",
                   shtc3_is_measuring(m)?"YES":"NO",
                   shtc3_is_sleeping(m) ?"YES":"NO"
                  );
            ret = HAL_I2C_ERROR_AF;
        }
        else
        {
            assert(Size==5);
            crc = shtc3_generate_crc(&pData[2], 2);
            if (crc!=pData[4]) {
                HERROR("CRC error");
                abort();
            }
            m->serialptr = 0;
            HLOG("Request serial read");
        }
        break;
    case SHTC3_CMD_READ_SERIAL:
        if (shtc3_is_measuring(m) || shtc3_is_sleeping(m))
        {
            ret = HAL_I2C_ERROR_AF;
        }
        else
        {
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
                ret = HAL_I2C_ERROR_AF;
            }
        }
        break;

    case SHTC3_CMD_NORMAL_READTFIRST:

        if (shtc3_is_measuring(m) || shtc3_is_sleeping(m))
        {
            ret = HAL_I2C_ERROR_AF;
        }
        else
        {

            if (m->sampling_callback!=NULL)
                m->sampling_callback(m->sampling_callback_user, m);

            shtc3_put_u16_u16(m, m->temp, m->hum);

            gettimeofday(&m->measurement_start, NULL);
            m->measurement_delay = 12100; // 12.1ms
        }
        break;
    default:
        HERROR("Unknown command 0x%04x", cmd);
        //abort();
        ret = HAL_I2C_ERROR_AF;
        break;
    }

    return ret;
}

i2c_status_t shtc3_master_receive(void *data, uint8_t *pData, uint16_t Size)
{
    struct shtc3_model *m = (struct shtc3_model *)data;
    i2c_status_t ret;

    if (shtc3_is_measuring(m) || shtc3_is_sleeping(m))
    {
        ret = HAL_I2C_ERROR_AF;
    }
    else
    {
        memcpy(pData, m->rxbuf, MIN(Size, sizeof(m->rxbuf)));
        ret = I2C_NORMAL;
    }
    return ret;
}

i2c_status_t shtc3_master_mem_write(void *data,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size)
{
    HERROR("Mem writes not supported");
    return HAL_I2C_ERROR_AF;
}

i2c_status_t shtc3_master_mem_read(void *data,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size)
{
    HERROR("Mem reads not supported");
    return HAL_I2C_ERROR_AF;
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
    m->measurement_start.tv_sec = 0UL;
    m->measurement_start.tv_usec = 0UL;
    m->wakeup_start.tv_sec = 0UL;
    m->wakeup_start.tv_usec = 0UL;

    m->serialptr = 2 ; // out of bounds
    m->sampling_callback = NULL;

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
    gettimeofday(&m->wakeup_start, NULL);
}

void shtc3_set_temperature(struct shtc3_model *m, float temp_c)
{
    m->temp = round(374.49143F*temp_c + 16852.114F);
}

void shtc3_set_humidity(struct shtc3_model *m, float hum_percent)
{
    uint32_t val;

    if (hum_percent<0.0F)
        hum_percent=0.0F;

    if (hum_percent>100.0F)
        hum_percent=100.0F;

    val = hum_percent * 655.36;
    if (val>63353)
        val = 65535;

    m->hum = (uint16_t)val;
}

void shtc3_set_sampling_callback(struct shtc3_model *m, void (*callback)(void *user, struct shtc3_model*), void*user)
{
    m->sampling_callback = callback;
    m->sampling_callback_user = user;
}

