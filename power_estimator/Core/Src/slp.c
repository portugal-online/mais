#include "slp.h"
#include <stdlib.h>
#include <assert.h>

#ifdef SLP_SUPPORT_QUEUE_RX
#include <malloc.h>
#include <string.h>
#endif

#undef SLP_DEBUG_ACTIVE

#ifdef SLP_DEBUG_ACTIVE

#define SLP_DEBUG(slp, x...) do { \
    printf("%s: ", slp->name); \
    printf(x); \
    printf("\n"); \
    } while (0)

#define SLP_ERROR(slp, x...) do { \
    printf("%s ERROR: ", slp->name); \
    printf(x); \
    printf("\n"); \
    } while (0)

#define SLP_DEBUGBUF(slp, tag, data,len) do { \
    printf("%s: ", slp->name); \
    printf("%s", tag); \
    int i; \
    for (i=0;i<len;i++) {\
    printf(" %02x", data[i]); \
    }\
    printf("\n"); \
    } while (0)

#else
#define SLP_DEBUG(slp, x...)
#define SLP_DEBUGBUF(slp, tag, data,len)
#define SLP_ERROR(slp, x...)

#endif

#define ACKDELAY 20


#define SLP_WINDOW_MASK (SLP_MAX_WINDOW_SIZE-1)

#define SLP_FRAME (0x7E)
#define SLP_ESCAPE (0x7D)
#define SLP_ESCAPEXOR  (0x20)

#define SLP_PACKET_NONE (0x00)
#define SLP_PACKET_PREPARING (0x01)
#define SLP_PACKET_SENT (0xFF)


#ifndef SLP_SUPPORT_TX_DMA
static void slp__sendtrailer(slp_t *slp, uint8_t seq);
static void slp__write(slp_t *slp, uint8_t v);
static void slp__writeraw(slp_t*, uint8_t);
#else
static void slp__sendtrailer(slp_t *slp, uint8_t *dest, uint8_t seq);
static uint8_t* slp__write(slp_t *slp, uint8_t *dest, uint8_t v);
#endif
static void slp__ackupto(slp_t *slp, uint8_t seq);
static void slp__reset(slp_t *slp);
static void slp__lock_packet(slp_t *slp);
static void slp__unlock_packet(slp_t *slp);
static void slp__data(slp_t *slp, const uint8_t *d, slp_size_t l);
static uint32_t slp__gettime(slp_t *slp);
static void slp__checktimeouts(slp_t *slp);
static uint8_t *slp__get_packet_data_pointer(slp_t *slp, uint8_t seq);


#ifdef SLP_SUPPORT_QUEUE_RX
static void slp__freepacket(uint8_t *packet)
{
    free(packet);
}
static uint8_t *slp__allocpacket(slp_size_t size)
{
    return malloc(size);
}
#endif

static inline void slp__lock_packet(slp_t *slp)
{
    SLP_LOCK(slp->packet_lock);
}

static inline void slp__unlock_packet(slp_t *slp)
{
    SLP_UNLOCK(slp->packet_lock);
}

void slp__init(slp_t *slp,
               const slp_interface_t *interface,
               void *ifdata,
               void (*data)(void *, const uint8_t *data, slp_size_t size),
               void *ddata)
{
    slp->interface = interface;
    slp->ifdata = ifdata;
    slp->data  = data;
    slp->ddata  = ddata;
    slp->rxpacket_size = 0;
    //memset(slp->packet_data, 0, sizeof(slp->packet_data) );
    SLP_LOCK_INIT( slp->packet_lock );
    slp__reset(slp);
}

static uint8_t *slp__get_packet_data_pointer(slp_t *slp, uint8_t seq)
{
#ifdef SLP_SUPPORT_TX_DMA
    return &slp->packet_data_buf[seq][SLP_HEADER_SIZE];
#else
    return slp->packet_data_buf[seq];
#endif
}

#ifndef SLP_SUPPORT_TX_DMA
static inline void slp__writeraw(slp_t *slp, uint8_t data)
{
    slp->interface->write(slp->ifdata, &data, 1);
}

static void slp__flush(slp_t *slp)
{
    slp->interface->flush(slp->ifdata);
}
#endif

static uint8_t slp__incrementseq(uint8_t s)
{
    s++;
    s &= SLP_WINDOW_MASK;
    return s;
}

uint8_t slp__transmitframesavailable(slp_t *slp)
{
    uint8_t ackinc = slp__incrementseq(slp->ackseq);
    ackinc = slp->txseq - ackinc;

    ackinc &= SLP_WINDOW_MASK;
#if 0
    SLP_DEBUG(slp, "ACKINC %d mask %08x: %d", ackinc, SLP_WINDOW_MASK,
              ((SLP_MAX_WINDOW_SIZE)-1)-ackinc
             );
#endif
    int free = ((SLP_WINDOW_SIZE))-ackinc;
    if (free<0)
        free = 0;

    //SLP_DEBUG(slp, "Free slots %d", free);

    return free;
}

#ifdef SLP_SUPPORT_TX_DMA
void slp__retransmit(slp_t *slp, const uint8_t seq)
{
    uint8_t size = slp->packet_size[seq];
    uint8_t *data = slp__get_packet_data_pointer(slp, seq);

    crc16__reset(&slp->txcrc);

    uint8_t control = 0x80 | (slp->rxseq<<3) | seq;

    // Start at data-2 or -3, depending if we need to escape control sequence
    if (control == SLP_FRAME) {
        data[-3] = SLP_FRAME;
        data[-2] = SLP_ESCAPE;
        data[-1] = control ^ SLP_ESCAPEXOR;
    } else {
        data[-2] = SLP_FRAME;
        data[-1] = control;
    }


    crc16__update(&slp->txcrc, control);

    while (size--) {
        uint8_t v = *data++;
        // Do not compute CRC over escaped chars
        if (v==SLP_ESCAPE) {
            v = *data++;
            size--;
            v^=SLP_ESCAPEXOR;
        }
        crc16__update(&slp->txcrc, v);
    }

    slp__sendtrailer(slp, data, seq);
}

static uint8_t *slp__write(slp_t *slp, uint8_t*dest, uint8_t v)
{
    if ((v==SLP_FRAME) || (v==SLP_ESCAPE)) {
        *dest++ = SLP_ESCAPE;
        v ^= SLP_ESCAPEXOR;
    }
    *dest++ = v;
    return dest;
}

static void slp__sendtrailer(slp_t *slp, uint8_t *dest, uint8_t seq)
{
    uint16_t crc = slp->txcrc;

    dest = slp__write( slp, dest, crc & 0xff);
    dest = slp__write( slp, dest, crc>>8 & 0xff);
    *dest++=SLP_FRAME;

    // Now compute start
    uint8_t *start = slp__get_packet_data_pointer(slp, seq);
    start-=2; // Points to frame or control
    if (*start != SLP_FRAME)
        start--;

    slp->interface->transmit( slp->ifdata, start, dest-start);

    SLP_DEBUG(slp, "Transmitted seq %d size=%d actual_size=%d", seq, slp->packet_size[seq], (int)(dest-start));
    slp->packet_tx_time[seq] = slp->interface->gettime(slp->ifdata);
}

#else
void slp__retransmit(slp_t *slp, const uint8_t seq)
{
    uint8_t size = slp->packet_size[seq];
    const uint8_t *data = slp__get_packet_data_pointer(slp, seq);

    crc16__reset(&slp->txcrc);

    uint8_t control = 0x80 | (slp->rxseq<<3) | seq;

    slp__writeraw(slp, SLP_FRAME);
    crc16__update(&slp->txcrc, control);
    slp__write(slp, control);

    while (size--) {
        uint8_t v = *data++;
        crc16__update(&slp->txcrc, v);
        slp__write(slp, v);
    }

    slp__sendtrailer(slp, seq);
}

static void slp__write(slp_t *slp, uint8_t v)
{
    if ((v==SLP_FRAME) || (v==SLP_ESCAPE)) {
        slp__writeraw(slp, SLP_ESCAPE);//, txcrc);
        v ^= SLP_ESCAPEXOR;
    }
    slp__writeraw(slp, v);
}


static void slp__sendtrailer(slp_t *slp, uint8_t seq)
{
    uint16_t crc = slp->txcrc;

    slp__write( slp, crc & 0xff);
    slp__write( slp, crc>>8 & 0xff);
    slp__writeraw(slp, SLP_FRAME);
    slp__flush(slp);
    SLP_DEBUG(slp, "Transmitted seq %d", seq);
    slp->packet_tx_time[seq] = slp->interface->gettime(slp->ifdata);
}
#endif

void slp__startpacket(slp_t *slp)
{
    crc16__reset(&slp->txcrc);

    slp->packet_status[slp->txseq] = SLP_PACKET_PREPARING;

    uint8_t control = 0x80 | (slp->rxseq<<3) | slp->txseq;
#ifdef SLP_SUPPORT_TX_DMA
    uint8_t *data = slp__get_packet_data_pointer(slp, slp->txseq);
    // Start at data-2 or -3, depending if we need to escape control sequence
    if (control == SLP_FRAME) {
        data[-3] = SLP_FRAME;
        data[-2] = SLP_ESCAPE;
        data[-1] = control ^ SLP_ESCAPEXOR;
    } else {
        data[-2] = SLP_FRAME;
        data[-1] = control;
    }
#else
    slp__writeraw(slp, SLP_FRAME);
    slp__write(slp, control);
#endif

    crc16__update(&slp->txcrc, control);
    slp->packet_size[slp->txseq] = 0;
}

void slp__append(slp_t *slp, uint8_t data)
{
    slp__appendbuf(slp, &data, 1);
}

void slp__appendbuf(slp_t *slp, const uint8_t *data, uint8_t size)
{
    uint8_t *packet = slp__get_packet_data_pointer(slp, slp->txseq);

    uint8_t *datastore = &packet[slp->packet_size[slp->txseq]];

    uint8_t len = size;

    while (len--) {
        uint8_t v = *data++;
#ifndef SLP_SUPPORT_TX_DMA
        *datastore++ = v;
#endif
        crc16__update(&slp->txcrc, v);

        if ((v==SLP_FRAME) || (v==SLP_ESCAPE)) {
#ifndef SLP_SUPPORT_TX_DMA
            slp__writeraw(slp, SLP_ESCAPE);
#else
            *datastore++ = SLP_ESCAPE;
            size++;
#endif
            v ^= SLP_ESCAPEXOR;
        }
#ifndef SLP_SUPPORT_TX_DMA
        slp__writeraw(slp, v);
#else
        *datastore++ = v;
#endif
    }
    // For TX_DMA, we store the size after escaping.
    // For normal ops, we store the original size. This allows for memory savings

    slp->packet_size[slp->txseq]+=size;
}

static void slp_timer_timedout(void *user)
{
    slp_t *slp = (slp_t*)user;
    slp__checktimeouts(slp);
}

void slp__finishpacket(slp_t *slp)
{
    uint8_t *start = slp__get_packet_data_pointer(slp, slp->txseq);
    uint8_t *data = &start[  slp->packet_size[slp->txseq] ];

    slp__sendtrailer(slp, data, slp->txseq);

    slp__lock_packet(slp);
    slp->packet_tx_time[slp->txseq] = slp__gettime(slp) + ACKDELAY;
    slp->packet_status[slp->txseq] = SLP_PACKET_SENT;

    // Start ack timer.
    if (slp->acktimer<0)
        slp->acktimer = slp->interface->addtimer( slp->ifdata,
                                                 ACKDELAY,
                                                 &slp_timer_timedout,
                                                 slp);

    slp__unlock_packet(slp);
    SLP_DEBUG(slp, "Sucessful initial tx of seq %d", slp->txseq);
    slp->txseq = slp__incrementseq(slp->txseq);

}


static uint32_t slp__gettime(slp_t *slp)
{
    return slp->interface->gettime(slp->ifdata);
}

static void slp__checktimeouts(slp_t *slp)
{
    uint8_t nextack;
    uint32_t now = slp__gettime(slp);

    nextack = slp__incrementseq(slp->ackseq);
    slp__lock_packet(slp);
    SLP_DEBUG(slp,"Cheking timeouts next ack %d txseq %d", nextack, slp->txseq);

    while (nextack != slp->txseq) {

        SLP_DEBUG(slp," > %d status %d timeout at %d now %d",
                  nextack,
                  slp->packet_status[nextack],
                  slp->packet_tx_time[nextack] + ACKDELAY, now
                 );
        if (slp->packet_status[nextack]==SLP_PACKET_SENT) {
            if ((slp->packet_tx_time[nextack] + ACKDELAY) < now) {
                SLP_DEBUG(slp,"Retransmit %d expire=%d now=%d", nextack,
                       slp->packet_tx_time[nextack] + ACKDELAY,
                       now

                         );

                slp__unlock_packet(slp);

                slp__retransmit(slp, nextack);
                slp__lock_packet(slp);
                break; // Do not retransmit more than one packet.
            }
        }
        nextack = slp__incrementseq(nextack);
    }

    slp->acktimer = slp->interface->addtimer( slp->ifdata,
                                             ACKDELAY,
                                             &slp_timer_timedout,
                                             slp);

    slp__unlock_packet(slp);
}

#ifndef SLP_SUPPORT_TX_DMA
static void slp__ack(slp_t *slp, uint8_t seq)
{
    crc16__reset(&slp->txcrc);

    SLP_DEBUG(slp,"ACKing packet seq %d", seq);
    uint8_t control = 0x00 | (seq<<3) | slp->txseq;

    slp__writeraw(slp, SLP_FRAME);
    crc16__update(&slp->txcrc, control);

    slp__write(slp, control);


    uint16_t crc = slp->txcrc;
    slp__write( slp, crc& 0xff);
    slp__write( slp, crc>>8 & 0xff);
    slp__writeraw(slp, SLP_FRAME);
    slp__flush(slp);
}
#else

static void slp__ack(slp_t *slp, uint8_t seq)
{
    crc16_t crc;
    uint8_t buf[8];
    uint8_t *ptr = &buf[0];

    crc16__reset(&crc);
    uint8_t control = 0x00 | (seq<<3) | slp->txseq;
    *ptr++ = SLP_FRAME;
    crc16__update(&crc, control);

    ptr = slp__write(slp, ptr, control);


    ptr = slp__write( slp, ptr, crc& 0xff);
    ptr = slp__write( slp, ptr, crc>>8 & 0xff);
    *ptr++=SLP_FRAME;
    slp->interface->transmit( slp->ifdata, buf, ptr-buf);
}
#endif

#ifdef SLP_SUPPORT_QUEUE_RX

static uint8_t slp__seqdistance(uint8_t received, uint8_t expected)
{
    if (received>=expected) {
        return received-expected;
    } else {
        // rx 0 exp 7
        return (received+SLP_MAX_WINDOW_SIZE)-expected;
    }
}
#endif


static void slp__processooopacket(slp_t *slp, uint8_t seq)
{
#ifdef SLP_SUPPORT_QUEUE_RX
    if (slp__seqdistance(seq, slp->rxseq)<=(SLP_WINDOW_SIZE)) {

        if (slp->rxpacketstore[seq].data==NULL)
        {
            SLP_DEBUG(slp,"Storing packet idx %d\n", seq);

            slp->rxpacketstore[seq].data = slp__allocpacket(slp->rxpacket_size-3);
            slp->rxpacketstore[seq].len = slp->rxpacket_size-3;

            memcpy(slp->rxpacketstore[seq].data, &slp->rxpacket_data[1], slp->rxpacket_size-3);
        } else {
            SLP_DEBUG(slp,"Packet idx %d already stored!\n", seq);
        }
    } else {
        SLP_DEBUG(slp,"Storing packet idx %d\n", seq);
    }
#endif
}

static void slp__handlepacket(slp_t *slp)
{
    uint8_t control;
    SLP_DEBUGBUF(slp,"RX", slp->rxpacket_data, slp->rxpacket_size);
    if (slp->rxcrc!=0) {
        // CRC error.
        SLP_ERROR(slp,"CRC error %04x", slp->rxcrc);
    } else {

        control = slp->rxpacket_data[0];
        uint8_t packet_txseq = control & 0x7;
        uint8_t packet_rxseq = (control>>3) & 0x7;

        SLP_DEBUG(slp,"RX packet size %d control=%02x", slp->rxpacket_size, control);

        if (control & 0x80) {
            if (packet_txseq != slp->rxseq) {
                //abort();
                SLP_DEBUG(slp,"Out of order packet seq %d, expecting %d", packet_txseq, slp->rxseq);
                slp__processooopacket(slp, packet_txseq);
                slp__ack(slp, slp->rxseq);
                return;
            }
            // Ack.
            slp->rxseq = slp__incrementseq(slp->rxseq);

#ifdef SLP_SUPPORT_QUEUE_RX

            slp__data( slp, &slp->rxpacket_data[1], slp->rxpacket_size-3 );

            do {
                if (slp->rxpacketstore[slp->rxseq].data != NULL) {

                    SLP_DEBUG(slp,"Have seq %d queued %p", (int)slp->rxseq, slp->rxpacketstore[slp->rxseq].data);

                    slp__data(slp, slp->rxpacketstore[slp->rxseq].data, slp->rxpacketstore[slp->rxseq].len);

                    slp__freepacket(slp->rxpacketstore[slp->rxseq].data);

                    slp->rxpacketstore[slp->rxseq].len = 0;
                    slp->rxpacketstore[slp->rxseq].data = NULL;

                    slp->rxseq = slp__incrementseq(slp->rxseq);
                } else {
                    SLP_DEBUG(slp,"Not queued seq %d ", (int)slp->rxseq);
                    break;
                }
            } while (1);

            slp__ack(slp, slp->rxseq);


#else
            slp__ack(slp, slp->rxseq);
            slp__data(slp, &slp->rxpacket_data[1], slp->rxpacket_size-3 );
#endif
        } else {
            // Control packet.
            slp__ackupto(slp, packet_rxseq);
        }
    }
}

static void slp__data(slp_t *slp, const uint8_t *d, slp_size_t l)
{
    slp->data( slp->ddata, d, l);
}

static uint32_t slp_get_pending_ack_locked(slp_t *slp)
{
    uint32_t next = UINT32_MAX;

    for (uint8_t i=0; i<SLP_MAX_WINDOW_SIZE;i++) {
        if (slp->packet_status[i] == SLP_PACKET_SENT) {
            if (next<slp->packet_tx_time[i])
                next = slp->packet_tx_time[i];
        }
    }
    return next;

}

static void slp__ackupto(slp_t *slp, uint8_t seq)
{
    uint8_t nextack = slp->ackseq;
    SLP_DEBUG(slp,"ACK %d up to %d", slp->ackseq, seq);
    slp__lock_packet(slp);

    if (slp__incrementseq(nextack)!=seq) {
        do {
            nextack = slp__incrementseq(nextack);
            SLP_DEBUG(slp,"ACK seq %d", nextack);
        slp->packet_status[nextack] = SLP_PACKET_NONE;
        } while (slp__incrementseq(nextack)!=seq);
        slp->ackseq = nextack;
    } else {
        slp__lock_packet(slp);
        return;
    }
    // Stop timer.
    if (slp->acktimer >= 0) {
        slp->interface->canceltimer(slp->ifdata, slp->acktimer);
        slp->acktimer = -1;
    }

    uint32_t next = slp_get_pending_ack_locked(slp);
    SLP_DEBUG(slp, "Next packet ack in %d", next);
    slp__unlock_packet(slp);

    if (next!=UINT32_MAX) {
        uint32_t now = slp__gettime(slp);
        if (now>next)
            next = 0;
        else
            next -= now;
        slp->acktimer = slp->interface->addtimer( slp->ifdata,
                                                 next,
                                                 &slp_timer_timedout,
                                                 slp);
    }
}


void slp__datainbuf(slp_t *slp, const uint8_t *buf, unsigned len)
{
    while (len--) {
        slp__datain(slp, *buf);
        buf++;
    }
}

void slp__datain(slp_t *slp, uint8_t value)
{
    //SLP_DEBUG(slp,"Datain %02x", value );
    do {
        if (slp->frame) {
            if (slp->escape) {
                value ^= SLP_ESCAPEXOR;
                slp->escape = false;
            } else {
                if (value==SLP_FRAME) {
                    // process
                    if (slp->rxpacket_size>=2)
                        slp__handlepacket(slp);

                    slp->rxpacket_size = 0;
                    slp->frame = true;
                    slp->escape = false;
                    crc16__reset(&slp->rxcrc);

                    break;
                } else if (value==SLP_ESCAPE) {
                    slp->escape = true;
                    break;
                }
            }
            // Append data
            crc16__update(&slp->rxcrc,value);
            slp->rxpacket_data[slp->rxpacket_size++] = value;

        } else {
            if (value==SLP_FRAME) {
                slp->rxpacket_size = 0;
                slp->frame = true;
                slp->escape = false;
                crc16__reset(&slp->rxcrc);
            } else {
                abort();
                // grrr
            }
        }
    } while(0);
}

static void slp__reset(slp_t *slp)
{
    slp->rxseq = slp->txseq = 0;
    slp->ackseq = SLP_MAX_WINDOW_SIZE-1;
    slp->escape = false;
    slp->frame = false;
    slp->acktimer = -1;
#ifdef SLP_SUPPORT_QUEUE_RX
    for (int i=0;i<8;i++) {
        slp->rxpacketstore[i].data = NULL;
    }
#endif
}

void slp__transmitpacket(slp_t *slp, const uint8_t *data, uint8_t size)
{
    slp__startpacket(slp);
    slp__appendbuf(slp, data, size);
    slp__finishpacket(slp);
}


