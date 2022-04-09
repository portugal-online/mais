#include "hw_radio.h"
#include <stdlib.h>
#include "hlog.h"

void hw_radio_init( RadioEvents_t *events )
{
    HLOG("Called");
}

RadioState_t hw_radio_get_status( void )
{
    HLOG("Called");
    return RF_IDLE; //, RF_RX_RUNNING, RF_TX_RUNNING]
}

void hw_radio_set_modem( RadioModems_t modem )
{
    HLOG("Called");
}

void hw_radio_set_channel( uint32_t freq )
{
    HLOG("Called");
}

bool hw_radio_is_channel_free( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime )
{
    HLOG("Called");
    return true;
}

uint32_t hw_radio_random( void )
{
    HLOG("Called");
    return 0;
}

void  hw_radio_set_rx_config( RadioModems_t modem, uint32_t bandwidth,
                             uint32_t datarate, uint8_t coderate,
                             uint32_t bandwidthAfc, uint16_t preambleLen,
                             uint16_t symbTimeout, bool fixLen,
                             uint8_t payloadLen,
                             bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                             bool iqInverted, bool rxContinuous )
{
    HLOG("Called");
}

void hw_radio_set_tx_config( RadioModems_t modem, int8_t power, uint32_t fdev,
                            uint32_t bandwidth, uint32_t datarate,
                            uint8_t coderate, uint16_t preambleLen,
                            bool fixLen, bool crcOn, bool freqHopOn,
                            uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
    HLOG("Called");
}

bool hw_radio_check_rf_frequency( uint32_t frequency )
{
    HLOG("Called");
    return true;
}

uint32_t hw_radio_time_on_air( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{
    HLOG("Called");
    abort();
}

void hw_radio_send ( uint8_t *buffer, uint8_t size )
{
    HLOG("Called");
}

void hw_radio_sleep ( void )
{
    HLOG("Called");
}

void hw_radio_standby ( void )
{
    HLOG("Called");

}
void hw_radio_rx ( uint32_t timeout )
{
    HLOG("Called");
}
void hw_radio_start_cad ( void )
{
    HLOG("Called");
}

void hw_radio_set_tx_continuous_wave ( uint32_t freq, int8_t power, uint16_t time )
{
    HLOG("Called");
}

int16_t hw_radio_rssi ( RadioModems_t modem )
{
    HLOG("Called");
    abort();
}

void hw_radio_write ( uint16_t addr, uint8_t data )
{
    HLOG("Called");
}

uint8_t hw_radio_read ( uint16_t addr )
{
    HLOG("Called");
    abort();
}

void hw_radio_write_registers ( uint16_t addr, uint8_t *buffer, uint8_t size )
{
    HLOG("Called");
}

void hw_radio_read_registers ( uint16_t addr, uint8_t *buffer, uint8_t size )
{
    HLOG("Called");
}

void hw_radio_set_max_payload_length ( RadioModems_t modem, uint8_t max )
{
    HLOG("Called");
}

void hw_radio_set_public_network ( bool enable )
{
    HLOG("Called");
}


uint32_t hw_radio_get_wakeup_time ( void )
{
    HLOG("Called");
    abort();
}

void hw_radio_irq_process ( void )
{
    HLOG("Called");
}

void hw_radio_set_event_notify ( void ( * notify ) ( void ) )
{
    HLOG("Called");
}

void hw_radio_rx_boosted ( uint32_t timeout )
{
    HLOG("Called");
}

void hw_radio_set_rx_duty_cycle( uint32_t rxTime, uint32_t sleepTime )
{
    HLOG("Called");
}

void hw_radio_tx_prbs( void )
{
    HLOG("Called");
}

void hw_radio_tx_cw( int8_t power )
{
    HLOG("Called");
}

int32_t hw_radio_radio_set_rx_generic_config( GenericModems_t modem, RxConfigGeneric_t* config, uint32_t rxContinuous, uint32_t symbTimeout)
{
    HLOG("Called");
    abort();
}

int32_t hw_radio_radio_set_tx_generic_config( GenericModems_t modem, TxConfigGeneric_t* config, int8_t power, uint32_t timeout )
{
    HLOG("Called");
    abort();
}



