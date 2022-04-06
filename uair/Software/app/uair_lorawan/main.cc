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

#ifdef HOSTMODE
extern "C"
{
    void bsp_set_hostmode_arguments(int argc, char **argv);
};
#endif

#ifdef UNITTESTS

  #define CATCH_CONFIG_RUNNER
  #include <catch2/catch.hpp>

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

#ifdef HOSTMODE
    bsp_set_hostmode_arguments(argc,argv);
#endif

    if (BSP_init(&config)!=BSP_ERROR_NONE) {
        while (1) {
            __WFI();
        }
    }
    
    std::set_terminate(&uair_terminate);
    UAIR_controller_start();
    
    // avoid fall-off
    while (1) UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);


#endif
}
