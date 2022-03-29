#include "UAIR_io_config.h"

#include <UAIR_BSP_flash.h>

#include <catch2/catch.hpp>

TEST_CASE("UAIR IO config", "basic")
{
     auto page_count = UAIR_BSP_flash_config_area_get_page_count();
     INFO("Config page count: " << page_count);
     REQUIRE(page_count >= 2);
}
