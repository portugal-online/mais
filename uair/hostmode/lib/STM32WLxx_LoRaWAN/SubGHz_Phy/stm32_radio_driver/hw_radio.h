#ifndef HW_RADIO_H__
#define HW_RADIO_H__

#include "radio.h"

#ifdef __cplusplus
extern "C" {
#endif

void hw_radio_init( RadioEvents_t *events );
RadioState_t hw_radio_get_status( void );
void hw_radio_set_modem( RadioModems_t modem );
void hw_radio_set_channel( uint32_t freq );
bool hw_radio_is_channel_free( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime );
uint32_t hw_radio_random( void );
void  hw_radio_set_rx_config( RadioModems_t modem, uint32_t bandwidth,
                             uint32_t datarate, uint8_t coderate,
                             uint32_t bandwidthAfc, uint16_t preambleLen,
                             uint16_t symbTimeout, bool fixLen,
                             uint8_t payloadLen,
                             bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                             bool iqInverted, bool rxContinuous );

void hw_radio_set_tx_config( RadioModems_t modem, int8_t power, uint32_t fdev,
                            uint32_t bandwidth, uint32_t datarate,
                            uint8_t coderate, uint16_t preambleLen,
                            bool fixLen, bool crcOn, bool freqHopOn,
                            uint8_t hopPeriod, bool iqInverted, uint32_t timeout );

bool hw_radio_check_rf_frequency( uint32_t frequency );

uint32_t hw_radio_time_on_air( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn );
void hw_radio_send ( uint8_t *buffer, uint8_t size );
void hw_radio_sleep ( void );
void hw_radio_standby ( void );
void hw_radio_rx ( uint32_t timeout );
void hw_radio_start_cad ( void );
void hw_radio_set_tx_continuous_wave ( uint32_t freq, int8_t power, uint16_t time );
int16_t hw_radio_rssi ( RadioModems_t modem );
void hw_radio_write ( uint16_t addr, uint8_t data );
uint8_t hw_radio_read ( uint16_t addr );
void hw_radio_write_registers ( uint16_t addr, uint8_t *buffer, uint8_t size );
void hw_radio_read_registers ( uint16_t addr, uint8_t *buffer, uint8_t size );
void hw_radio_set_max_payload_length ( RadioModems_t modem, uint8_t max );
void hw_radio_set_public_network ( bool enable );
uint32_t  hw_radio_get_wakeup_time ( void );
void hw_radio_irq_process ( void );
void hw_radio_set_event_notify ( void ( * notify ) ( void ) );
void hw_radio_rx_boosted ( uint32_t timeout );
void hw_radio_set_rx_duty_cycle( uint32_t rxTime, uint32_t sleepTime );
void hw_radio_tx_prbs( void );
void hw_radio_tx_cw( int8_t power );
int32_t hw_radio_radio_set_rx_generic_config( GenericModems_t modem, RxConfigGeneric_t* config, uint32_t rxContinuous, uint32_t symbTimeout);
int32_t hw_radio_radio_set_tx_generic_config( GenericModems_t modem, TxConfigGeneric_t* config, int8_t power, uint32_t timeout );

#ifdef __cplusplus
}
#endif

#endif
