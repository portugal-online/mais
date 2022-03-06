#include "zmod4510.h"
#include <stdlib.h>
#include <stdbool.h>
#include "timeutils.h"
#include <assert.h>

#define ZMOD4XXX_ADDR_PID       (0x00)
#define ZMOD4XXX_ADDR_CONF      (0x20)
#define ZMOD4XXX_ADDR_PROD_DATA (0x26)
#define ZMOD4XXX_ADDR_CMD       (0x93)
#define ZMOD4XXX_ADDR_STATUS    (0x94)
#define ZMOD4XXX_ADDR_TRACKING  (0x3A)

struct zmod4510_model
{
    bool powered;
    int busycount;
    uint8_t mem[256];
};

int zmod4510_master_transmit(void *data, const uint8_t *pData, uint16_t Size)
{
    HERROR("Transmit not supported of size %d", Size);
    return -1;

}

static void zmod4510_process_command(struct zmod4510_model *m)
{
    uint8_t cmd = m->mem[ZMOD4XXX_ADDR_CMD];
    //HLOG("Processing command 0x%02x", cmd);
    switch (cmd) {
    case 0x00:
        // Reset?
        break;
    }
    
}

int zmod4510_master_receive(void *data, uint8_t *pData, uint16_t Size)
{
    HERROR("Receive not supported of size %d", Size);
    return 0;
}

int zmod4510_master_mem_write(void *data,uint16_t memaddress, uint8_t memaddrsize, const uint8_t *pData, uint16_t Size)
{
    struct zmod4510_model *m = (struct zmod4510_model *)data;
    bool checkcommand = false;

    if (memaddrsize!=1) {
        HERROR("Invalid mem addr size %d", memaddrsize);
        return -1;
    }
    while (Size--) {
        m->mem[memaddress] = *pData++;
        if (memaddress==ZMOD4XXX_ADDR_CMD)
            checkcommand=true;
        memaddress++;
    }
    if (checkcommand) {
        zmod4510_process_command(m);
    }
    return 0;
}

int zmod4510_master_mem_read(void *data,uint16_t memaddress, uint8_t memaddrsize, uint8_t *pData, uint16_t Size)
{
    struct zmod4510_model *m = (struct zmod4510_model *)data;

    // Update busy status
    m->mem[ZMOD4XXX_ADDR_STATUS] = m->busycount?0x80:0x00; // Busy
    if (m->busycount>0)
        m->busycount--;


    if (memaddrsize!=1) {
        HERROR("Invalid mem addr size %d", memaddrsize);
        return -1;
    }
    //HLOG("Mem read %04x size %d", memaddress, Size);
    int maxlen = 256 - memaddress;

    memcpy(pData, &m->mem[memaddress], MIN(Size, maxlen));

    return 0;
}

struct i2c_device_ops zmod4510_ops = {
    &zmod4510_master_transmit,
    &zmod4510_master_receive,
    &zmod4510_master_mem_write,
    &zmod4510_master_mem_read
};

struct zmod4510_model *zmod4510_model_new()
{
    struct zmod4510_model *m = (struct zmod4510_model *) malloc(sizeof(struct zmod4510_model));
    m->powered  = false;
    m->busycount = 4; // Start with 4 busy cycles

    m->mem[0] = 0x63;
    m->mem[1] = 0x20;

    const uint8_t pdata[] = { 0xd1 ,0xd3 ,0x39 ,0xe5 ,0x4d ,0x2c ,0xb0 ,0x96 ,0x6c ,0xb5 ,0x2c ,0xdc ,0x41 ,0x1f ,0x01 ,0x00 };
    memcpy(&m->mem[0x20], pdata, sizeof(pdata));

    // Some defaults missing. TBD.

    return m;
}

void zmod4510_powerdown(struct zmod4510_model *m)
{
    HLOG("Powered down");
    m->powered = false;
}

void zmod4510_powerup(struct zmod4510_model *m)
{
    HLOG("Powered up");
    m->powered = true;
}

