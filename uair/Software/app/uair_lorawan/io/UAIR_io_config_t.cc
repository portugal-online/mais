#include "UAIR_io_config.h"

#include <UAIR_BSP_flash.h>

#include <array>
#include <random>

#include <catch2/catch.hpp>

TEST_CASE("UAIR IO config", "write")
{
	auto page_count = UAIR_BSP_flash_config_area_get_page_count();
	INFO("Config page count: " << page_count);
    INFO("Config page size: " << BSP_FLASH_PAGE_SIZE);
    REQUIRE(page_count >= 2);
    REQUIRE(((page_count * BSP_FLASH_PAGE_SIZE) % sizeof(uint64_t)) == 0);

    SECTION("clear flash")
	{
		for (decltype(page_count) page_index = 0; page_index < page_count; page_index++)
			UAIR_BSP_flash_config_area_erase_page(page_index);
	}

	SECTION("write uint8")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		UAIR_io_config_write_uint8(&ctx, (uair_io_context_keys)10, 0xD8);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		uint8_t val8;
		UAIR_io_config_read_uint8(&ctx, (uair_io_context_keys)10, &val8);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		REQUIRE(val8 == 0xD8);

		size_t num_keys;
		UAIR_io_config_stats(&ctx, &num_keys, nullptr, nullptr, nullptr);
		REQUIRE(num_keys == 1);
	}

	SECTION("write uint16")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		UAIR_io_config_write_uint16(&ctx, (uair_io_context_keys)11, 0xCAFE);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		uint16_t val16;
		UAIR_io_config_read_uint16(&ctx, (uair_io_context_keys)11, &val16);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		REQUIRE(val16 == 0xCAFE);

		size_t num_keys;
		UAIR_io_config_stats(&ctx, &num_keys, nullptr, nullptr, nullptr);
		REQUIRE(num_keys == 2);
	}

	SECTION("write uint32")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		UAIR_io_config_write_uint32(&ctx, (uair_io_context_keys)12, 0xC00010FF);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		uint32_t val32;
		UAIR_io_config_read_uint32(&ctx, (uair_io_context_keys)12, &val32);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		REQUIRE(val32 == 0xC00010FF);

		size_t num_keys;
		UAIR_io_config_stats(&ctx, &num_keys, nullptr, nullptr, nullptr);
		REQUIRE(num_keys == 3);
	}

	SECTION("write uint64")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		UAIR_io_config_write_uint64(&ctx, (uair_io_context_keys)13, 0xC00010FFC00010FF);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		uint64_t val64;
		UAIR_io_config_read_uint64(&ctx, (uair_io_context_keys)13, &val64);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		REQUIRE(val64 == 0xC00010FFC00010FF);

		size_t num_keys;
		UAIR_io_config_stats(&ctx, &num_keys, nullptr, nullptr, nullptr);
		REQUIRE(num_keys == 4);
	}

	SECTION("write random")
	{
		struct Entry
		{
			enum class Type{ UInt8, UInt16, UInt32, UInt64 };

			Type type;
			uair_io_context_keys id;
			uint64_t value;
		};

		int entry_id = 14;
		std::mt19937 mt{ std::random_device()() };
		std::uniform_int_distribution<uint64_t> dist;

		std::vector<Entry> entries{
			{Entry::Type::UInt8, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt8, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt16, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt16, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt32, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt32, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt64, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt64, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt32, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt64, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt16, static_cast<uair_io_context_keys>(entry_id++), dist(mt)},
			{Entry::Type::UInt8, static_cast<uair_io_context_keys>(entry_id++), dist(mt)}
		};

		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		size_t num_keys;
		UAIR_io_config_stats(&ctx, &num_keys, nullptr, nullptr, nullptr);

		for (const auto& entry : entries)
		{
			switch(entry.type)
			{
			case Entry::Type::UInt8: UAIR_io_config_write_uint8(&ctx, entry.id, static_cast<uint8_t>(entry.value)); break;
			case Entry::Type::UInt16: UAIR_io_config_write_uint16(&ctx, entry.id, static_cast<uint16_t>(entry.value)); break;
			case Entry::Type::UInt32: UAIR_io_config_write_uint32(&ctx, entry.id, static_cast<uint32_t>(entry.value)); break;
			case Entry::Type::UInt64: UAIR_io_config_write_uint64(&ctx, entry.id, entry.value); break;
			default: FAIL("Unknown type");
			}
			
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);			
		}

		{
			size_t new_num_keys;
			UAIR_io_config_stats(&ctx, &new_num_keys, nullptr, nullptr, nullptr);
			REQUIRE(new_num_keys == (num_keys + entries.size()));
		}

		std::reverse(entries.begin(), entries.end());
		for (const auto& entry : entries)
		{
			switch(entry.type)
			{
			case Entry::Type::UInt8:{
				uint8_t val;
				UAIR_io_config_read_uint8(&ctx, entry.id, &val);
				REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
				REQUIRE(val == static_cast<uint8_t>(entry.value));
				} break;
			case Entry::Type::UInt16: {
				uint16_t val;
				UAIR_io_config_read_uint16(&ctx, entry.id, &val);
				REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
				REQUIRE(val == static_cast<uint16_t>(entry.value));
				} break;
			case Entry::Type::UInt32: {
				uint32_t val;
				UAIR_io_config_read_uint32(&ctx, entry.id, &val);
				REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
				REQUIRE(val == static_cast<uint32_t>(entry.value));
				} break;
			case Entry::Type::UInt64: {
				uint64_t val;
				UAIR_io_config_read_uint64(&ctx, entry.id, &val);
				REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
				REQUIRE(val == entry.value);
				} break;
			default: FAIL("Unknown type");
			}
		}
	}

	SECTION("replace uint8")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		struct ConfigStats {
			size_t num_keys, used_space, free_space, recyclable_space;
		};
		
		std::array<ConfigStats, 3> stats;
		UAIR_io_config_stats(&ctx, &stats[0].num_keys, &stats[0].used_space, &stats[0].free_space, &stats[0].recyclable_space);

		UAIR_io_config_write_uint8(&ctx, (uair_io_context_keys)10, static_cast<uint8_t>(~0xD8));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		
		{
			//check that the number of keys remain the same, but new space was allocated
			UAIR_io_config_stats(&ctx, &stats[1].num_keys, &stats[1].used_space, &stats[1].free_space, &stats[1].recyclable_space);
			REQUIRE(stats[1].num_keys == stats[0].num_keys);
			REQUIRE(stats[1].used_space > stats[0].used_space);
			REQUIRE(stats[1].free_space < stats[0].free_space);
			REQUIRE(stats[1].recyclable_space > stats[0].recyclable_space);
		}

		{
			//check the value written
			uint8_t val;
			UAIR_io_config_read_uint8(&ctx, (uair_io_context_keys)10, &val);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
			REQUIRE(val == static_cast<uint8_t>(~0xD8));
		}

		//write the same value

		UAIR_io_config_write_uint8(&ctx, (uair_io_context_keys)10, static_cast<uint8_t>(~0xD8));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		{
			//check that the number of keys remains the same and that no space changed
			UAIR_io_config_stats(&ctx, &stats[2].num_keys, &stats[2].used_space, &stats[2].free_space, &stats[2].recyclable_space);
			REQUIRE(stats[2].num_keys == stats[1].num_keys);
			REQUIRE(stats[2].used_space == stats[1].used_space);
			REQUIRE(stats[2].free_space == stats[1].free_space);
			REQUIRE(stats[2].recyclable_space == stats[1].recyclable_space);
		}
	}

	SECTION("replace uint16")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		struct ConfigStats {
			size_t num_keys, used_space, free_space, recyclable_space;
		};
		
		std::array<ConfigStats, 3> stats;
		UAIR_io_config_stats(&ctx, &stats[0].num_keys, &stats[0].used_space, &stats[0].free_space, &stats[0].recyclable_space);

		UAIR_io_config_write_uint16(&ctx, (uair_io_context_keys)11, static_cast<uint16_t>(~0xCAFE));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		
		{
			//check that the number of keys remain the same, but new space was allocated
			UAIR_io_config_stats(&ctx, &stats[1].num_keys, &stats[1].used_space, &stats[1].free_space, &stats[1].recyclable_space);
			REQUIRE(stats[1].num_keys == stats[0].num_keys);
			REQUIRE(stats[1].used_space > stats[0].used_space);
			REQUIRE(stats[1].free_space < stats[0].free_space);
			REQUIRE(stats[1].recyclable_space > stats[0].recyclable_space);
		}

		{
			//check the value written
			uint16_t val;
			UAIR_io_config_read_uint16(&ctx, (uair_io_context_keys)11, &val);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
			REQUIRE(val == static_cast<uint16_t>(~0xCAFE));
		}

		//write the same value

		UAIR_io_config_write_uint16(&ctx, (uair_io_context_keys)11, static_cast<uint16_t>(~0xCAFE));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		{
			//check that the number of keys remains the same and that no space changed
			UAIR_io_config_stats(&ctx, &stats[2].num_keys, &stats[2].used_space, &stats[2].free_space, &stats[2].recyclable_space);
			REQUIRE(stats[2].num_keys == stats[1].num_keys);
			REQUIRE(stats[2].used_space == stats[1].used_space);
			REQUIRE(stats[2].free_space == stats[1].free_space);
			REQUIRE(stats[2].recyclable_space == stats[1].recyclable_space);
		}
	}

	SECTION("replace uint32")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		struct ConfigStats {
			size_t num_keys, used_space, free_space, recyclable_space;
		};
		
		std::array<ConfigStats, 3> stats;
		UAIR_io_config_stats(&ctx, &stats[0].num_keys, &stats[0].used_space, &stats[0].free_space, &stats[0].recyclable_space);

		UAIR_io_config_write_uint32(&ctx, (uair_io_context_keys)12, static_cast<uint32_t>(~0xC00010FF));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		
		{
			//check that the number of keys remain the same, but new space was allocated
			UAIR_io_config_stats(&ctx, &stats[1].num_keys, &stats[1].used_space, &stats[1].free_space, &stats[1].recyclable_space);
			REQUIRE(stats[1].num_keys == stats[0].num_keys);
			REQUIRE(stats[1].used_space > stats[0].used_space);
			REQUIRE(stats[1].free_space < stats[0].free_space);
			REQUIRE(stats[1].recyclable_space > stats[0].recyclable_space);
		}

		{
			//check the value written
			uint32_t val;
			UAIR_io_config_read_uint32(&ctx, (uair_io_context_keys)12, &val);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
			REQUIRE(val == static_cast<uint32_t>(~0xC00010FF));
		}

		//write the same value

		UAIR_io_config_write_uint32(&ctx, (uair_io_context_keys)12, static_cast<uint32_t>(~0xC00010FF));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		{
			//check that the number of keys remains the same and that no space changed
			UAIR_io_config_stats(&ctx, &stats[2].num_keys, &stats[2].used_space, &stats[2].free_space, &stats[2].recyclable_space);
			REQUIRE(stats[2].num_keys == stats[1].num_keys);
			REQUIRE(stats[2].used_space == stats[1].used_space);
			REQUIRE(stats[2].free_space == stats[1].free_space);
			REQUIRE(stats[2].recyclable_space == stats[1].recyclable_space);
		}
	}

	SECTION("replace uint64")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		struct ConfigStats {
			size_t num_keys, used_space, free_space, recyclable_space;
		};
		
		std::array<ConfigStats, 3> stats;
		UAIR_io_config_stats(&ctx, &stats[0].num_keys, &stats[0].used_space, &stats[0].free_space, &stats[0].recyclable_space);

		UAIR_io_config_write_uint64(&ctx, (uair_io_context_keys)13, static_cast<uint64_t>(~0xC00010FFC00010FF));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		
		{
			//check that the number of keys remain the same, but new space was allocated
			UAIR_io_config_stats(&ctx, &stats[1].num_keys, &stats[1].used_space, &stats[1].free_space, &stats[1].recyclable_space);
			REQUIRE(stats[1].num_keys == stats[0].num_keys);
			REQUIRE(stats[1].used_space > stats[0].used_space);
			REQUIRE(stats[1].free_space < stats[0].free_space);
			REQUIRE(stats[1].recyclable_space > stats[0].recyclable_space);
		}

		{
			//check the value written
			uint64_t val;
			UAIR_io_config_read_uint64(&ctx, (uair_io_context_keys)13, &val);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
			REQUIRE(val == static_cast<uint64_t>(~0xC00010FFC00010FF));
		}

		//write the same value

		UAIR_io_config_write_uint64(&ctx, (uair_io_context_keys)13, static_cast<uint64_t>(~0xC00010FFC00010FF));
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		{
			//check that the number of keys remains the same and that no space changed
			UAIR_io_config_stats(&ctx, &stats[2].num_keys, &stats[2].used_space, &stats[2].free_space, &stats[2].recyclable_space);
			REQUIRE(stats[2].num_keys == stats[1].num_keys);
			REQUIRE(stats[2].used_space == stats[1].used_space);
			REQUIRE(stats[2].free_space == stats[1].free_space);
			REQUIRE(stats[2].recyclable_space == stats[1].recyclable_space);
		}
	}
}