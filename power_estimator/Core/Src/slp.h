#ifndef SLP_H__
#define SLP_H__

#include <inttypes.h>
#include "crc16.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "slp_glue.h"
//#include "slp_config.h"

#undef SLP_SUPPORT_QUEUE_RX
#undef SLP_SUPPORT_TX_DMA


#ifdef __cplusplus
extern "C" {
#endif

typedef struct slp_interface
{
#ifndef SLP_SUPPORT_TX_DMA
    void (*flush)(void *);
    void (*write)(void *, const uint8_t *data, uint8_t len);
#else
    void (*transmit)(void *, const uint8_t *data, uint8_t len);
#endif
    uint32_t (*gettime)(void*);
    int (*addtimer)(void *, uint32_t interval_ms, void (*callback)(void*), void *userdata);
    void (*canceltimer)(void*, int timer);
} slp_interface_t;


#define SLP_MAX_PACKET_SIZE (255)
#define SLP_MAX_WINDOW_SIZE (8)
#define SLP_WINDOW_SIZE (4)

typedef uint8_t slp_packet_t[SLP_MAX_PACKET_SIZE];
typedef uint8_t slp_size_t;

struct rxpacket {
    uint8_t *data;
    uint8_t len;
};

typedef struct slp
{
    const slp_interface_t *interface;
    void *ifdata;
    void (*data)(void *,const uint8_t *data, slp_size_t size);
    void *ddata;
    char name[8];
    int acktimer;
    slp_packet_t packet_data[SLP_MAX_WINDOW_SIZE];
    uint8_t packet_size[SLP_MAX_WINDOW_SIZE];
    uint8_t packet_status[SLP_MAX_WINDOW_SIZE];
    uint32_t packet_tx_time[SLP_MAX_WINDOW_SIZE];

    slp_packet_t rxpacket_data;
    uint8_t  rxpacket_size;

#ifdef SLP_SUPPORT_QUEUE_RX
    struct rxpacket rxpacketstore[SLP_MAX_WINDOW_SIZE];
#endif
    uint8_t rxseq;
    uint8_t txseq;
    uint8_t ackseq;

    crc16_t rxcrc;
    crc16_t txcrc;

    bool escape;
    bool frame;
    slp_lock_t packet_lock;

} slp_t;


void slp__init(slp_t *slp,
               const slp_interface_t *interface,
               void *ifdata,
               void (*data)(void *, const uint8_t *data, slp_size_t size),
               void *ddata
              );

uint8_t slp__transmitframesavailable(slp_t *slp);
void slp__datain(slp_t *slp, uint8_t value);
void slp__datainbuf(slp_t *slp, const uint8_t *buf, unsigned len);


void slp__startpacket(slp_t *slp);
void slp__append(slp_t *slp, uint8_t data);
void slp__appendbuf(slp_t *slp, const uint8_t *data, uint8_t size);
void slp__finishpacket(slp_t *slp);

#ifdef __cplusplus
}
#endif

#endif

