#include "hw_radio.h"

const struct Radio_s Radio =
{
    .Init = &hw_radio_init,
    .GetStatus = &hw_radio_get_status,
    .SetModem = &hw_radio_set_modem,
    .SetChannel = &hw_radio_set_channel,
    .IsChannelFree = &hw_radio_is_channel_free,
    .Random = &hw_radio_random,
    .SetRxConfig = &hw_radio_set_rx_config,

    .SetTxConfig = &hw_radio_set_tx_config,
    .CheckRfFrequency = &hw_radio_check_rf_frequency,

    .TimeOnAir = &hw_radio_time_on_air,
    .Send  = &hw_radio_send,
    .Sleep = &hw_radio_sleep,
    .Standby = &hw_radio_standby,
    .Rx  = &hw_radio_rx,
    .StartCad = &hw_radio_start_cad,

    .SetTxContinuousWave = &hw_radio_set_tx_continuous_wave,

    .Rssi = &hw_radio_rssi,
    .Write = &hw_radio_write,
    .Read = &hw_radio_read,

    .WriteRegisters = &hw_radio_write_registers,
    .ReadRegisters  = &hw_radio_read_registers,

    .SetMaxPayloadLength = &hw_radio_set_max_payload_length,
    .SetPublicNetwork = &hw_radio_set_public_network,
    .GetWakeupTime = &hw_radio_get_wakeup_time,

    .IrqProcess = &hw_radio_irq_process,

    .SetEventNotify = &hw_radio_set_event_notify,

    .RxBoosted = &hw_radio_rx_boosted,

    .SetRxDutyCycle = &hw_radio_set_rx_duty_cycle,
    .TxPrbs = &hw_radio_tx_prbs,
    .TxCw   = &hw_radio_tx_cw,

    .RadioSetRxGenericConfig = &hw_radio_radio_set_rx_generic_config,
    .RadioSetTxGenericConfig = &hw_radio_radio_set_tx_generic_config
};

