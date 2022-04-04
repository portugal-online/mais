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
 * @file main.c
 *
 * @copyright Copyright (c) 2021 The Things Industries B.V.
 *
 */

#include <exception>

extern "C" {
#include "BSP.h"
#include "stm32_seq.h"
#include "sys_app.h"
#include "lora_app.h"
}

#ifdef UNITTESTS

  #define CATCH_CONFIG_RUNNER
  #include <catch2/catch.hpp>

#else

    static void MX_LoRaWAN_Init(void)
    {
        SystemApp_Init();
        LoRaWAN_Init();
    }

    static void MX_LoRaWAN_Process(void)
    {
        UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
    }

#endif

void uair_terminate(void)
{
    BSP_FATAL();
}

int main(int argc, char* argv[])
{

#ifdef UNITTESTS
    int r;
    BSP_config_t config;

    BSP_get_default_config(&config);

    config.skip_shield_init = true;

    if (BSP_init(&config)!=BSP_ERROR_NONE) {
        fprintf(stderr,"Cannot initialise BSP");
        abort();
    }
    UAIR_BSP_link_powerzones();


    r =  Catch::Session().run(argc, argv);

    BSP_deinit();

    return r;

#else

    BSP_config_t config;
    BSP_get_default_config(&config);

  //  config.force_uart_on = true;

    if (BSP_init(&config)!=BSP_ERROR_NONE) {
        while (1) {
            __WFI();
        }
    }

    std::set_terminate(&uair_terminate);

    MX_LoRaWAN_Init();
    while (1)
        MX_LoRaWAN_Process();

#endif
}
