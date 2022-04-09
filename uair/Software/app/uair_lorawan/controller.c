/** Copyright © 2022 MAIS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file controller.c
 *
 *
 */

#include "app.h"
#include "BSP.h"
#include "stm32_timer.h"
#include "stm32_seq.h"
#include "sensors.h"
#include "io/UAIR_io_config.h"
#include "io/UAIR_io_audit.h"
#include "anomaly_guard.h"
#include "lora_app.h"
#include "UAIR_BSP_watchdog.h"

// 75 minutes
#define UAIR_CONSERVATIVE_TX    4500000

#define UTIL_SEQ_RFU 0
//static UTIL_TIMER_Object_t TxTimerTmp;
static UTIL_TIMER_Object_t TxTimer;

#ifdef DEBUGGER_ON
// size is bytes
static void print_binary(uint8_t const size, void const * const ptr) {
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "%u", byte);
        }
    }
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n");
}
#endif


static void OnTxTimerEvent(void *context)
{
    UAIR_BSP_watchdog_kick();
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

    /*Wait for next tx slot*/
    UTIL_TIMER_Start(&TxTimer);
}

/*
static void send_type2(void) { 
    ToDo
}
*/

void cmd_tx_policy(uint32_t value) {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n cmd_tx_policy not implemented yet\r\n");
    return;
}

void cmd_fair_ratio(uint32_t value) { 
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n cmd_fair_ratio not implemented yet\r\n");
    return;
}

void cmd_factory_reset(uint32_t value) { 
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n cmd_factory_reset not implemented yet\r\n");
    return;
}

void cmd_healthchk_ack(uint32_t value) { 
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n cmd_healthchk_ack not implemented yet\r\n");
    return;
}


static void send_type1(void) { 
    uint8_t payload_type = 1;
    uint16_t value;
    sensors_op_result_t res;
    uint8_t UAIR_net_buffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    // 0 | [7:6] | Payload type (00)
    UAIR_net_buffer[0] = (UAIR_net_buffer[0] & ~0x3) | (payload_type & 0x3);
    
    res = UAIR_sensors_read_measure(SENSOR_ID_AIR_QLT, &value);
    // 0 | [2] | OAQ health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 2;
    // 0 | [0] | Max OAQ (worst OAQ) since last report (MSB [8])
    UAIR_net_buffer[0] |= ((value >> 8) & 1) << 0;
    // 3 | [7:0] | EPA OAQ since last report (LSB [7:0])
    UAIR_net_buffer[3] = (UAIR_net_buffer[3] & ~0xff) | (value & 0xff);
    
    res = UAIR_sensors_read_measure(SENSOR_ID_AIR_QLT_MAX, &value);
    // FixMe: already taken? sames as above payload wrong??
    // 0 | [2] | OAQ health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 2;
    // 0 | [1] | EPA OAQ since last report (MSB [8])
    UAIR_net_buffer[0] |= ((value >> 8) & 1) << 1;
    // 4 | [7:0] | Max OAQ (worst OAQ) since last report (LSB [7:0])
    UAIR_net_buffer[4] = (UAIR_net_buffer[4] & ~0xff) | (value & 0xff);

    res = UAIR_sensors_read_measure(SENSOR_ID_TEMP_AVG_EXTERNAL, &value);
    // 0 | [4] | External temp/hum health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 4;
    // 1 | [7:0] | Avg ext. temp since last report
    UAIR_net_buffer[1] = (uint8_t)value;

    res = UAIR_sensors_read_measure(SENSOR_ID_HUM_AVG_EXTERNAL, &value);
    // FixMe already taken? payload wrong??
    // 0 | [4] | External temp/hum health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 4;
    // 2 | [7:1] | Avg ext. hum since last report
    UAIR_net_buffer[2] = (UAIR_net_buffer[2] & ~0x7f) | (value & 0x7f);
    
    res = UAIR_sensors_read_measure(SENSOR_ID_SOUND_LVL_MAX, &value);
    // 0 | [3] | Microphone health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 3;
    // 2 | [0] | Max sound level (noisiest) since last report (MSB [4])
    UAIR_net_buffer[2] |= ((value >> 4) & 1) << 0;
    // 5 | [7:4] | Max sound level (noisiest) since last report (LSB)
    UAIR_net_buffer[5] = (UAIR_net_buffer[5] & ~0xf0) | (value & 0xf0);

    res = UAIR_sensors_read_measure(SENSOR_ID_SOUND_LVL_AVG, &value);
    // 5 | [3:0] | Avg sound level since last report (LSB [3:0])
    UAIR_net_buffer[5] = (UAIR_net_buffer[5] & ~0x0f) | (value & 0x0f);
    // 7 | [0] | Avg sound level since last report (MSB)
    UAIR_net_buffer[7] = (UAIR_net_buffer[7] & ~0x1) | (value & 0x1);
    
    res = UAIR_sensors_read_measure(SENSOR_ID_TEMP_MAX_INTERNAL, &value);
    // 0 | [5] | Internal temp/hum health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 5;
    // 6 | [7:0] | Max. internal temp since last report
    UAIR_net_buffer[6] = (uint8_t)value;

    res = UAIR_sensors_read_measure(SENSOR_ID_HUM_MAX_INTERNAL, &value);
    // FixMe already taken? payload wrong??
    // 0 | [5] | Internal temp/hum health (1: valid, 0: not valid)
    UAIR_net_buffer[0] |= res << 5;
    // 7 | [7:1] | Max. internal hum since last reportAPP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "decimal 0 %d \r\n", UAIR_net_buffer[0]);
    UAIR_net_buffer[7] = (UAIR_net_buffer[7] & ~0x7f) | (value & 0x7f);    

    #ifdef DEBUGGER_ON
    print_binary(6, &UAIR_net_buffer[0]);
    #endif

    if (UAIR_lora_send(UAIR_net_buffer, 8))
        UAIR_sensors_clear_measures();
}

void UAIR_sensor_event_listener(void *userdata, uint8_t audit_type) { 
    uair_io_context ctx;

    // ToDo: we need to persist policy in config, retrieve it to know state each time we boot
    uint8_t policy = 0;
    UAIR_anomaly_process(audit_type, &policy);
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "current policy is %d\n", policy);

    UAIR_io_init_ctx(&ctx);
    int new_id = UAIR_io_audit_add(&ctx, &audit_type, sizeof(uint8_t));
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "got an anomaly event from sensors. Audit %d added with id %d\n", audit_type, new_id);
}

void UAIR_controller_start(void) {
    UAIR_link_commands_t cmd_cbs = { 
        .cmd_tx_policy = &cmd_tx_policy, 
        .cmd_fair_ratio = &cmd_fair_ratio, 
        .cmd_factory_reset =  &cmd_factory_reset, 
        .cmd_healthchk_ack = &cmd_healthchk_ack };

    LoRaWAN_Init(&cmd_cbs);
    UAIR_sensors_init();

    // load configuration
    uint8_t value;
    uair_io_context ctx;
    UAIR_io_init_ctx(&ctx);
    UAIR_io_config_read_uint8(&ctx, UAIR_IO_CONTEXT_KEY_CONFIG_TX_POLICY, &value);
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "config NET_TX : %d FAIR_RATIO %d \n", value, 0);

    UAIR_sensors_audit_register_listener(NULL, &UAIR_sensor_event_listener);

    UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, send_type1);
    // send every time timer elapses
    UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer, UAIR_CONSERVATIVE_TX);
    UTIL_TIMER_Start(&TxTimer);
/*
    //Ticking every two seconds
    UTIL_TIMER_Create(&TxTimerTmp, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEventTmp, NULL);
    UTIL_TIMER_SetPeriod(&TxTimerTmp, 2000);
    UTIL_TIMER_Start(&TxTimerTmp);
*/
}

uint8_t UAIR_policy_set(uair_io_context_keys id, uint8_t value) {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n set_policy not implemented yet\r\n");
    return 0;
}