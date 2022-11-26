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
 * @file main.c
 *
 */

#include <exception>

#include "BSP.h"
#include "stm32_seq.h"
#include "sys_app.h"
#include "controller.h"
#include "lora_app.h"

void uair_terminate(void)
{
    BSP_FATAL();
}

extern "C" void sensor_start_measure_callback()
{
    if (UAIR_BSP_FR_Active())
        BSP_LED_on(LED_GREEN);
}

extern "C" void sensor_end_measure_callback()
{
    BSP_LED_off(LED_GREEN);
}


int APP_MAIN(int argc, char* argv[])
{

    BSP_config_t config;
    BSP_get_default_config(&config);

#if defined (RELEASE) && (RELEASE==1)
#else
    config.force_uart_on = true;
    config.disable_network = true;
#endif

    if (BSP_init(&config)!=BSP_ERROR_NONE) {
        while (1) {
            __WFI();
        }
    }
    
    std::set_terminate(&uair_terminate);
    UAIR_controller_start();
    
    // avoid fall-off
    while (FOREVER)
    {
        UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
    }
    return 0;
}
