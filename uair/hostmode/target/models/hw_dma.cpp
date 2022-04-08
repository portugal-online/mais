#include "hw_dma.h"
#include "hw_interrupts.h"
#include "cqueue.hpp"
#include <unordered_map>
#include <thread>
#include "stm32wlxx_hal_dma.h"
#include <cassert>

CQueue<int> g_dma_request;
std::thread g_dmathread;

typedef struct {
    int dma_channel;
} dmamux_conf_t;

static dmamux_conf_t dmamux[64];

void dma_mux_configure_request(int dmamuxline, int dmachannel)
{
    dmamux[dmamuxline].dma_channel = dmachannel;
}

struct periph_fun
{
    int (*read)(void *user, size_t location);
    void *user;
};

static std::unordered_map<size_t, periph_fun> g_addressmap;

static size_t g_dummyptr = 0;

size_t dma_alloc_periph_read_request( int (*read)(void *user, size_t location), void *user)
{
    size_t p = g_dummyptr;
    g_dummyptr += 4;

    periph_fun f;
    f.read = read;
    f.user = user;

    g_addressmap[p] = f;

//    printf("Allocated peripheral %08x\n", p);
    return p;
}

void dma_release_periph_read_request( size_t ptr )
{
    g_addressmap.erase(ptr);
}

void dma_notify(int line)
{
    // Get DMAMUX
    int channel = dmamux[line].dma_channel;

    g_dma_request.enqueue(channel);
}

void dma_thread()
{
    while (1)
    {
        int i = g_dma_request.dequeue();

        if (i<0)
            return;

        DMA_Channel_TypeDef *ch = &_dma1channels[i];
        //printf("DMA chan %d source=%08x\n", ch->id, ch->Source);
        // Find MUX request.
        if (ch->Direction != DMA_PERIPH_TO_MEMORY) {
            printf("DMA invalid direction %08x", ch->Direction);
            abort();
        }

        std::unordered_map<size_t, periph_fun>::iterator it = g_addressmap.find( ch->Source );
        if (it == g_addressmap.end()) {
            printf("Cannot find device at %08x", ch->Source);
            abort();
        }
        assert( ch->PeriphDataAlignment == DMA_PDATAALIGN_BYTE);
        assert( ch->MemDataAlignment == DMA_MDATAALIGN_BYTE);
        assert( ch->PeriphInc  == DMA_PINC_DISABLE);
        assert( ch->MemInc  == DMA_MINC_ENABLE);
        assert( ch->Mode  == DMA_CIRCULAR);

        //printf("Will read\n");
        uint32_t val = it->second.read( it->second.user, ch->Source);

        ((uint8_t*)ch->Dest)[ ch->Offset ] = (uint8_t)val;
        ch->Offset++;
        if (ch->Offset >= ch->Len) {
            //abort();
            //DMA complete;
            ch->Offset = 0;
            if (ch->interrupt>0)
                raise_interrupt( ch->interrupt );
        }
        else
        {
            if (ch->Offset == ch->Len/2)
            {
                if (ch->interrupt>0)
                    raise_interrupt( ch->interrupt );
                // DMA half complete
            }
        }
    }
}


int dma_channel_start(DMA_Channel_TypeDef *handle, size_t source, size_t dest, unsigned len)
{
    handle->Source = source;
    handle->Dest = dest;
    handle->Len = len;
    handle->Offset = 0;

    if (!g_dmathread.joinable()) {
        g_dmathread = std::thread(dma_thread);
    }
    return 0;
}
