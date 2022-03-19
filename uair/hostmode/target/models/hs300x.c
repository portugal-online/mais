#include "hs300x.h"
#include <stdlib.h>
#include <stdbool.h>
#include "timeutils.h"
#include <assert.h>
#include <math.h>

struct hs300x_model
{
    enum {
        PROGRAM,
        NORMAL
    } mode;
    uint32_t dummy;
    struct timeval powerup_time;
    bool powered;
    uint8_t rxbuf[16];
    uint16_t hsr;
    uint16_t tsr;
    uint16_t temp;
    uint16_t hum;
    void (*sampling_callback)(void *user, struct hs300x_model*);
    void *sampling_callback_user;
};

#define HS300X_REG_ENTER_PROGRAM_MODE (0xA0)
#define HS300X_REG_LEAVE_PROGRAM_MODE (0x80)
#define HS300X_REG_HSR_READ (0x06)
#define HS300X_REG_HSR_WRITE (0x46)
#define HS300X_REG_TSR_READ (0x11)
#define HS300X_REG_TSR_WRITE (0x51)
#define HS300X_REG_SENSOR_ID0 (0x1E)
#define HS300X_REG_SENSOR_ID1 (0x1F)

#define HS300X_SUCCESS (0x81)

static bool hs300x_can_configure(struct hs300x_model *m);

static void hs300x_put_reg_value(struct hs300x_model *m,  uint16_t value)
{
    m->rxbuf[0] = HS300X_SUCCESS;
    m->rxbuf[1] = value>>8;
    m->rxbuf[2] = value & 0xFF;
}

static int hs300x_write_reg(struct hs300x_model *m, uint8_t reg, uint16_t value)
{
    switch (reg) {
    case HS300X_REG_ENTER_PROGRAM_MODE:
        assert(m->mode==NORMAL);
        assert(hs300x_can_configure(m));
        m->mode = PROGRAM;
        break;
    case HS300X_REG_LEAVE_PROGRAM_MODE:
        assert(m->mode==PROGRAM);
        m->mode = NORMAL;
        break;
    case HS300X_REG_SENSOR_ID0:
        assert(m->mode==PROGRAM);
        hs300x_put_reg_value(m,0xDEAD);
        break;
    case HS300X_REG_SENSOR_ID1:
        assert(m->mode==PROGRAM);
        hs300x_put_reg_value(m,0xBEEF);
        break;
    case HS300X_REG_HSR_READ:
        assert(m->mode==PROGRAM);
        hs300x_put_reg_value(m,m->hsr);
        break;
    case HS300X_REG_TSR_READ:
        assert(m->mode==PROGRAM);
        hs300x_put_reg_value(m,m->tsr);
        break;

    default:
        m->rxbuf[0] = 0x00;
        HERROR("Access to unknown register %d (0x%02x)", reg, reg);
        abort();
    }


    return 0;
}

int hs300x_master_transmit(void *data, const uint8_t *pData, uint16_t Size)
{
    struct hs300x_model *m = (struct hs300x_model *)data;

    if (!m->powered)
        return -1;

    if (Size==3) {
        return hs300x_write_reg(m, pData[0], pData[1] + (((uint16_t)pData[2])<<8));
    } else if (Size==0) {

        if (m->sampling_callback!=NULL)
            m->sampling_callback(m->sampling_callback_user, m);

        // Sample temp+hum
        m->rxbuf[0] = (m->hum)>>8;
        m->rxbuf[1] = (m->hum);
        m->rxbuf[2] = (m->temp)>>8;
        m->rxbuf[3] = (m->temp);
        return 0;
    } else {
        HERROR("Command error len %d", Size);
    }

    return -1;
}

int hs300x_master_receive(void *data, uint8_t *pData, uint16_t Size)
{
    struct hs300x_model *m = (struct hs300x_model *)data;
    if (!m->powered)
        return -1;
    memcpy(pData, m->rxbuf, MIN(Size,sizeof(m->rxbuf)));
    return 0;
}

int hs300x_master_mem_write(void *data,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size)
{
    HERROR("Mem writes not supported");
    return -1;
}

int hs300x_master_mem_read(void *data,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size)
{
    HERROR("Mem reads not supported");
    return -1;
}

struct i2c_device_ops hs300x_ops = {
    &hs300x_master_transmit,
    &hs300x_master_receive,
    &hs300x_master_mem_write,
    &hs300x_master_mem_read
};

struct hs300x_model *hs300x_model_new()
{
    struct hs300x_model *m = (struct hs300x_model *) malloc(sizeof(struct hs300x_model));

    m->powered  = false;
    m->mode = NORMAL;
    m->hsr = 0x5555;
    m->tsr = 0xAAAA;
    m->temp = 0;
    m->hum = 0;
    m->sampling_callback = NULL;
    return m;
}

static bool hs300x_can_configure(struct hs300x_model *m)
{
    struct timeval now, delta;

    if (!m->powered) {
        HWARN("Cannot configure while unpowered");
        return false;
    }

    gettimeofday(&now, NULL);

    timeval_subtract (&delta, &m->powerup_time, &now);

    if ((delta.tv_sec>0) || (delta.tv_usec > 10000000)) { // 10ms
        HWARN("Cannot configure after %ld secs %d usecs", (long)delta.tv_sec, delta.tv_usec);

        return false;
    }

    return true;
}

void hs300x_powerdown(struct hs300x_model *m)
{
    HLOG("Powered down");
    m->powered = false;
}

void hs300x_powerup(struct hs300x_model *m)
{
    HLOG("Powered up");
    m->powered = true;
    gettimeofday(&m->powerup_time, NULL);
}

void hs300x_set_temperature(struct hs300x_model *m, float temp_c)
{
    int32_t x = round(99.290909F*temp_c + 3971.6364);
    m->temp = (x)<<2;
}

void hs300x_set_humidity(struct hs300x_model *m, float hum_percent)
{
    m->hum = round(163.83F*hum_percent);
}

void hs300x_set_sampling_callback(struct hs300x_model *m, void (*callback)(void *user, struct hs300x_model*), void*user)
{
    m->sampling_callback = callback;
    m->sampling_callback_user = user;
}

