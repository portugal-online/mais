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


extern "C" {
#include "BSP.h"
#include "stm32_seq.h"
#include "sys_app.h"
#include "controller.h"
}

#ifdef UAIR_UNIT_TESTS

  #define CATCH_CONFIG_RUNNER
  #include <catch2/catch.hpp>

#endif

int main(int argc, char* argv[])
{
#ifdef UAIR_UNIT_TESTS

    return Catch::Session().run(argc, argv);

#else

    if (BSP_init(NULL)!=BSP_ERROR_NONE) {
        while (1) {
            __WFI();
        }
    }

    UAIR_controller_start();
    
    // avoid fall-off
    while (1) UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);


#endif
}
