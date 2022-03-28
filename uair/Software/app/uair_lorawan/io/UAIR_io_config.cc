#include "UAIR_io_config.h"

#include <cstdint>
#include <functional>

#include <UAIR_BSP_flash.h>

namespace
{
     #pragma pack(push, 1)
     struct PageHeader
     {
          bool is_unused : 1;
          uint64_t reserved : 63;
     };
     #pragma pack(pop)
     static_assert(sizeof(PageHeader) == 8); //the minimum that the flash can write

     enum class EntryType { None, Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64, BlobStart, BlobMiddle, BlobEnd };

     #pragma pack(push, 1)
     struct EntryHeader
     {
          bool is_unused : 1;
          EntryType type : 4;
          uint8_t id : 8;
          uint32_t reserved : 19;

          uint32_t data;
     };
     #pragma pack(pop)
     static_assert(sizeof(EntryHeader) == 8); //the minimum that the flash can write

     bool pages_iterate(const std::function<void(const PageHeader& page_header, uint32_t page_offset)> cb)
     {
          PageHeader page_header;

          auto num_pages = UAIR_BSP_flash_config_area_get_page_count();
          for (decltype(num_pages) i = 0; i < num_pages; i++)
          {
               auto page_offset = i * BSP_FLASH_PAGE_SIZE;
               if (!UAIR_BSP_flash_config_area_read(page_offset, reinterpret_cast<uint8_t*>(&page_header), sizeof(PageHeader)))
                    return false;

               if (page_header.is_unused)
                    continue;

               cb(page_header, page_offset);
          }

          return true;
     }

     bool entries_iterate(const std::function<void(const EntryHeader& entry_header)> cb)
     {
          pages_iterate([&cb](const PageHeader& page_header, uint32_t page_offset)
          {
               page_offset += sizeof(PageHeader);

               EntryHeader entry_header;
               if (!UAIR_BSP_flash_config_area_read(page_offset, reinterpret_cast<uint8_t*>(&entry_header), sizeof(EntryHeader)))
                    return;

               if (entry_header.is_unused)
                    return;

               cb(entry_header);
          });

          return true;
     }
}

uair_io_config_key_type UAIR_io_config_check_key(uair_io_context* ctx, uair_io_context_keys key)
{
     if (!ctx) return UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE;

     return UAIR_IO_CONFIG_KEY_TYPE_UNKNOWN;
}

void UAIR_io_config_read_uint8(uair_io_context* ctx, uair_io_context_keys key, uint8_t* out)
{
     if (!ctx) return;

     entries_iterate([&key](const EntryHeader& entry_header)
     {
          if (entry_header.type == EntryType::None) return;
          if (entry_header.id != static_cast<uint8_t>(key)) return;
     });
}

void UAIR_io_config_read_uint16(uair_io_context* ctx, uair_io_context_keys key, uint16_t* out)
{
     if (!ctx) return;

}

void UAIR_io_config_read_uint32(uair_io_context* ctx, uair_io_context_keys key, uint32_t* out)
{
     if (!ctx) return;

}

void UAIR_io_config_read_uint64(uair_io_context* ctx, uair_io_context_keys key, uint64_t* out)
{
     if (!ctx) return;

}

size_t UAIR_io_config_read_blob(uair_io_context* ctx, uair_io_context_keys key, void* out, size_t out_max_size)
{
     if (!ctx) return 0;

     return 0;
}

size_t UAIR_io_config_read_blob_size(uair_io_context* ctx, uair_io_context_keys key)
{
     if (!ctx) return 0;

     return 0;
}

void UAIR_io_config_write_uint8(uair_io_context* ctx, uair_io_context_keys key, const uint8_t in)
{
     if (!ctx) return;

}

void UAIR_io_config_write_uint16(uair_io_context* ctx, uair_io_context_keys key, const uint16_t in)
{
     if (!ctx) return;

}

void UAIR_io_config_write_uint32(uair_io_context* ctx, uair_io_context_keys key, const uint32_t in)
{
     if (!ctx) return;

}

void UAIR_io_config_write_uint64(uair_io_context* ctx, uair_io_context_keys key, const uint64_t in)
{
     if (!ctx) return;

}

void UAIR_io_config_write_blob(uair_io_context* ctx, uair_io_context_keys key, const void* in, size_t in_size)
{
     if (!ctx) return;

}

void UAIR_io_config_flush(uair_io_context* ctx)
{
     if (!ctx) return;

}
