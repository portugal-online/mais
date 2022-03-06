#include "vm3011.h"
#include <stdlib.h>
#include <stdbool.h>
#include "timeutils.h"
#include <assert.h>

#define VM3011_REG_WOS_PGA_GAIN (0x01)

struct vm3011_model
{
    bool powered;
    uint8_t mem[6];
};


int vm3011_master_transmit(void *data, const uint8_t *pData, uint16_t Size)
{
    //struct vm3011_model *m = (struct vm3011_model *)data;
    HERROR("Transmit not supported");
    return -1;
}

int vm3011_master_receive(void *data, uint8_t *pData, uint16_t Size)
{
    HERROR("Receive not supported");
    return -1;
}

int vm3011_master_mem_write(void *data,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size)
{
    struct vm3011_model *m = (struct vm3011_model *)data;

    if (memaddrsize!=1) {
        HERROR("Invalid mem addr size %d", memaddrsize);
        return -1;
    }
    while (Size--) {
        if (memaddress>=sizeof(m->mem)) {
            HERROR("Invalid mem addr %d", memaddress);
            abort();
        }
        m->mem[memaddress] = *pData++;
        memaddress++;
    }
    return 0;
}

int vm3011_master_mem_read(void *data,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size)
{
    struct vm3011_model *m = (struct vm3011_model *)data;

    if (memaddrsize!=1) {
        HERROR("Invalid mem addr size %d", memaddrsize);
        return -1;
    }
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
    HLOG("Powered down");
    m->powered = false;
}

void vm3011_powerup(struct vm3011_model *m)
{
    HLOG("Powered up");
    m->powered = true;
}

void vm3011_set_gain(struct vm3011_model *m, uint8_t gain)
{
    if (gain>31)
        gain=31;

    m->mem[VM3011_REG_WOS_PGA_GAIN] = gain;
}

