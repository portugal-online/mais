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
 * @file lora_app.h
 * @based lora_app (Application of the LRWAN Middleware)
 *
 * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044

 */

#include "app.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "stm32_timer.h"
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "LmHandler.h"
#include "lora_info.h"
#include "sensors.h"

/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief Application data transmission issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief AppdataTransmition external event plugged on OnSendEvent( )
    */
  TX_ON_EVENT
} TxEventType_t;

/**
  * @brief  LoRa endNode send request
  * @param  none
  * @return none
  */
//static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  timer context
  * @return none
  */
//static void OnTxTimerEvent(void *context);

/**
  * @brief  LED timer callback function
  * @param  LED context
  * @return none
  */
//static void OnTimerLedEvent(void *context);

/**
  * @brief  join event callback function
  * @param  params
  * @return none
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params
  * @return none
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa endNode has received a frame
  * @param appData
  * @param params
  * @return None
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = {0, 0, AppDataBuffer};

static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

static UAIR_link_commands_t *UAIR_cmd_callbacks;

/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
    {
        .GetBatteryLevel = GetBatteryLevel,
        .GetTemperature = GetTemperatureLevel,
        .OnMacProcess = OnMacProcessNotify,
        .OnJoinRequest = OnJoinRequest,
        .OnTxData = OnTxData,
        .OnRxData = OnRxData
        };

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
    {
        .ActiveRegion = ACTIVE_REGION,
        .DefaultClass = LORAWAN_DEFAULT_CLASS,
        .AdrEnable = LORAWAN_ADR_STATE,
        .TxDatarate = LORAWAN_DEFAULT_DATA_RATE,
        .PingPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY};

/**
  * @brief Type of Event to generate application Tx
  */
//static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
//static UTIL_TIMER_Object_t TxTimer;

/**
  * @brief Timer to handle the application Tx Led to toggle
  */
//static UTIL_TIMER_Object_t TxLedTimer;

void LoRaWAN_Init(UAIR_link_commands_t *cmd_callbacks)
{
  // User can add any indication here (LED manipulation or Buzzer)

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  //UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);
#ifdef JOIN_IMMEDIATLY
  LmHandlerJoin(ActivationType);
#else
  (void)ActivationType;
#endif

  UAIR_cmd_callbacks = cmd_callbacks;
/*
  if (EventType == TX_ON_TIMER)
  {
    // send every time timer elapses
    UTIL_TIMER_Create(&TxTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer, SENSORS_TX_DUTYCYCLE);
    UTIL_TIMER_Start(&TxTimer);
  }
*/
}

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  if ((appData != NULL) && (params != NULL)) {
    // all commands have 6 bytes in size
    if (appData->BufferSize != 6) {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Received unsupported Downlink with %d bytes, on F_PORT:%d \r\n", appData->BufferSize, appData->Port);
      return;
    }

    uint32_t uair_msg;
    uint8_t cmd;
    uint8_t magic_sig;

    //byte 3-6 is msg
    memcpy(&uair_msg, &appData->Buffer[16], 32);
    //byte 2 is cmd
    memcpy(&cmd, &appData->Buffer[8], 8);
    //byte 1 is magic number
    memcpy(&magic_sig, &appData->Buffer[0], 8);
    
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Received Downlink with %d bytes, on F_PORT:%d | magic %d cmd %d msgs %d \r\n", 
      appData->BufferSize, appData->Port, magic_sig, cmd, uair_msg);  

    if (magic_sig != 99) {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Msg with wrong magic number %d %d \r\n", magic_sig);
      return;
    }

    switch (cmd) {
      case 10:
        UAIR_cmd_callbacks->cmd_tx_policy(uair_msg);
        break;
      case 20:
        UAIR_cmd_callbacks->cmd_fair_ratio(uair_msg);
        break;
      case 30:
        UAIR_cmd_callbacks->cmd_healthchk_ack(uair_msg);
        break;
      case 99:
        UAIR_cmd_callbacks->cmd_factory_reset(uair_msg);
        break;
      default:
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n Msg with wrong cmd number %d %d \r\n", cmd);
    }
  }
}
/*
static uint8_t EncodeTemperature(uint32_t temp_millicentigrades)
{
    
     Encodes a temperature between -11.75C and +52C into 8-bit, with 0.25C accuracy

     To extract the temperature from the data (B), use:

      T = (B-47)/4

     */
/*
    int32_t rtemp = (temp_millicentigrades/250) + 47;
    if (rtemp<0)
        rtemp = 0;
    if (rtemp>255)
        rtemp = 255;
    return (uint8_t)(rtemp & 0xff);
}
*/

/*
static uint8_t EncodeHumidity(uint32_t hum_millipercent)
{
    
     Encodes a humidity (0-100%) into 8-bit, using 0.5% accuracy

     To extract the humidity from the data (B), use:

      H= B/2

     */
/*    
    int32_t rhum = hum_millipercent / 500;

    if (rhum<0)
        rhum=0;

    if (rhum>200)
        rhum=200;

    return (int8_t)(rhum & 0xff);
}
*/

uint8_t UAIR_lora_send(uint8_t buf[], uint8_t len) {
  UTIL_TIMER_Time_t nextTxIn = 0;
  AppData.Port = SENSORS_PAYLOAD_APP_PORT;
  //AppData.BufferSize = sizeof(*buf)/sizeof(uint8_t);
  AppData.BufferSize = len;
  AppData.Buffer = buf;
  LmHandlerErrorStatus_t res = LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false);
  APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST %d\r\n", res);

  return 1;
}


/*
static void SendTxData(void)
{
    sensors_t sensor_data;
    UTIL_TIMER_Time_t nextTxIn = 0;

    sensors_sample(&sensor_data);
    AppData.Port = SENSORS_PAYLOAD_APP_PORT;
    AppData.BufferSize = 2;

    // sensor temp is reported in milli degrees C.

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "Internal sensor temperature mC: %d humidity %f%%\r\n",
            sensor_data.th_internal.temp,
            (float)sensor_data.th_internal.hum/1000.0F);

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "External temperature mC: %d humidity %f%%\r\n",
            sensor_data.th_external.temp,
            (float)sensor_data.th_external.hum/1000.0F);

    AppData.Buffer[1] = EncodeTemperature(sensor_data.th_internal.temp);
    AppData.Buffer[0] = EncodeHumidity(sensor_data.th_internal.hum);

    UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTimerLedEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxLedTimer, 200);
    UTIL_TIMER_Start(&TxLedTimer);

    BSP_LED_on(LED_BLUE);

    if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
    {
        APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "SEND REQUEST\r\n");
    }
    else if (nextTxIn > 0)
    {
        APP_LOG(ADV_TRACER_TS_ON, ADV_TRACER_VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
    }
}
*/

/*
static void OnTxTimerEvent(void *context)
{
    UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

    // Wait for next tx slot
    UTIL_TIMER_Start(&TxTimer);
}
*/

/*
static void OnTimerLedEvent(void *context)
{
    BSP_LED_off(LED_BLUE);
}
*/

static void OnTxData(LmHandlerTxParams_t *params)
{
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### ========== MCPS-Confirm =============\r\n");
    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d", params->UplinkCounter,
            params->AppData.Port, params->Datarate, params->TxPower);

    APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, " | MSG TYPE:");
    if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "CONFIRMED [%s]\r\n", (params->AckReceived != 0) ? "ACK" : "NACK");
    }
    else
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_H, "UNCONFIRMED\r\n");
    }
  }
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOINED = ");
      if (joinParams->Mode == ACTIVATION_TYPE_ABP)
      {
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "ABP ======================\r\n");
      }
      else
      {
        APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "OTAA =====================\r\n");
      }
    }
    else
    {
      APP_LOG(ADV_TRACER_TS_OFF, ADV_TRACER_VLEVEL_M, "\r\n###### = JOIN FAILED. Status %d\r\n", joinParams->Status);
    }
  }
}

static void OnMacProcessNotify(void)
{
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
