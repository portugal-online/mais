#include "stm32wlxx_hal.h"
#include <stdlib.h>
#include <sys/time.h>
#include "timeutils.h"

DECLARE_LOG_TAG(HAL_RTC)
#define TAG "HAL_RTC"

#include "models/hw_rtc.h"

static uint32_t flags = 0;


HAL_StatusTypeDef HAL_RTC_Init(RTC_HandleTypeDef *hrtc)
{
    rtc_engine_enable();

    return HAL_OK;
}

HAL_StatusTypeDef HAL_RTC_DeInit(RTC_HandleTypeDef *hrtc)
{
    return HAL_OK;
}

HAL_StatusTypeDef HAL_RTC_SetAlarm(RTC_HandleTypeDef *hrtc, RTC_AlarmTypeDef *sAlarm, uint32_t Format)
{
    abort();
    return HAL_OK;
}


HAL_StatusTypeDef HAL_RTC_SetAlarm_IT(RTC_HandleTypeDef *hrtc, RTC_AlarmTypeDef *sAlarm, uint32_t Format)
{
    if (sAlarm->AlarmMask != RTC_ALARMMASK_NONE)
        abort();

    if (sAlarm->AlarmSubSecondMask!=RTC_ALARMSUBSECONDBINMASK_NONE)
        abort();
    

    HLOG(TAG, "Set alarm subseconds=%08x",sAlarm->AlarmTime.SubSeconds);

    rtc_engine_set_alarm_a(sAlarm->AlarmTime.SubSeconds);

    return HAL_OK;
}

HAL_StatusTypeDef HAL_RTC_DeactivateAlarm(RTC_HandleTypeDef *hrtc, uint32_t Alarm)
{
    return HAL_OK;
}


uint32_t LL_RTC_TIME_GetSubSecond(RTC_TypeDef*rtc)
{
    uint32_t r = rtc_engine_get_counter();
//    HLOG("Timer %08x",r);
    return r;
    /*
    update_time_since_start();
    return 0xFFFFFFFF - (rtc_ticks_since_start & 0xFFFFFFFF);
    */
}

HAL_StatusTypeDef HAL_RTCEx_SetSSRU_IT(RTC_HandleTypeDef *hrtc)
{
    return HAL_OK;
}

void              HAL_RTCEx_BKUPWrite(RTC_HandleTypeDef *hrtc, uint32_t BackupRegister, uint32_t Data)
{
    //abort();
}

uint32_t HAL_RTCEx_BKUPRead(RTC_HandleTypeDef *hrtc, uint32_t BackupRegister)
{
    uint32_t r = rtc_engine_get_second_counter();
    return r;
}

uint32_t __HAL_RTC_ALARM_GET_FLAG(RTC_HandleTypeDef*hrtc,uint32_t f)
{
    return flags & f;
}

uint32_t __HAL_RTC_ALARM_CLEAR_FLAG(RTC_HandleTypeDef*hrtc,uint32_t f)
{
    flags &= ~f;
    return 0;
}


HAL_StatusTypeDef HAL_RTCEx_EnableBypassShadow(RTC_HandleTypeDef *hrtc)
{
    return HAL_OK;
}

void HAL_RTCEx_SSRUIRQHandler(RTC_HandleTypeDef *hrtc)
{
    abort();
}

void HAL_RTC_AlarmIRQHandler(RTC_HandleTypeDef *hrtc)
{
    HAL_RTC_AlarmAEventCallback(hrtc);

    //abort();
}
