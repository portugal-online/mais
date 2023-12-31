/** Copyright © 2021 The Things Industries B.V.
 *  Copyright © 2021 MAIS Project
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
 * @file UAIR_bsp_conf.h
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V., (c) 2021 MAIS Project
 *
 * @ingroup UAIR_BSP_CORE
 */

#ifndef UAIR_BSP_CONF_H__
#define UAIR_BSP_CONF_H__

#ifdef __cplusplus
 extern "C" {
#endif

#define UAIR_SYSCLK_SPEED_MHZ 4

#if UAIR_SYSCLK_SPEED_MHZ==2
#define MSI_RANGE_LOWPOWER RCC_MSIRANGE_5
#elif UAIR_SYSCLK_SPEED_MHZ==4
#define MSI_RANGE_LOWPOWER RCC_MSIRANGE_6
#else
#error Invalid clock speed
#endif


/* IRQ priorities
* Can be from 0 to 15 with 0 as the highest
* With OS, the lowest value should be the scheduler timer IRQ
* In FreeRTOS this is configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
*/
#define UAIR_BSP_SUBGHZ_RADIO_IT_PRIORITY       5U
#define UAIR_BSP_RTC_IT_PRIORITY                5U
#define UAIR_BSP_LPTIM1_IT_PRIORITY             1U
#define DEBUG_USART_IT_PRIORITY                 7U
#define DEBUG_USART_DMA_IT_PRIORITY             7U
#define MICROPHONE_IT_PRIORITY                  8U
#define MICROPHONE_DMA_IT_PRIORITY              8U


#define UAIR_BSP_BUTTON_SWx_IT_PRIORITY         15U
#define ACC_INT_PRIORITY                        15U

//#define UAIR_BSP_ONBATTERY_DEBUG 1

#ifdef __cplusplus
}
#endif

#endif /* UAIR_BSP_CONF_H */
