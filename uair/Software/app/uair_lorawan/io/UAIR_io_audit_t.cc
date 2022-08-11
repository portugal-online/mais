#include "UAIR_io_audit.h"

#include <UAIR_BSP_flash.h>

#include <array>
#include <random>
#include <cstring>
#include <algorithm>

#include <catch2/catch.hpp>

namespace
{
	int count_entries(uair_io_context* ctx)
	{
		int cur_id = UAIR_io_audit_iter_begin(ctx);

		int num_entries = 0;
		while(cur_id > 0)
		{
			num_entries++;
			cur_id = UAIR_io_audit_iter_next(ctx, cur_id);
		}

		return num_entries;
	}
}

TEST_CASE("UAIR IO audit", "[BSP][BSP app][BSP IO][BSP audit]")
{
	auto page_count = UAIR_BSP_flash_audit_area_get_page_count();
	INFO("Audit page count: " << page_count);
    INFO("Audit page size: " << BSP_FLASH_PAGE_SIZE);
    REQUIRE(page_count >= 2);
    REQUIRE(((page_count * BSP_FLASH_PAGE_SIZE) % sizeof(uint64_t)) == 0);

    constexpr std::array<uint8_t, 1> data_1 = {{ 0xDE }};
    constexpr std::array<uint8_t, 2> data_2 = {{ 0x56, 0xDE }};
    constexpr std::array<uint8_t, 3> data_3 = {{ 0xA3, 0x56, 0xDE }};
    constexpr std::array<uint8_t, 4> data_4 = {{ 0x03, 0xA3, 0x56, 0xDE }};
    constexpr std::array<uint8_t, 5> data_5 = {{ 0x49, 0x03, 0xA3, 0x56, 0xDE }};

    SECTION("clear flash")
	{
		for (decltype(page_count) page_index = 0; page_index < page_count; page_index++)
			UAIR_BSP_flash_audit_area_erase_page(page_index);
	}

	SECTION("write (no data)")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		//these are invalid uses of the API
		REQUIRE(UAIR_io_audit_add(&ctx, nullptr, 0) == 0);
		REQUIRE(static_cast<uair_io_context_audit_errors>(ctx.error) == UAIR_IO_AUDIT_ERROR_INVALID_DATA_SIZE);
		REQUIRE(UAIR_io_audit_add(&ctx, data_1.data(), -1) == 0);
		REQUIRE(static_cast<uair_io_context_audit_errors>(ctx.error) == UAIR_IO_AUDIT_ERROR_INVALID_DATA_SIZE);
		REQUIRE(UAIR_io_audit_add(&ctx, nullptr, data_1.size()) == 0);
		REQUIRE(static_cast<uair_io_context_audit_errors>(ctx.error) == UAIR_IO_AUDIT_ERROR_INVALID_DATA_SIZE);
		REQUIRE(UAIR_io_audit_add(&ctx, data_1.data(), std::numeric_limits<uint16_t>::max()) == 0);
		REQUIRE(static_cast<uair_io_context_audit_errors>(ctx.error) == UAIR_IO_AUDIT_ERROR_INVALID_DATA_SIZE);		
	}
	
	SECTION("write simple")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		std::array<int, 5> ids;
		ids[0] = UAIR_io_audit_add(&ctx, data_1.data(), data_1.size());
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		ids[1] = UAIR_io_audit_add(&ctx, data_2.data(), data_2.size());
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		ids[2] = UAIR_io_audit_add(&ctx, data_3.data(), data_3.size());
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		ids[3] = UAIR_io_audit_add(&ctx, data_4.data(), data_4.size());
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		ids[4] = UAIR_io_audit_add(&ctx, data_5.data(), data_5.size());
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		REQUIRE(count_entries(&ctx) == 5);

		for (size_t i = 0; i < ids.size(); i++)
		{
			auto entry_size = UAIR_io_audit_retrieve(&ctx, ids[i], nullptr);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

			switch(i)
			{
			case 0:
				REQUIRE(entry_size == data_1.size()); break;
			case 1:
				REQUIRE(entry_size == data_2.size()); break;
			case 2:
				REQUIRE(entry_size == data_3.size()); break;
			case 3:
				REQUIRE(entry_size == data_4.size()); break;
			case 4:
				REQUIRE(entry_size == data_5.size()); break;
			}
		}

		for (size_t i = 0; i < ids.size(); i++)
		{
			std::array<uint8_t, data_5.size() + 1> data_tmp;
			std::fill(data_tmp.begin(), data_tmp.end(), 0);

			auto entry_size = UAIR_io_audit_retrieve(&ctx, ids[i], data_tmp.data());
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

			switch(i)
			{
			case 0:
				REQUIRE(entry_size == data_1.size());
				REQUIRE(std::memcmp(data_tmp.data(), data_1.data(), entry_size) == 0);
				REQUIRE(std::all_of(data_tmp.data() + entry_size, data_tmp.data() + data_tmp.size(), [](auto value){ return (value == 0);}) == true);
				break;
			case 1:
				REQUIRE(entry_size == data_2.size());
				REQUIRE(std::memcmp(data_tmp.data(), data_2.data(), entry_size) == 0);
				REQUIRE(std::all_of(data_tmp.data() + entry_size, data_tmp.data() + data_tmp.size(), [](auto value){ return (value == 0);}) == true);
				break;
			case 2:
				REQUIRE(entry_size == data_3.size());
				REQUIRE(std::memcmp(data_tmp.data(), data_3.data(), entry_size) == 0);
				REQUIRE(std::all_of(data_tmp.data() + entry_size, data_tmp.data() + data_tmp.size(), [](auto value){ return (value == 0);}) == true);
				break;
			case 3:
				REQUIRE(entry_size == data_4.size());
				REQUIRE(std::memcmp(data_tmp.data(), data_4.data(), entry_size) == 0);
				REQUIRE(std::all_of(data_tmp.data() + entry_size, data_tmp.data() + data_tmp.size(), [](auto value){ return (value == 0);}) == true);
				break;
			case 4:
				REQUIRE(entry_size == data_5.size());
				REQUIRE(std::memcmp(data_tmp.data(), data_5.data(), entry_size) == 0);
				REQUIRE(std::all_of(data_tmp.data() + entry_size, data_tmp.data() + data_tmp.size(), [](auto value){ return (value == 0);}) == true);
				break;
			}
		}
	}

	SECTION("write big")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		{
			std::array<uint8_t, 512> buffer, buffer_tmp;
			{
				std::mt19937 mt{ std::random_device()() };
				std::uniform_int_distribution<uint8_t> dist;
				for (auto& e : buffer)
					e = dist(mt);

				std::fill(buffer_tmp.begin(), buffer_tmp.end(), 0);
			}

			auto id = UAIR_io_audit_add(&ctx, buffer.data(), buffer.size());
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

			auto entry_size = UAIR_io_audit_retrieve(&ctx, id, nullptr);
			REQUIRE(entry_size == buffer.size());

			entry_size = UAIR_io_audit_retrieve(&ctx, id, buffer_tmp.data());
			REQUIRE(entry_size == buffer.size());
			REQUIRE(std::memcmp(buffer.data(), buffer_tmp.data(), entry_size) == 0);
		}

		{
			std::array<uint8_t, 679> buffer, buffer_tmp;
			{
				std::mt19937 mt{ std::random_device()() };
				std::uniform_int_distribution<uint8_t> dist;
				for (auto& e : buffer)
					e = dist(mt);

				std::fill(buffer_tmp.begin(), buffer_tmp.end(), 0);
			}

			auto id = UAIR_io_audit_add(&ctx, buffer.data(), buffer.size());
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

			auto entry_size = UAIR_io_audit_retrieve(&ctx, id, nullptr);
			REQUIRE(entry_size == buffer.size());

			entry_size = UAIR_io_audit_retrieve(&ctx, id, buffer_tmp.data());
			REQUIRE(entry_size == buffer.size());
			REQUIRE(std::memcmp(buffer.data(), buffer_tmp.data(), entry_size) == 0);
		}

		{
			std::array<uint8_t, 1024> buffer, buffer_tmp;
			{
				std::mt19937 mt{ std::random_device()() };
				std::uniform_int_distribution<uint8_t> dist;
				for (auto& e : buffer)
					e = dist(mt);

				std::fill(buffer_tmp.begin(), buffer_tmp.end(), 0);
			}

			auto id = UAIR_io_audit_add(&ctx, buffer.data(), buffer.size());
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

			auto entry_size = UAIR_io_audit_retrieve(&ctx, id, nullptr);
			REQUIRE(entry_size == buffer.size());

			entry_size = UAIR_io_audit_retrieve(&ctx, id, buffer_tmp.data());
			REQUIRE(entry_size == buffer.size());
			REQUIRE(std::memcmp(buffer.data(), buffer_tmp.data(), entry_size) == 0);
		}

		{
			std::array<uint8_t, 1025> buffer, buffer_tmp;
			{
				std::mt19937 mt{ std::random_device()() };
				std::uniform_int_distribution<uint8_t> dist;
				for (auto& e : buffer)
					e = dist(mt);

				std::fill(buffer_tmp.begin(), buffer_tmp.end(), 0);
			}

			auto id = UAIR_io_audit_add(&ctx, buffer.data(), buffer.size());
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

			auto entry_size = UAIR_io_audit_retrieve(&ctx, id, nullptr);
			REQUIRE(entry_size == buffer.size());

			entry_size = UAIR_io_audit_retrieve(&ctx, id, buffer_tmp.data());
			REQUIRE(entry_size == buffer.size());
			REQUIRE(std::memcmp(buffer.data(), buffer_tmp.data(), entry_size) == 0);
		}
	}

	SECTION("try write huge")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		//this must fail, because with the headers taking a bit of space so we can't use the full space
		{
			size_t target_size = static_cast<size_t>(page_count * BSP_FLASH_PAGE_SIZE);
			std::unique_ptr<uint8_t[]> buffer{new uint8_t[target_size]};

			auto id = UAIR_io_audit_add(&ctx, buffer.get(), target_size);
			REQUIRE(id == 0);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NO_SPACE_AVAILABLE);
		}

		//this must also fail, because we already used up some space with other entries
		{
			size_t target_size = static_cast<size_t>((page_count * BSP_FLASH_PAGE_SIZE) - 2048);
			std::unique_ptr<uint8_t[]> buffer{new uint8_t[target_size]};

			auto id = UAIR_io_audit_add(&ctx, buffer.get(), target_size);
			REQUIRE(id == 0);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NO_SPACE_AVAILABLE);
		}
	}

	SECTION("iterate")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		//we should have 9 entries with ids [1, ..., 9]

		int cur_id = UAIR_io_audit_iter_begin(&ctx);
		REQUIRE(cur_id >= 1);
		REQUIRE(cur_id <= 9);

		int num_entries = 0;
		while(cur_id > 0)
		{
			num_entries++;
			cur_id = UAIR_io_audit_iter_next(&ctx, cur_id);
			
			if (num_entries >= 9) //the last one
			{
				REQUIRE(cur_id == 0);
				continue;
			}
			
			REQUIRE(cur_id >= 1);
			REQUIRE(cur_id <= 9);
		}

		REQUIRE(num_entries == 9);
	}

	SECTION("clear all")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		std::array<int, 9> ids = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		for (auto id : ids)
		{
			UAIR_io_audit_dispose(&ctx, id);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
		}

		REQUIRE(count_entries(&ctx) == 0);
	}

	SECTION("flush")
	{
		uair_io_context ctx;
		UAIR_io_init_ctx(&ctx);

		size_t target_size = static_cast<size_t>((page_count * BSP_FLASH_PAGE_SIZE) - 2048);
		std::unique_ptr<uint8_t[]> buffer{new uint8_t[target_size]};
		std::unique_ptr<uint8_t[]> buffer_tmp{new uint8_t[target_size]};
		{
			std::mt19937 mt{ std::random_device()() };
			std::uniform_int_distribution<uint8_t> dist;
			for (size_t i = 0; i < target_size; ++i)
				buffer[i] = dist(mt);

			std::fill(buffer_tmp.get(), buffer_tmp.get() + target_size, 0);
		}

		//because we already cleared all entries...
		{
			auto id = UAIR_io_audit_add(&ctx, buffer.get(), target_size);
			REQUIRE(id == 0);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_CTX_CHECK);
			REQUIRE(ctx.flags == UAIR_IO_CONTEXT_FLAG_FLUSH);
		}

		//flush
		UAIR_io_audit_flush(&ctx);
		REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);

		//try again
		{
			auto id = UAIR_io_audit_add(&ctx, buffer.get(), target_size);
			REQUIRE(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
			REQUIRE(id > 0);

			auto entry_size = UAIR_io_audit_retrieve(&ctx, id, buffer_tmp.get());
			REQUIRE(entry_size == target_size);
			REQUIRE(std::memcmp(buffer.get(), buffer_tmp.get(), entry_size) == 0);
		}
	}
}
