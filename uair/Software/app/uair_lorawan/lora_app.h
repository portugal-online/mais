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

#ifndef LORA_APP_H__
#define LORA_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* LoraWAN application configuration (Mw is configured by lorawan_conf.h) */
#define ACTIVE_REGION        LORAMAC_REGION_EU868

/*!
 * LoRaWAN default endNode class port
 */
#define LORAWAN_DEFAULT_CLASS                       CLASS_A

/*!
 * LoRaWAN default confirm state
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE         LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE                           LORAMAC_HANDLER_ADR_ON

/*!
 * LoRaWAN default activation type
 */
#define LORAWAN_DEFAULT_ACTIVATION_TYPE             ACTIVATION_TYPE_OTAA

/*!
 * LoRaWAN Default data Rate Data Rate
 * @note Please note that LORAWAN_DEFAULT_DATA_RATE is used only when LORAWAN_ADR_STATE is disabled
 */
#define LORAWAN_DEFAULT_DATA_RATE                   DR_0

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE            242

/*!
 * Default Unicast ping slots periodicity
 *
 * \remark periodicity is equal to 2^LORAWAN_DEFAULT_PING_SLOT_PERIODICITY seconds
 *         example: 2^3 = 8 seconds. The end-device will open an Rx slot every 8 seconds.
 */
#define LORAWAN_DEFAULT_PING_SLOT_PERIODICITY       4

// Command (downlink message) callbacks
typedef struct UAIR_link_commands_s
{
  void (*cmd_tx_policy)(uint32_t command);
  void (*cmd_fair_ratio)(uint32_t command);
  void (*cmd_factory_reset)(uint32_t command);
  void (*cmd_healthchk_ack)(uint32_t command);
} UAIR_link_commands_t;


/**
  * @brief  Init Lora Application
  * @param None
  * @return None
  */
void LoRaWAN_Init(UAIR_link_commands_t *cmd_callbacks);
uint8_t UAIR_lora_send(uint8_t buf[], uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /*__LORA_APP_H__*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
