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
#include <atomic>

extern "C"
{
    void bsp_set_hostmode_arguments(int argc, char **argv);
    void test_BSP_init(int skip_shield);
};

#ifdef UNITTESTS

  #define CATCH_CONFIG_RUNNER
  #include <catch2/catch.hpp>

#endif

void test_BSP_init(int skip_shield)
{
    BSP_config_t config;

    BSP_get_default_config(&config);

    config.skip_shield_init = (skip_shield==0) ? false : true;

    if (BSP_init(&config)!=BSP_ERROR_NONE) {
        fprintf(stderr,"Cannot initialise BSP");
        abort();
    }
}

std::atomic_flag run_main_loop(1);

int forever_hook()
{
    if (!run_main_loop.test_and_set())
        return 0;
    return 1;
}

void test_exit_main_loop()
{
    run_main_loop.clear();
}

void test_BSP_deinit()
{
    BSP_deinit();
}

int main(int argc, char* argv[])
{

#ifdef UNITTESTS


    int r =  Catch::Session().run(argc, argv);


    return r;

#else

    bsp_set_hostmode_arguments(argc,argv);

    return app_main(argc, argv);

#endif
}

