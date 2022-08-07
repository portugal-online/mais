#include "UAIR_config_api.h"

#include <UAIR_BSP_flash.h>

#include <limits>
#include <array>

#include <catch2/catch.hpp>

extern size_t g_config_api_flash_num_reads; //declared in UAIR_config_api.cc

TEST_CASE("UAIR config API - cache", "[BSP][BSP app][BSP config]")
{
     REQUIRE(uair_config_cache_size(-1) == 0);
     REQUIRE(uair_config_cache_size(2) == 2);
     REQUIRE(uair_config_cache_size(-1) == 2);
     REQUIRE(uair_config_cache_size(-5) == 2);
     REQUIRE(uair_config_cache_size(0) == 0);
}

TEST_CASE("UAIR config API - defaults", "[BSP][BSP app][BSP config]")
{
     //this is valid
     REQUIRE(config_defaults_uint8(nullptr) != nullptr);

     int defs_size;
     auto defs = config_defaults_uint8(&defs_size);

     //this is standard, so the values must match
     REQUIRE(defs != nullptr);
     REQUIRE(defs_size == 2);
     REQUIRE(defs[0].id == UAIR_CONFIG_ID_TX_POLICY);
     REQUIRE(defs[1].id == UAIR_CONFIG_ID_FAIR_RATIO);
     REQUIRE(defs[0].value == 1);
     REQUIRE(defs[1].value == 1);

     //this doesn't allocate, so the pointers must match
     REQUIRE(config_defaults_uint8(nullptr) == defs);
}

TEST_CASE("UAIR config API - read / write single", "[BSP][BSP app][BSP config]")
{
     REQUIRE(uair_config_cache_size(0) == 0); //disables cache

     SECTION("clear flash")
     {
          auto page_count = UAIR_BSP_flash_config_area_get_page_count();
          for (decltype(page_count) page_index = 0; page_index < page_count; page_index++)
               UAIR_BSP_flash_config_area_erase_page(page_index);
     }

     SECTION("read no data")
     {
          uint8_t value;
          REQUIRE(uair_config_read_uint8((uair_config_id)0xdead, &value) != UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) != UAIR_IO_CONTEXT_ERROR_NONE);
     }

     SECTION("write")
     {
          REQUIRE(uair_config_write_uint8(UAIR_CONFIG_ID_TX_POLICY, 2) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_write_uint8(UAIR_CONFIG_ID_FAIR_RATIO, 3) == UAIR_IO_CONTEXT_ERROR_NONE);
     }

     SECTION("read")
     {
          REQUIRE(uair_config_cache_size(10) == 10);

          auto num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 2);

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 3);

               REQUIRE((num_reads + 2) == g_config_api_flash_num_reads); //all of these should have been read from the flash
          }

          num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 2);

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 3);

               REQUIRE(num_reads == g_config_api_flash_num_reads); //but not these
          }

          //this should also be valid (it's a way to check if the key exists)
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, nullptr) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, nullptr) == UAIR_IO_CONTEXT_ERROR_NONE);
     }

     SECTION("read (small cache)")
     {
          REQUIRE(uair_config_cache_size(1) == 1);

          auto num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 2);

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 3);

               REQUIRE((num_reads + 2) == g_config_api_flash_num_reads); //all of these should have been read from the flash
          }

          num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               //taking into account the last read, this next one should be on the cache
               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 3);

               REQUIRE(num_reads == g_config_api_flash_num_reads);
          }

          num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               //taking into account the last read, this next one should not be on the cache
               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 2);

               REQUIRE((num_reads + 1) == g_config_api_flash_num_reads);
          }

          //this should also be valid (it's a way to check if the key exists)
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, nullptr) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, nullptr) == UAIR_IO_CONTEXT_ERROR_NONE);
     }

     SECTION("read (no cache)")
     {
          REQUIRE(uair_config_cache_size(-1) == 0); //should already be disabled

          auto num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 2);

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 3);

               REQUIRE((num_reads + 2) == g_config_api_flash_num_reads); //all of these should have been read from the flash
          }

          num_reads = g_config_api_flash_num_reads;
          {
               uint8_t value;

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 2);

               REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
               REQUIRE(value == 3);

               REQUIRE((num_reads + 2) == g_config_api_flash_num_reads); //no item were stored on the cache
          }

          //this should also be valid (it's a way to check if the key exists)
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, nullptr) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, nullptr) == UAIR_IO_CONTEXT_ERROR_NONE);
     }

     SECTION("write / read cached")
     {
          REQUIRE(uair_config_cache_size(10) == 10); //setting the cache size always resets the cache

          REQUIRE(uair_config_write_uint8(UAIR_CONFIG_ID_TX_POLICY, 63) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_write_uint8(UAIR_CONFIG_ID_FAIR_RATIO, 59) == UAIR_IO_CONTEXT_ERROR_NONE);

          auto num_reads = g_config_api_flash_num_reads;

          uint8_t value;

          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(value == 63);

          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(value == 59);

          REQUIRE(num_reads == g_config_api_flash_num_reads); //values should already be in cache
     }
}

TEST_CASE("UAIR config API - read / write multiple", "[BSP][BSP app][BSP config]")
{
     REQUIRE(uair_config_cache_size(0) == 0); //disables cache

     SECTION("clear flash")
     {
          auto page_count = UAIR_BSP_flash_config_area_get_page_count();
          for (decltype(page_count) page_index = 0; page_index < page_count; page_index++)
               UAIR_BSP_flash_config_area_erase_page(page_index);
     }

     SECTION("write (no data)")
     {
          //these are invalid uses of the API, but protected
          REQUIRE(uair_config_write_uint8s(nullptr, 0) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_write_uint8s(nullptr, 7) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_write_uint8s((uair_config_pair_uint8*)0xcafe, 0) == UAIR_IO_CONTEXT_ERROR_NONE);
     }

     SECTION("read (no data)")
     {
          //these are invalid uses of the API, but protected
          REQUIRE(uair_config_read_uint8s(nullptr, 0) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_read_uint8s(nullptr, 10) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(uair_config_read_uint8s((uair_config_pair_uint8*)0xcafe, 0) == UAIR_IO_CONTEXT_ERROR_NONE);

          std::array<uair_config_pair_uint8, 2> values_read = {{
               { UAIR_CONFIG_ID_TX_POLICY, 0xFF },
               { UAIR_CONFIG_ID_FAIR_RATIO, 0 }
          }};
          REQUIRE(uair_config_read_uint8s(values_read.data(), (int)values_read.size()) != UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(values_read[0].value == 0xFF);
          REQUIRE(values_read[1].value == 0);
     }

     SECTION("write")
     {
          std::array<uair_config_pair_uint8, 2> values_write = {{
               { UAIR_CONFIG_ID_FAIR_RATIO, 12 },
               { UAIR_CONFIG_ID_TX_POLICY, 34 }
          }};
          REQUIRE(uair_config_write_uint8s(values_write.data(), (int)values_write.size()) == UAIR_IO_CONTEXT_ERROR_NONE);

          uint8_t value;

          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_FAIR_RATIO, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(value == 12);

          REQUIRE(uair_config_read_uint8(UAIR_CONFIG_ID_TX_POLICY, &value) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(value == 34);
     }

     SECTION("read")
     {
          std::array<uair_config_pair_uint8, 2> values_read = {{
               { UAIR_CONFIG_ID_FAIR_RATIO, 55 },
               { UAIR_CONFIG_ID_TX_POLICY, 66 }
          }};
          REQUIRE(uair_config_read_uint8s(values_read.data(), (int)values_read.size()) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(values_read[0].id == UAIR_CONFIG_ID_FAIR_RATIO);
          REQUIRE(values_read[0].value == 12);
          REQUIRE(values_read[1].id == UAIR_CONFIG_ID_TX_POLICY);
          REQUIRE(values_read[1].value == 34);

          //this is also valid
          std::array<uair_config_pair_uint8, 4> values_read_dup = {{
               { UAIR_CONFIG_ID_FAIR_RATIO, 55 },
               { UAIR_CONFIG_ID_TX_POLICY, 66 },
               { UAIR_CONFIG_ID_TX_POLICY, 77 },
               { UAIR_CONFIG_ID_FAIR_RATIO, 88 }
          }};
          REQUIRE(uair_config_read_uint8s(values_read_dup.data(), (int)values_read_dup.size()) == UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(values_read_dup[0].id == UAIR_CONFIG_ID_FAIR_RATIO);
          REQUIRE(values_read_dup[0].value == 12);
          REQUIRE(values_read_dup[1].id == UAIR_CONFIG_ID_TX_POLICY);
          REQUIRE(values_read_dup[1].value == 34);
          REQUIRE(values_read_dup[2].id == UAIR_CONFIG_ID_TX_POLICY);
          REQUIRE(values_read_dup[2].value == 34);
          REQUIRE(values_read_dup[3].id == UAIR_CONFIG_ID_FAIR_RATIO);
          REQUIRE(values_read_dup[3].value == 12);
     }

     SECTION("read incorrect")
     {
          std::array<uair_config_pair_uint8, 4> values_read_dup = {{
               { UAIR_CONFIG_ID_FAIR_RATIO, 91 },
               { UAIR_CONFIG_ID_TX_POLICY, 92 },
               { (uair_config_id)0xbeef, 93 },
               { UAIR_CONFIG_ID_FAIR_RATIO, 94 }
          }};
          REQUIRE(uair_config_read_uint8s(values_read_dup.data(), (int)values_read_dup.size()) != UAIR_IO_CONTEXT_ERROR_NONE);
          REQUIRE(values_read_dup[0].id == UAIR_CONFIG_ID_FAIR_RATIO);
          REQUIRE(values_read_dup[0].value == 12);
          REQUIRE(values_read_dup[1].id == UAIR_CONFIG_ID_TX_POLICY);
          REQUIRE(values_read_dup[1].value == 34);
          REQUIRE(values_read_dup[2].id == (uair_config_id)0xbeef);
          REQUIRE(values_read_dup[2].value == 93);
          REQUIRE(values_read_dup[3].id == UAIR_CONFIG_ID_FAIR_RATIO);
          REQUIRE(values_read_dup[3].value == 94);
     }
}