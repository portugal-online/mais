#include "vm3011.h"
#include <stdlib.h>
#include <stdbool.h>
#include "timeutils.h"
#include <assert.h>

DECLARE_LOG_TAG(VM3011)
#define TAG "VM3011"

#define VM3011_REG_WOS_PGA_GAIN (0x01)

struct vm3011_model
{
    bool powered;
    uint8_t mem[6];
    void (*read_callback)(void *user, struct vm3011_model*);
    void *read_callback_user;
};


i2c_status_t vm3011_master_transmit(void *data, const uint8_t *pData, uint16_t Size)
{
    //struct vm3011_model *m = (struct vm3011_model *)data;
    HERROR(TAG, "Transmit not supported");
    return HAL_I2C_ERROR_AF;
}

i2c_status_t vm3011_master_receive(void *data, uint8_t *pData, uint16_t Size)
{
    HERROR(TAG, "Receive not supported");
    return HAL_I2C_ERROR_AF;
}

i2c_status_t vm3011_master_mem_write(void *data,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size)
{
    struct vm3011_model *m = (struct vm3011_model *)data;

    if (memaddrsize!=1) {
        HERROR(TAG, "Invalid mem addr size %d", memaddrsize);
        return HAL_I2C_ERROR_AF;
    }
    while (Size--) {
        if (memaddress>=sizeof(m->mem)) {
            HERROR(TAG, "Invalid mem addr %d", memaddress);
            abort();
        }
        m->mem[memaddress] = *pData++;
        memaddress++;
    }
    return 0;
}

i2c_status_t vm3011_master_mem_read(void *data,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size)
{
    struct vm3011_model *m = (struct vm3011_model *)data;

    if (memaddrsize!=1) {
        HERROR(TAG, "Invalid mem addr size %d", memaddrsize);
        return HAL_I2C_ERROR_AF;
    }

    if (m->read_callback)
        m->read_callback(m->read_callback_user, m);

    int maxlen = sizeof(m->mem) - memaddress;

    memcpy(pData, &m->mem[memaddress], MIN(Size, maxlen));

    return 0;
}

struct i2c_device_ops vm3011_ops = {
    &vm3011_master_transmit,
    &vm3011_master_receive,
    &vm3011_master_mem_write,
    &vm3011_master_mem_read
};

struct vm3011_model *vm3011_model_new()
{
    struct vm3011_model *m = (struct vm3011_model *) malloc(sizeof(struct vm3011_model));

    m->powered  = false;
    return m;
}

void vm3011_powerdown(struct vm3011_model *m)
{
    HWARN(TAG, "Powered down");
    m->powered = false;
}

void vm3011_powerup(struct vm3011_model *m)
{
    HWARN(TAG, "Powered up");
    m->powered = true;
}

void vm3011_set_gain(struct vm3011_model *m, uint8_t gain)
{
    if (gain>31)
        gain=31;

    m->mem[VM3011_REG_WOS_PGA_GAIN] = gain;
}

void vm3011_set_read_callback(struct vm3011_model *m, void (*callback)(void *user, struct vm3011_model*), void*user)
{
    m->read_callback = callback;
    m->read_callback_user = user;
}

