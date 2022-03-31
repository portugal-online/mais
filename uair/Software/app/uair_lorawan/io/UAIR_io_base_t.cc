#include "UAIR_io_base.h"

#include <UAIR_BSP_flash.h>

#include <limits>

#include <catch2/catch.hpp>

TEST_CASE("UAIR IO flash", "io_base_flash")
{
     auto page_count = UAIR_BSP_flash_config_area_get_page_count();
     INFO("Config page count: " << page_count);
     INFO("Config page size: " << BSP_FLASH_PAGE_SIZE);
     REQUIRE(page_count >= 2);
     REQUIRE(((page_count * BSP_FLASH_PAGE_SIZE) % sizeof(uint64_t)) == 0);

     //fill all the available space with dwords starting at 0 and incrementing 1 per dword written
     SECTION("write")
     {
          flash_address_t fbegin = 0;
          flash_address_t fend = page_count * BSP_FLASH_PAGE_SIZE;

          uint64_t data = 0;
          for (; fbegin < fend; fbegin += sizeof(uint64_t))
          {
               REQUIRE(UAIR_BSP_flash_config_area_write(fbegin, (uint64_t*)&data, 1) == 1);
               data++;
          }
          REQUIRE(fbegin == fend);
          REQUIRE(data == ((page_count * BSP_FLASH_PAGE_SIZE) / sizeof(uint64_t)));
     }

     //read and check if the expected dwords were written properly
     SECTION("read")
     {
          flash_address_t fbegin = 0;
          flash_address_t fend = page_count * BSP_FLASH_PAGE_SIZE;

          uint64_t data_expected = 0;
          for (; fbegin < fend; fbegin += sizeof(uint64_t))
          {
               uint64_t data = 0xbadcafe;
               REQUIRE(UAIR_BSP_flash_config_area_read(fbegin, (uint8_t*)&data, sizeof(uint64_t)) == sizeof(uint64_t));
               REQUIRE(data == data_expected);

               data_expected++;
          }
          REQUIRE(fbegin == fend);
     }

     //erase the first page and read/check again
     SECTION("erase")
     {
          flash_address_t fbegin = 0;
          flash_address_t fpage = BSP_FLASH_PAGE_SIZE;
          flash_address_t fend = page_count * BSP_FLASH_PAGE_SIZE;

          REQUIRE(UAIR_BSP_flash_config_area_erase_page(0) == 0);

          uint64_t data_expected = 0;
          for (; fbegin < fpage; fbegin += sizeof(uint64_t))
          {
               uint64_t data = 0xbadcafe;
               REQUIRE(UAIR_BSP_flash_config_area_read(fbegin, (uint8_t*)&data, sizeof(uint64_t)) == sizeof(uint64_t));
               REQUIRE(data == std::numeric_limits<uint64_t>::max() );

               data_expected++;
          }
          REQUIRE(fbegin == fpage);
          REQUIRE(fbegin < fend);

          for (; fbegin < fend; fbegin += sizeof(uint64_t))
          {
               uint64_t data = 0xbadcafe;
               REQUIRE(UAIR_BSP_flash_config_area_read(fbegin, (uint8_t*)&data, sizeof(uint64_t)) == sizeof(uint64_t));
               REQUIRE(data == data_expected);

               data_expected++;
          }
          REQUIRE(fbegin == fend);
     }
}

TEST_CASE("UAIR IO context", "io_base_ctx")
{
    SECTION("init")
    {
        UAIR_io_init_ctx(nullptr);

        uair_io_context ctx;
        UAIR_io_init_ctx(&ctx);

        CHECK(ctx.flags == UAIR_IO_CONTEXT_FLAG_NONE);
        CHECK(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
    }
}
