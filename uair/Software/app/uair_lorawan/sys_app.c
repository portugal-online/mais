/**
  ******************************************************************************
  * @file    sys_app.c
  * @author  MCD Application Team
  * @brief   Initializes HW and SW system entities (not related to the radio)
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include <stdio.h>
#include "app.h"
#include "sys_app.h"
#include "stm32_seq.h"
#include "stm32_systime.h"
#include "BSP.h"
#include "sensors.h"
#include "VM3011.h"
#define MAX_TS_SIZE (int)16


#if 0
/**
  * @brief Returns sec and msec based on the systime in use
  * @param none
  * @return  none
  */
static void TimestampNow(uint8_t *buff, uint16_t *size);

/**
  * @brief  it calls ADV_TRACER_VSNPRINTF
  */
static void tiny_snprintf_like(char *buf, uint32_t maxsize, const char *strFormat, ...);
#endif

/**
  * @brief initialises the system (dbg pins, trace, mbmux, systimer, LPM, ...)
  * @param none
  * @return  none
  * TODO: Improve with system wide Init(), see https://github.com/TheThingsIndustries/generic-node-se/issues/57
  */
void SystemApp_Init(void)
{
    //sensors_init();
}

/**
  * @brief redefines __weak function in stm32_seq.c such to enter low power
  * @param none
  * @return  none
  */
void UTIL_SEQ_Idle(void)
{
    BSP_LPM_enter_low_power_mode();
}

#if 0
static void tiny_snprintf_like(char *buf, uint32_t maxsize, const char *strFormat, ...)
{
    va_list vaArgs;
    va_start(vaArgs, strFormat);
    ADV_TRACER_VSNPRINTF(buf, maxsize, strFormat, vaArgs);
    va_end(vaArgs);
}
static void TimestampNow(uint8_t *buff, uint16_t *size)
{
    SysTime_t curtime = SysTimeGet();
    tiny_snprintf_like((char *)buff, MAX_TS_SIZE, "%ds%03d:", curtime.Seconds, curtime.SubSeconds);
    *size = strlen((char *)buff);
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
