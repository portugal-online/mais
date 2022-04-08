/** Copyright © 2021 The Things Industries B.V.
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
 * @file app_conf.h
 * @brief Common configuration file for GNSE applications
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#ifndef APP_CONF_H
#define APP_CONF_H

#define UAIR_ADVANCED_TRACER_ENABLE 0
#define UAIR_TINY_TRACER_ENABLE 1

/* if ON (=1) it enables the debugger plus 4 dbg pins */
/* if OFF (=0) the debugger is OFF (lower consumption) */
#define DEBUGGER_ON       0

/*!
 * LoRaWAN application port where sensors information can be retrieved by the application server
 * @note do not use 224. It is reserved for certification
 */
#define SENSORS_PAYLOAD_APP_PORT        2

/*!
 * Defines the application data transmission duty cycle. 120s, value in [ms].
 */
#define SENSORS_TX_DUTYCYCLE                            120000

/* LoRaWAN v1.0.3 software based OTAA activation information */

#define APPEUI                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define DEVEUI                 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0x42, 0xD6
#define APPKEY                 0x9B, 0x45, 0x27, 0xBA, 0x42, 0x28, 0xF4, 0x3C, 0xB9, 0x30, 0x0F, 0xCF, 0xD5, 0xDE, 0x5C, 0xA6
/**
  * sequencer definitions
  */

/**
  * This is the list of priority required by the application
  * Each Id shall be in the range 0..31
  */
typedef enum
{
  CFG_SEQ_Prio_0,
  CFG_SEQ_Prio_NBR,
} CFG_SEQ_Prio_Id_t;

/**
  * This is the list of task id required by the application
  * Each Id shall be in the range 0..31
  */
typedef enum
{
  CFG_SEQ_Task_LmHandlerProcess,
  CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent,
  CFG_SEQ_Task_NBR,
  CFG_SEQ_Task_CMD,
} CFG_SEQ_Task_Id_t;

#endif /* APP_CONF_H */
