#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_iwdg.h"
#include "stm32wlxx_hal_conf.h"
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

DECLARE_LOG_TAG(HAL_IWDG)
#define TAG "HAL_IWDG"

static pthread_t iwdg_thread;
static bool iwdg_thread_initialized = false;
static volatile bool iwdg_exit = false;

void __attribute__((weak)) watchdog_timeout();

void iwdg_deinit()
{
    iwdg_exit = true;
    void *ret;
    if (iwdg_thread_initialized) {
        pthread_join(iwdg_thread, &ret);
        iwdg_thread_initialized = false;
    }
}

static void *iwdg_thread_runner(void *user)
{
    IWDG_HandleTypeDef *hiwdg = (IWDG_HandleTypeDef *)user;
    uint32_t prescaler = 4U << (hiwdg->Instance->prescaler);

    while (!iwdg_exit)
    {
        usleep( (1000000U/LSI_VALUE) * prescaler); // 4 ms
        if (hiwdg->Instance->counter == 0U) {
            watchdog_timeout();
        } else {
            hiwdg->Instance->counter--;
        }
    }
    return NULL;
}

HAL_StatusTypeDef HAL_IWDG_Init(IWDG_HandleTypeDef *hiwdg)
{
    HAL_StatusTypeDef r = HAL_ERROR;
    /* Return function status */
    if (!iwdg_thread_initialized) {
        hiwdg->Instance->prescaler = hiwdg->Init.Prescaler;
        hiwdg->Instance->period = hiwdg->Init.Reload;
        hiwdg->Instance->counter = hiwdg->Init.Reload;

        HLOG(TAG, "Initializing IWDG, LSI_VALUE %lu", LSI_VALUE);

        if (pthread_create(&iwdg_thread, NULL, iwdg_thread_runner, hiwdg)==0) {
            iwdg_thread_initialized = true;

            r = HAL_OK;
        }
    }
    return r;
}

HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg)
{
    /* Return function status */
    uint16_t oldcounter = hiwdg->Instance->counter;

    hiwdg->Instance->counter = hiwdg->Instance->period;

    HLOG(TAG, "Watchdog kick: %d ms remaining", oldcounter * 4 * (4U<<hiwdg->Instance->prescaler));

    return HAL_OK;
}

void watchdog_timeout()
{
    HERROR(TAG, "\n**********************************************************\n"
    "*\n"
    "*\n"
    "* WATCHDOG TIMEOUT\n"
    "*\n"
    "*\n"
    "**********************************************************\n");
    abort();
}
