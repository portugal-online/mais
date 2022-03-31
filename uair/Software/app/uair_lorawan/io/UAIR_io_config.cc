#include "UAIR_io_config.h"

#include <cstdint>
#include <cassert>
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
          EntryType type : 7;
          uint8_t id : 8;
          uint16_t reserved;

          uint32_t data;
     };
     #pragma pack(pop)
     static_assert(sizeof(EntryHeader) == 8); //the minimum that the flash can write

     struct EntryInfo
     {
          EntryHeader header;
          flash_page_t page_index;
          flash_address_t page_address;
     };

     bool pages_iterate(const std::function<bool(const PageHeader& page_header, flash_page_t page_index)> cb, bool ignore_unused = true)
     {
          PageHeader page_header;

          auto num_pages = UAIR_BSP_flash_config_area_get_page_count();
          for (decltype(num_pages) i = 0; i < num_pages; i++)
          {
               if (UAIR_BSP_flash_config_area_read(i * BSP_FLASH_PAGE_SIZE, reinterpret_cast<uint8_t*>(&page_header), sizeof(PageHeader)) != sizeof(PageHeader))
                    return false;

               if (ignore_unused && page_header.is_unused)
                    continue; //this page was never written to (it's empty)

               if (!cb(page_header, i)) break;
          }

          return true;
     }

     bool entries_iterate(const std::function<bool(const EntryInfo& entry_info)> cb)
     {
          pages_iterate([&cb](const PageHeader& page_header, flash_page_t page_index)
          {
               auto page_address = (static_cast<flash_address_t>(page_index) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
               auto page_address_end = page_address + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

               while (page_address < page_address_end)
               {
                    EntryHeader header;
                    if (UAIR_BSP_flash_config_area_read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(EntryHeader)) != sizeof(EntryHeader))
                         return false;

                    if (header.is_unused)
                         return true; //this entry was never written: the entry before this one was the last of the page

                    EntryInfo info;
                    info.header = header;
                    info.page_index = page_index;
                    info.page_address = page_address;
                    if (!cb(info)) return false;

                    page_address += sizeof(EntryHeader);

                    //some entries don't have the data written into the header
                    switch(info.header.type)
                    {
                    case EntryType::Int64:
                    case EntryType::UInt64:
                         page_address += sizeof(uint64_t);
                         break;
                    default:
                         break;
                    }
               }

               return true; //continue to the next page
          });

          return true;
     }

     void entries_find_key(uair_io_context& ctx, uair_io_context_keys key_id, EntryType key_type, EntryInfo& target_entry)
     {
          ctx.flags = UAIR_IO_CONTEXT_FLAG_NONE;
          ctx.error = UAIR_IO_CONTEXT_ERROR_NONE;

          bool found = false;
          entries_iterate([&found, &target_entry, &key_id](const EntryInfo& entry_info)
          {
               if (entry_info.header.type == EntryType::None) return true;
               if (entry_info.header.id != static_cast<uint8_t>(key_id)) return true;

               found = true;
               target_entry = entry_info;

               return false; //stop iteration
          });

          if (!found)
          {
               ctx.error = static_cast<uair_io_context_errors>(UAIR_IO_CONFIG_ERROR_INVALID_KEY);
               return;
          }

          if (target_entry.header.type != key_type)
          {
               ctx.error = static_cast<uair_io_context_errors>(UAIR_IO_CONFIG_ERROR_KEY_TYPE_MISMATCH);
               return;
          }
     }
}

uair_io_config_key_type UAIR_io_config_check_key(uair_io_context* ctx, uair_io_context_keys key)
{
     if (!ctx) return UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE;

     auto key_type = UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE; //doesn't exist

     entries_iterate([&key_type, &key](const EntryInfo& entry_info)
     {
          if (entry_info.header.type == EntryType::None) return true;
          if (entry_info.header.id != static_cast<uint8_t>(key)) return true;

          switch(entry_info.header.type)
          {
          case EntryType::UInt8:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT8; break;
          case EntryType::UInt16:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT16; break;
          case EntryType::UInt32:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT32; break;
          case EntryType::UInt64:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT64; break;
          case EntryType::None:
          case EntryType::Int8:
          case EntryType::Int16:
          case EntryType::Int32:
          case EntryType::Int64:
          case EntryType::BlobStart:
          case EntryType::BlobMiddle:
          case EntryType::BlobEnd:
               assert(!"Unsupported type");
               break;
          }

          return false; //stop iteration
     });

     return key_type;
}

void UAIR_io_config_read_uint8(uair_io_context* ctx, uair_io_context_keys key, uint8_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, EntryType::UInt8, entry);
     if (ctx->error || !out) return;

     *out = *reinterpret_cast<uint8_t*>(&entry.header.data);
}

void UAIR_io_config_read_uint16(uair_io_context* ctx, uair_io_context_keys key, uint16_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, EntryType::UInt16, entry);
     if (ctx->error || !out) return;

     *out = *reinterpret_cast<uint16_t*>(&entry.header.data);
}

void UAIR_io_config_read_uint32(uair_io_context* ctx, uair_io_context_keys key, uint32_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, EntryType::UInt32, entry);
     if (ctx->error || !out) return;

     *out = entry.header.data;
}

void UAIR_io_config_read_uint64(uair_io_context* ctx, uair_io_context_keys key, uint64_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, EntryType::UInt64, entry);
     if (ctx->error || !out) return;

     //this key type has the data on the next 64bits (not in-situ)
     auto data_address = entry.page_address + sizeof(EntryHeader);
     assert(data_address < ((entry.page_index * BSP_FLASH_PAGE_SIZE) + BSP_FLASH_PAGE_SIZE));

     if (UAIR_BSP_flash_config_area_read(data_address, reinterpret_cast<uint8_t*>(out), sizeof(uint64_t)) != sizeof(uint64_t))
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_CONFIG_ERROR_DATA_ERROR);
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
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     struct
     {
          int16_t page_free = -1;
          int16_t page_to_clean = -1;
          size_t page_to_clean_num_entries = 0;
     } stats;

     pages_iterate([&stats](const PageHeader& page_header, flash_page_t page_index) mutable
     {
          if (page_header.is_unused)
          {
               if (stats.page_free == -1)
                  stats.page_free = page_index;
          }
          else
          {
               auto page_address = (static_cast<flash_address_t>(page_index) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
               auto page_address_end = page_address + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

               size_t num_entries_deleted;
               while (page_address < page_address_end)
               {
                    EntryHeader header;
                    if (UAIR_BSP_flash_config_area_read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(EntryHeader)) != sizeof(EntryHeader))
                         return false;

                    if (header.is_unused)
                         break; //no more entries written

                    if (header.type == EntryType::None)
                         num_entries_deleted++;
               }

               if ((num_entries_deleted > 0) && (num_entries_deleted > stats.page_to_clean_num_entries))
               {
                    stats.page_to_clean = page_index;
                    stats.page_to_clean_num_entries = num_entries_deleted;
               }
          }

          return true;

     }, false);

     //there's nothing to clean
     if (stats.page_to_clean == -1)
          return;

     //there's stuff to clean, but there's no free page
     if (stats.page_free == -1)
     {
          ctx->error = UAIR_IO_CONTEXT_ERROR_CTX_FLUSH_NO_FREE_PAGE;
          return;
     }
}
