#include "hw_radio.h"
#include <stdlib.h>
#include "hlog.h"
#include "utilities.h"
#include "cqueue.hpp"
#include <thread>
#include <vector>
#include "hw_interrupts.h"
#include "stm32wlxx_hal_subghz.h"
#include <unistd.h>
#include "cmac.h"
#include "models/network/network.hpp"

DECLARE_LOG_TAG(RADIO)
#define TAG "RADIO"

// Radio buffer seems to be... global.
uint8_t radiobuffer[128];

extern "C" float get_speedup();

static std::thread radio_processing_thread;

const RadioLoRaBandwidths_t Bandwidths[] = { LORA_BW_125, LORA_BW_250, LORA_BW_500 };



typedef struct
{
    uint32_t freq;
    RadioModems_t modem;
    int8_t power;
    uint32_t fdev;
    uint32_t bandwidth;
    uint32_t datarate;
    uint8_t coderate;
    uint16_t preambleLen;
    bool fixLen;
    bool crcOn;
    bool freqHopOn;
    uint8_t hopPeriod;
    bool iqInverted;
    uint32_t timeout;
    RadioEvents_t *events;
} hwradio_t;

static hwradio_t hwradio;

typedef struct {
    enum {
        RADIO_EXIT,
        RADIO_TX,
        RADIO_RX
    } cmd;
    uint32_t rxtimeout;
    std::vector<uint8_t> txdata;
} radio_request_t;

typedef struct {
    enum {
        TX_COMPLETE,
        RX_TIMEOUT,
        RX_COMPLETE
    } resp;
    std::vector<uint8_t> rxdata;
    int16_t rssi;
    int8_t snr;
} radio_response_t;

static CQueue<radio_request_t> hw_radio_requests;
static CQueue<radio_response_t> hw_radio_rx_queue;

static CQueue<radio_response_t> hw_radio_responses;



static void hw_radio_do_tx(const std::vector<uint8_t> &data)
{
    char frame[512];

    uint32_t time = hw_radio_time_on_air( hwradio.modem,
                                         hwradio.bandwidth,
                                         hwradio.datarate,
                                         hwradio.coderate,
                                         hwradio.preambleLen,
                                         hwradio.fixLen,
                                         data.size(),
                                         hwradio.crcOn );


    HLOG(TAG, "TX time on air: %u us", time*1000);

    UplinkPayload *payload = new UplinkPayload(data, time);

    payload->print(frame, sizeof(frame));

    HWARN(TAG,"Transmitting frame: (%d) [%s]", payload->size(), frame);

    usleep((time * 1000) / get_speedup() );

    Network::Uplink( payload );

    radio_response_t r;
    r.resp = radio_response_t::TX_COMPLETE;

    hw_radio_responses.enqueue(r);


    raise_interrupt(66);
}

static void hw_radio_do_rx(uint32_t timeout)
{
    DownlinkPayload *downlink = Network::Downlink( timeout, get_speedup() );

    radio_response_t r;

    if (nullptr==downlink) {
        r.resp = radio_response_t::RX_TIMEOUT;
    } else {
        r.resp = radio_response_t::RX_COMPLETE;
        r.rxdata = downlink->data();
        r.rssi = downlink->rssi();
        r.snr = downlink->snr();

        delete(downlink);
    }
    HWARN(TAG,"Downlink completed, resp=%d", r.resp);

    hw_radio_responses.enqueue(r);

    raise_interrupt(66);
}

void hw_radio_thread_runner()
{
    radio_request_t r;
    do {
        r = hw_radio_requests.dequeue();
        switch (r.cmd) {
        case radio_request_t::RADIO_EXIT:
            return;
        case radio_request_t::RADIO_TX:
            hw_radio_do_tx(r.txdata);
            break;
        case radio_request_t::RADIO_RX:
            hw_radio_do_rx(r.rxtimeout);
            break;
        default:
            break;
        }

    } while (1);
}

void hw_radio_init( RadioEvents_t *events )
{
    HLOG(TAG, "Called");
    hwradio.events = events;
    if (!radio_processing_thread.joinable())
    {
        radio_processing_thread = std::thread(hw_radio_thread_runner);
    }
}

void hw_radio_deinit()
{
    if (radio_processing_thread.joinable())
    {
        radio_request_t r;
        r.cmd = radio_request_t::RADIO_EXIT;
        r.rxtimeout = 0;
        hw_radio_requests.enqueue(r);

        radio_processing_thread.join();
    }
    else
    {
        HWARN(TAG, "HW radio not running!");
    }
}

RadioState_t hw_radio_get_status( void )
{
    HLOG(TAG, "Called");
    return RF_IDLE; //, RF_RX_RUNNING, RF_TX_RUNNING]
}

void hw_radio_set_modem( RadioModems_t modem )
{
    HLOG(TAG, "Called");
}

void hw_radio_set_channel( uint32_t freq )
{
    hwradio.freq = freq;
}

bool hw_radio_is_channel_free( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime )
{
    HLOG(TAG, "Called");
    return true;
}

uint32_t hw_radio_random( void )
{
    HLOG(TAG, "Called");
    return random() & 0xFFFFFFFF;
}

void  hw_radio_set_rx_config( RadioModems_t modem, uint32_t bandwidth,
                             uint32_t datarate, uint8_t coderate,
                             uint32_t bandwidthAfc, uint16_t preambleLen,
                             uint16_t symbTimeout, bool fixLen,
                             uint8_t payloadLen,
                             bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                             bool iqInverted, bool rxContinuous )
{
    hwradio.modem = modem;
    hwradio.bandwidth = bandwidth;
    hwradio.datarate = datarate;
    hwradio.coderate = coderate;
    hwradio.preambleLen = preambleLen;
    hwradio.fixLen = fixLen;
    hwradio.crcOn = crcOn;
    hwradio.freqHopOn = freqHopOn;
    hwradio.hopPeriod = hopPeriod;
    hwradio.iqInverted = iqInverted;
    hwradio.timeout = symbTimeout; // TBD
    HLOG(TAG, "Called");
}

void hw_radio_set_tx_config( RadioModems_t modem, int8_t power, uint32_t fdev,
                            uint32_t bandwidth, uint32_t datarate,
                            uint8_t coderate, uint16_t preambleLen,
                            bool fixLen, bool crcOn, bool freqHopOn,
                            uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
    HLOG(TAG, "Called");
    hwradio.modem = modem;
    hwradio.power = power;
    hwradio.fdev = fdev;
    hwradio.bandwidth = bandwidth;
    hwradio.datarate = datarate;
    hwradio.coderate = coderate;
    hwradio.preambleLen = preambleLen;
    hwradio.fixLen = fixLen;
    hwradio.crcOn = crcOn;
    hwradio.freqHopOn = freqHopOn;
    hwradio.hopPeriod = hopPeriod;
    hwradio.iqInverted = iqInverted;
    hwradio.timeout = timeout;
}

bool hw_radio_check_rf_frequency( uint32_t frequency )
{
    HLOG(TAG, "Called");
    return true;
}

static uint32_t RadioGetLoRaBandwidthInHz( RadioLoRaBandwidths_t bw )
{
    uint32_t bandwidthInHz = 0;

    switch( bw )
    {
    case LORA_BW_007:
        bandwidthInHz = 7812UL;
        break;
    case LORA_BW_010:
        bandwidthInHz = 10417UL;
        break;
    case LORA_BW_015:
        bandwidthInHz = 15625UL;
        break;
    case LORA_BW_020:
        bandwidthInHz = 20833UL;
        break;
    case LORA_BW_031:
        bandwidthInHz = 31250UL;
        break;
    case LORA_BW_041:
        bandwidthInHz = 41667UL;
        break;
    case LORA_BW_062:
        bandwidthInHz = 62500UL;
        break;
    case LORA_BW_125:
        bandwidthInHz = 125000UL;
        break;
    case LORA_BW_250:
        bandwidthInHz = 250000UL;
        break;
    case LORA_BW_500:
        bandwidthInHz = 500000UL;
        break;
    }

    return bandwidthInHz;
}

static uint32_t RadioGetGfskTimeOnAirNumerator( uint32_t datarate, uint8_t coderate,
                                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                                                bool crcOn )
{
    /*
    const RadioAddressComp_t addrComp = RADIO_ADDRESSCOMP_FILT_OFF;
    const uint8_t syncWordLength = 3;

    return ( preambleLen << 3 ) +
           ( ( fixLen == false ) ? 8 : 0 ) +
             ( syncWordLength << 3 ) +
             ( ( payloadLen +
               ( addrComp == RADIO_ADDRESSCOMP_FILT_OFF ? 0 : 1 ) +
               ( ( crcOn == true ) ? 2 : 0 )
               ) << 3
             );
    */
    /* ST_WORKAROUND_BEGIN: Simplified calculation without const values */
    return ( preambleLen << 3 ) +
           ( ( fixLen == false ) ? 8 : 0 ) + 24 +
           ( ( payloadLen + ( ( crcOn == true ) ? 2 : 0 ) ) << 3 );
    /* ST_WORKAROUND_END */
}

static uint32_t RadioGetLoRaTimeOnAirNumerator( uint32_t bandwidth,
                                                uint32_t datarate, uint8_t coderate,
                                                uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                                                bool crcOn )
{
    int32_t crDenom           = coderate + 4;
    bool    lowDatareOptimize = false;

    /* Ensure that the preamble length is at least 12 symbols when using SF5 or SF6 */
    if( ( datarate == 5 ) || ( datarate == 6 ) )
    {
        if( preambleLen < 12 )
        {
            preambleLen = 12;
        }
    }

    if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
        ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
    {
        lowDatareOptimize = true;
    }

    int32_t ceilDenominator;
    int32_t ceilNumerator = ( payloadLen << 3 ) +
                            ( crcOn ? 16 : 0 ) -
                            ( 4 * datarate ) +
                            ( fixLen ? 0 : 20 );

    if( datarate <= 6 )
    {
        ceilDenominator = 4 * datarate;
    }
    else
    {
        ceilNumerator += 8;

        if( lowDatareOptimize == true )
        {
            ceilDenominator = 4 * ( datarate - 2 );
        }
        else
        {
            ceilDenominator = 4 * datarate;
        }
    }

    if( ceilNumerator < 0 )
    {
        ceilNumerator = 0;
    }

    // Perform integral ceil()
    int32_t intermediate =
        ( ( ceilNumerator + ceilDenominator - 1 ) / ceilDenominator ) * crDenom + preambleLen + 12;

    if( datarate <= 6 )
    {
        intermediate += 2;
    }

    return ( uint32_t )( ( 4 * intermediate + 1 ) * ( 1 << ( datarate - 2 ) ) );
}


uint32_t hw_radio_time_on_air( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{

    HLOG(TAG, "Time on air: modem %d, bw %d dr %d, coderate %d preamblelen %d, fixlen %d, payloadlen %d crcon %d",
         modem, bandwidth, datarate, coderate, preambleLen, fixLen, payloadLen, crcOn);

    uint32_t numerator = 0;
    uint32_t denominator = 1;

    switch( modem )
    {
    case MODEM_FSK:
        {
            numerator   = 1000U * RadioGetGfskTimeOnAirNumerator( datarate, coderate,
                                                                  preambleLen, fixLen,
                                                                  payloadLen, crcOn );
            denominator = datarate;
        }
        break;
    case MODEM_LORA:
        {
            numerator   = 1000U * RadioGetLoRaTimeOnAirNumerator( bandwidth, datarate,
                                                                  coderate, preambleLen,
                                                                  fixLen, payloadLen, crcOn );
            denominator = RadioGetLoRaBandwidthInHz( Bandwidths[bandwidth] );
        }
        break;
    default:
        break;
    }
    // Perform integral ceil()
    return DIVC(numerator, denominator);
}

void hw_radio_send ( uint8_t *buffer, uint8_t size )
{
    radio_request_t r;
    r.cmd = radio_request_t::RADIO_TX;
    r.txdata = std::vector<uint8_t>(buffer, buffer+size);
    hw_radio_requests.enqueue(r);
    HLOG(TAG, "Called");
}

void hw_radio_sleep ( void )
{
    HLOG(TAG, "Called");
}

void hw_radio_standby ( void )
{
    HLOG(TAG, "Called");

}
void hw_radio_rx ( uint32_t timeout )
{
    radio_request_t r;
    r.cmd = radio_request_t::RADIO_RX;
    r.rxtimeout = timeout;
    hw_radio_requests.enqueue(r);
    HLOG(TAG, "Called");
}
void hw_radio_start_cad ( void )
{
    HLOG(TAG, "Called");
}

void hw_radio_set_tx_continuous_wave ( uint32_t freq, int8_t power, uint16_t time )
{
    HLOG(TAG, "Called");
}

int16_t hw_radio_rssi ( RadioModems_t modem )
{
    HLOG(TAG, "Called");
    abort();
}

void hw_radio_write ( uint16_t addr, uint8_t data )
{
    HLOG(TAG, "Called");
}

uint8_t hw_radio_read ( uint16_t addr )
{
    HLOG(TAG, "Called");
    abort();
}

void hw_radio_write_registers ( uint16_t addr, uint8_t *buffer, uint8_t size )
{
    HLOG(TAG, "Called");
}

void hw_radio_read_registers ( uint16_t addr, uint8_t *buffer, uint8_t size )
{
    HLOG(TAG, "Called");
}

void hw_radio_set_max_payload_length ( RadioModems_t modem, uint8_t max )
{
    HLOG(TAG, "Called");
}

void hw_radio_set_public_network ( bool enable )
{
    HLOG(TAG, "Called");
}


uint32_t hw_radio_get_wakeup_time ( void )
{
    HLOG(TAG, "Called");
    return 10;//abort();
}

void hw_radio_irq_process ( void )
{
    HLOG(TAG, "Called");
}

void hw_radio_set_event_notify ( void ( * notify ) ( void ) )
{
    HLOG(TAG, "Called");
    abort();
}

void hw_radio_rx_boosted ( uint32_t timeout )
{
    HLOG(TAG, "Called");
}

void hw_radio_set_rx_duty_cycle( uint32_t rxTime, uint32_t sleepTime )
{
    HLOG(TAG, "Called");
}

void hw_radio_tx_prbs( void )
{
    HLOG(TAG, "Called");
}

void hw_radio_tx_cw( int8_t power )
{
    HLOG(TAG, "Called");
}

int32_t hw_radio_radio_set_rx_generic_config( GenericModems_t modem, RxConfigGeneric_t* config, uint32_t rxContinuous, uint32_t symbTimeout)
{
    HLOG(TAG, "Called");
    abort();
}

int32_t hw_radio_radio_set_tx_generic_config( GenericModems_t modem, TxConfigGeneric_t* config, int8_t power, uint32_t timeout )
{
    HLOG(TAG, "Called");
    abort();
}

void HAL_SUBGHZ_IRQHandler(SUBGHZ_HandleTypeDef *)
{
    char temp[128];

    radio_response_t r = hw_radio_responses.dequeue();
    switch (r.resp)
    {
    case radio_response_t::TX_COMPLETE:
        hwradio.events->TxDone();
        break;
    case radio_response_t::RX_COMPLETE:
        Network::sprint_buffer(temp, r.rxdata.data(), r.rxdata.size());
        HWARN(TAG, "RxDone : [%s]", temp);
        memcpy(radiobuffer, r.rxdata.data(), r.rxdata.size());
        hwradio.events->RxDone( radiobuffer, r.rxdata.size(), r.rssi, r.snr);
        break;
    case radio_response_t::RX_TIMEOUT:
        hwradio.events->RxTimeout();
        break;
    }
}

