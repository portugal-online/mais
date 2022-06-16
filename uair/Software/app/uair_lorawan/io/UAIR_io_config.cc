#include "UAIR_io_config.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>

#include "UAIR_BSP_flash.h"
#include "UAIR_tracer.h"

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

     enum EntryType { ENTRY_TYPE_INT8, ENTRY_TYPE_UINT8, ENTRY_TYPE_INT16, ENTRY_TYPE_UINT16, ENTRY_TYPE_INT32, ENTRY_TYPE_UINT32, ENTRY_TYPE_INT64, ENTRY_TYPE_UINT64, ENTRY_TYPE_BLOB_START, ENTRY_TYPE_BLOB_MIDDLE, ENTRY_TYPE_BLOB_END };

     #pragma pack(push, 1)
     struct EntryHeader
     {
          bool is_unused : 1;
          bool is_valid : 1;
          EntryType type : 6;
          uint8_t id : 8;
          uint16_t reserved;

          union {
               int8_t i8;
               uint8_t ui8;
               int16_t i16;
               uint16_t ui16;
               int32_t i32;
               uint32_t ui32;
          } data;

          static void print(const EntryHeader& header)
          {
               LIB_PRINTF("Entry header{ unused: %d, valid: %d, type: %u, id: %d, reserved: %u, data: %lu}\n", header.is_unused, header.is_valid, (uint8_t)header.type, header.id, header.reserved, header.data.ui32);
          }

          static size_t total_size(EntryType type) noexcept
          {
               switch(type)
               {
               case ENTRY_TYPE_BLOB_START:
               case ENTRY_TYPE_BLOB_MIDDLE:
               case ENTRY_TYPE_BLOB_END:
                    assert(!"Unsupported type");
                    return 0;
               case ENTRY_TYPE_INT64:
               case ENTRY_TYPE_UINT64:
                    return (sizeof(EntryHeader) + sizeof(uint64_t));
               default:
                    return sizeof(EntryHeader);
               }
          }

     };
     #pragma pack(pop)
     static_assert(sizeof(EntryHeader) == 8); //the minimum that the flash can write

     struct EntryInfo
     {
          EntryHeader header;
          flash_page_t page_index;
          flash_address_t page_address;

          static void print(const EntryInfo& info)
          {
               EntryHeader::print(info.header);
               LIB_PRINTF("Entry page{ index: %u, address: %lu}\n", info.page_index, info.page_address);
          }
     };

     bool pages_iterate(const std::function<bool(const PageHeader& page_header, flash_page_t page_index)>& cb, bool ignore_unused = true)
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

     bool entries_iterate(flash_page_t target_page_index, const std::function<bool(const EntryInfo& entry_info)>& cb)
     {
          assert((target_page_index >= 0) && (target_page_index < UAIR_BSP_flash_config_area_get_page_count()));

          auto page_address = (static_cast<flash_address_t>(target_page_index) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
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
               info.page_index = target_page_index;
               info.page_address = page_address;
               if (!cb(info)) return false;

               page_address += EntryHeader::total_size(header.type);
          }

          return true;
     }

     bool entries_iterate(const std::function<bool(const EntryInfo& entry_info)>& cb)
     {
          pages_iterate([&cb](const PageHeader&, flash_page_t page_index)
          {
               if (!entries_iterate(page_index, cb))
                    return false; //something went wrong

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
               if (!entry_info.header.is_valid) return true;
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

     template<class T>
     bool entry_values_compare(const T& valA, const T& valB)
     {
          static_assert(std::is_arithmetic<T>::value, "Type must be arithmetic.");

          //best case scenario
          if (valA == valB) return true;

          /**
           * We should be if the bits are compatible. But we can do that later...
           * For now, assume that they aren't.
           */
          return false;
     }

     bool entry_read_ahead(flash_page_t entry_page_index, flash_address_t entry_page_address, void* data_out, size_t data_size)
     {
          auto data_address = entry_page_address + sizeof(EntryHeader);
          assert(data_address < ((entry_page_index * BSP_FLASH_PAGE_SIZE) + BSP_FLASH_PAGE_SIZE));

          return (UAIR_BSP_flash_config_area_read(data_address, reinterpret_cast<uint8_t*>(data_out), data_size) == (int)data_size);
     }

     bool entry_copy(uair_io_context& ctx, flash_address_t source, flash_address_t dest, const EntryHeader& header)
     {
          //we can write the header already (optimization)
          assert(sizeof(EntryHeader) == 8);
          if (UAIR_BSP_flash_config_area_write(dest, (uint64_t*)&header, 1) != 1)
          {
               ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
               return false;
          }

          auto remaining = EntryHeader::total_size(header.type) - sizeof(EntryHeader);
          if (remaining <= 0)
               return true;

          source += sizeof(EntryHeader);
          dest += sizeof(EntryHeader);
          assert((remaining % 8) == 0);

          uint64_t buffer;
          while(remaining > 0)
          {
               if (UAIR_BSP_flash_config_area_read(source, reinterpret_cast<uint8_t*>(&buffer), sizeof(uint64_t)) != sizeof(uint64_t))
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_READ;
                    return false;
               }

               if (UAIR_BSP_flash_config_area_write(dest, &buffer, 1) != 1)
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }

               remaining -= 8;
               source += sizeof(uint64_t);
               dest += sizeof(uint64_t);
          }

          return true;
     }

     bool entry_replace_or_invalidate(uair_io_context& ctx, const EntryHeader& header, const void* extra_data, size_t extra_data_size, bool& replaced)
     {
          ctx.flags = UAIR_IO_CONTEXT_FLAG_NONE;
          ctx.error = UAIR_IO_CONTEXT_ERROR_NONE;
          replaced = false;

          assert(EntryHeader::total_size(header.type) == (sizeof(EntryHeader) + extra_data_size));

          EntryInfo entry_info;
          entries_find_key(ctx, static_cast<uair_io_context_keys>(header.id), header.type, entry_info);
          switch((int)ctx.error)
          {
          case UAIR_IO_CONTEXT_ERROR_NONE: break;
          case UAIR_IO_CONFIG_ERROR_INVALID_KEY: return true; //there's no key (nothing to replace / invalidate)
          default: return false; //error out
          }

          //reaching this point, the key already exists

          //read the data and compare
          {
               switch(header.type)
               {
               case ENTRY_TYPE_INT8:
                    replaced = entry_values_compare(entry_info.header.data.i8, header.data.i8);
                    break;
               case ENTRY_TYPE_UINT8:
                    replaced = entry_values_compare(entry_info.header.data.ui8, header.data.ui8);
                    break;
               case ENTRY_TYPE_INT16:
                    replaced = entry_values_compare(entry_info.header.data.i16, header.data.i16);
                    break;
               case ENTRY_TYPE_UINT16:
                    replaced = entry_values_compare(entry_info.header.data.ui16, header.data.ui16);
                    break;
               case ENTRY_TYPE_INT32:
                    replaced = entry_values_compare(entry_info.header.data.i32, header.data.i32);
                    break;
               case ENTRY_TYPE_UINT32:
                    replaced = entry_values_compare(entry_info.header.data.ui32, header.data.ui32);
                    break;
               case ENTRY_TYPE_INT64:
               {
                    assert(extra_data_size == sizeof(int64_t));

                    int64_t stored_value;
                    if (!entry_read_ahead(entry_info.page_index, entry_info.page_address, &stored_value, sizeof(int64_t)))
                    {
                         ctx.error = static_cast<uair_io_context_errors>(UAIR_IO_CONFIG_ERROR_DATA_ERROR);
                         return false;
                    }

                    replaced = entry_values_compare(stored_value, *reinterpret_cast<const int64_t*>(extra_data));
                    break;
               }
               case ENTRY_TYPE_UINT64:
               {
                    assert(extra_data_size == sizeof(uint64_t));

                    uint64_t stored_value;
                    if (!entry_read_ahead(entry_info.page_index, entry_info.page_address, &stored_value, sizeof(uint64_t)))
                    {
                         ctx.error = static_cast<uair_io_context_errors>(UAIR_IO_CONFIG_ERROR_DATA_ERROR);
                         return false;
                    }

                    replaced = entry_values_compare(stored_value, *reinterpret_cast<const uint64_t*>(extra_data));
                    break;
               }
               default:
                    break;
               }

               //if we can indeed replace the value
               if (replaced)
               {
                    switch(header.type)
                    {
                    case ENTRY_TYPE_INT8:
                    case ENTRY_TYPE_UINT8:
                    case ENTRY_TYPE_INT16:
                    case ENTRY_TYPE_UINT16:
                    case ENTRY_TYPE_INT32:
                    case ENTRY_TYPE_UINT32:
                    {
                         entry_info.header.data = header.data;
                         if (UAIR_BSP_flash_config_area_write(entry_info.page_address, (uint64_t*)&entry_info.header, 1) != 1)
                         {
                              ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                              return false;
                         }
                         break;
                    }
                    case ENTRY_TYPE_INT64:
                    case ENTRY_TYPE_UINT64:
                    {
                         auto data_address = entry_info.page_address + sizeof(EntryHeader);
                         assert(data_address < ((entry_info.page_index * BSP_FLASH_PAGE_SIZE) + BSP_FLASH_PAGE_SIZE));

                         if (UAIR_BSP_flash_config_area_write(data_address, (uint64_t*)extra_data, 1) != 1)
                         {
                              ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                              return false;
                         }

                         break;
                    }
                    default:
                         assert(!"Jump impossible.");
                    }

                    return true;
               }
          }

          //reaching this point, the value can't be replaced, so we must invalidate the entry

          entry_info.header.is_valid = false;
          if (UAIR_BSP_flash_config_area_write(entry_info.page_address, (uint64_t*)&entry_info.header, 1) == 1)
               return true;

          ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
          return false;
     }

     bool entries_write_entry(uair_io_context& ctx, const EntryHeader& header, const void* extra_data, size_t extra_data_size)
     {
          ctx.flags = UAIR_IO_CONTEXT_FLAG_NONE;
          ctx.error = UAIR_IO_CONTEXT_ERROR_NONE;

          assert(EntryHeader::total_size(header.type) == (sizeof(EntryHeader) + extra_data_size));

          struct
          {
               size_t num_free_pages = 0;
               size_t num_invalidated_entries = 0;

               bool has_page_free = false;
               flash_page_t page_free = 0;

               bool has_last_entry = false;
               size_t last_entry_size = 0;
               flash_page_t last_entry_page_index = 0;
               flash_address_t last_entry_page_address = 0;
          } page_info;

          //gather information

          pages_iterate([&page_info, extra_data_size](const PageHeader& page_header, flash_page_t page_index) mutable
          {
               if (page_header.is_unused)
               {
                    page_info.num_free_pages++;

                    if (!page_info.has_page_free)
                    {
                       page_info.has_page_free = true;
                       page_info.page_free = page_index;
                    }

                    return true; //next page
               }

               entries_iterate(page_index, [&page_info](const EntryInfo& entry_info)
               {
                    if (!entry_info.header.is_valid)
                         page_info.num_invalidated_entries++;

                    page_info.has_last_entry = true;
                    page_info.last_entry_size = EntryHeader::total_size(entry_info.header.type);
                    page_info.last_entry_page_index = entry_info.page_index;
                    page_info.last_entry_page_address = entry_info.page_address;

                    return true;
               });

               //if this page still has room, we're done
               if (page_info.has_last_entry)
               {
                    auto page_remaining_space = ((page_info.last_entry_page_index + 1) * BSP_FLASH_PAGE_SIZE) - (page_info.last_entry_page_address + page_info.last_entry_size);
                    if (page_remaining_space >= (sizeof(EntryHeader) + extra_data_size))
                    {
                         page_info.has_page_free = false;
                         return false; //found where we can write
                    }
               }

               //must move on to the next page
               page_info.has_last_entry = false;
               return true;

          }, false);

          //helper method to write an entry header

          auto write_entry_data = [](uair_io_context& ctx, flash_address_t target_address, const EntryHeader& header, const void* extra_data, size_t extra_data_size)
          {
               switch(header.type)
               {
               case ENTRY_TYPE_BLOB_START:
               case ENTRY_TYPE_BLOB_MIDDLE:
               case ENTRY_TYPE_BLOB_END:
                    assert(!"Unsupported type");
                    ctx.error = UAIR_IO_CONTEXT_ERROR_INTERNAL;
                    return false;
               default: break;
               }

               //no extra data in the entry
               if (extra_data_size <= 0)
               {
                    assert(!extra_data);

                    if (UAIR_BSP_flash_config_area_write(target_address, (uint64_t*)&header, 1) == 1)
                         return true;

                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }

               //we have to write the header + extra data

               assert(extra_data);

               #pragma pack(push, 1)
               struct EntryData
               {
                    EntryHeader header;
                    uint64_t extra_data;
               } entry_data;
               #pragma pack(pop)

               static_assert(sizeof(EntryData) == 16);

               assert(sizeof(EntryData) == EntryHeader::total_size(header.type));
               assert(EntryHeader::total_size(header.type) == (sizeof(EntryHeader) + extra_data_size));

               entry_data.header = header;
               memcpy(&entry_data.extra_data, extra_data, extra_data_size);
               if (UAIR_BSP_flash_config_area_write(target_address, (uint64_t*)&entry_data, 2) == 2)
                    return true;

               ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
               return false;
          };

          //if we can write after the last entry we found
          if (page_info.has_last_entry)
               return write_entry_data(ctx, page_info.last_entry_page_address + page_info.last_entry_size, header, extra_data, extra_data_size);

          //reaching this point, we have to write to a new page

          //if we don't have a free page
          if (!page_info.has_page_free)
          {
               ctx.error = UAIR_IO_CONTEXT_ERROR_NO_SPACE_AVAILABLE;
               return false;
          }

          //if we have a free page, but it's the only one
          if (page_info.has_page_free && (page_info.num_free_pages == 1))
          {
               //if there's nothing to clean, we run out of space
               if (page_info.num_invalidated_entries <= 0)
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_NO_SPACE_AVAILABLE;
                    return false;
               }

               //reaching this point, we have a single free page and some trash we can clean
               //so we can still try after a flush
               ctx.flags = UAIR_IO_CONTEXT_FLAG_FLUSH;
               ctx.error = UAIR_IO_CONTEXT_ERROR_CTX_CHECK;
               return false;
          }

          //we can safely write to a new page

          {
               PageHeader page_header;
               page_header.is_unused = false;
               page_header.reserved = 0x7FFFFFFFFFFFFFF;

               if (UAIR_BSP_flash_config_area_write(page_info.page_free * BSP_FLASH_PAGE_SIZE, (uint64_t*)&page_header, 1) != 1)
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }
          }

          return write_entry_data(ctx, (page_info.page_free * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader), header, extra_data, extra_data_size);
     }
}

uair_io_config_key_type UAIR_io_config_check_key(uair_io_context* ctx, uair_io_context_keys key)
{
     if (!ctx) return UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE;

     auto key_type = UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE; //doesn't exist

     entries_iterate([&key_type, &key](const EntryInfo& entry_info)
     {
          if (!entry_info.header.is_valid) return true;
          if (entry_info.header.id != static_cast<uint8_t>(key)) return true;

          switch(entry_info.header.type)
          {
          case ENTRY_TYPE_UINT8:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT8; break;
          case ENTRY_TYPE_UINT16:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT16; break;
          case ENTRY_TYPE_UINT32:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT32; break;
          case ENTRY_TYPE_UINT64:
               key_type = UAIR_IO_CONFIG_KEY_TYPE_UINT64; break;
          case ENTRY_TYPE_INT8:
          case ENTRY_TYPE_INT16:
          case ENTRY_TYPE_INT32:
          case ENTRY_TYPE_INT64:
          case ENTRY_TYPE_BLOB_START:
          case ENTRY_TYPE_BLOB_MIDDLE:
          case ENTRY_TYPE_BLOB_END:
               assert(!"Unsupported type"); break;
          }

          return false; //stop iteration
     });

     return key_type;
}

void UAIR_io_config_stats(uair_io_context* ctx, size_t* num_keys, size_t* used_space, size_t* free_space, size_t* recyclable_space)
{
     if (!ctx) return;

     struct {
          size_t num_keys = 0;
          size_t used_space = 0;
          size_t free_space = 0;
          size_t recyclable_space = 0;
     } info;

     pages_iterate([&info](const PageHeader& page_header, flash_page_t page_index)
     {
          if (page_header.is_unused)
          {
               info.free_space += BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);
               return true;
          }

          size_t last_page_address = 0;
          entries_iterate(page_index, [&info, &last_page_address](const EntryInfo& entry_info) mutable
          {
               auto entry_size = EntryHeader::total_size(entry_info.header.type);

               if (entry_info.header.is_valid)
                    info.num_keys++;
               else
                    info.recyclable_space += entry_size;

               info.used_space += entry_size;
               last_page_address = entry_info.page_address + entry_size;

               return true;
          });

          info.free_space += ((page_index + 1) * BSP_FLASH_PAGE_SIZE) - last_page_address;

          return true;
     });

     if (num_keys) *num_keys = info.num_keys;
     if (used_space) *used_space = info.used_space;
     if (free_space) *free_space = info.free_space;
     if (recyclable_space) *recyclable_space = info.recyclable_space;
}

void UAIR_io_config_read_uint8(uair_io_context* ctx, uair_io_context_keys key, uint8_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, ENTRY_TYPE_UINT8, entry);
     if (ctx->error || !out) return;

     *out = entry.header.data.ui8;
}

void UAIR_io_config_read_uint16(uair_io_context* ctx, uair_io_context_keys key, uint16_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, ENTRY_TYPE_UINT16, entry);
     if (ctx->error || !out) return;

     *out = entry.header.data.ui16;
}

void UAIR_io_config_read_uint32(uair_io_context* ctx, uair_io_context_keys key, uint32_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, ENTRY_TYPE_UINT32, entry);
     if (ctx->error || !out) return;

     *out = entry.header.data.ui32;
}

void UAIR_io_config_read_uint64(uair_io_context* ctx, uair_io_context_keys key, uint64_t* out)
{
     if (!ctx) return;

     EntryInfo entry;
     entries_find_key(*ctx, key, ENTRY_TYPE_UINT64, entry);
     if (ctx->error || !out) return;

     //this key type has the data on the next 64bits (not in-situ)
     if (!entry_read_ahead(entry.page_index, entry.page_address, out, sizeof(uint64_t)))
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

     EntryHeader header;
     header.is_unused = 0;
     header.is_valid = 1;
     header.type = ENTRY_TYPE_UINT8;
     header.id = static_cast<uint8_t>(key);
     header.reserved = 0xFFFF;
     header.data.ui8 = in;

     bool replaced;
     if (!entry_replace_or_invalidate(*ctx, header, nullptr, 0, replaced) || replaced)
          return;

     entries_write_entry(*ctx, header, nullptr, 0);
}

void UAIR_io_config_write_uint16(uair_io_context* ctx, uair_io_context_keys key, const uint16_t in)
{
     if (!ctx) return;

     EntryHeader header;
     header.is_unused = 0;
     header.is_valid = 1;
     header.type = ENTRY_TYPE_UINT16;
     header.id = static_cast<uint8_t>(key);
     header.reserved = 0xFFFF;
     header.data.ui16 = in;

     bool replaced;
     if (!entry_replace_or_invalidate(*ctx, header, nullptr, 0, replaced) || replaced)
          return;

     entries_write_entry(*ctx, header, nullptr, 0);
}

void UAIR_io_config_write_uint32(uair_io_context* ctx, uair_io_context_keys key, const uint32_t in)
{
     if (!ctx) return;

     EntryHeader header;
     header.is_unused = 0;
     header.is_valid = 1;
     header.type = ENTRY_TYPE_UINT32;
     header.id = static_cast<uint8_t>(key);
     header.reserved = 0xFFFF;
     header.data.ui32 = in;

     bool replaced;
     if (!entry_replace_or_invalidate(*ctx, header, nullptr, 0, replaced) || replaced)
          return;

     entries_write_entry(*ctx, header, nullptr, 0);
}

void UAIR_io_config_write_uint64(uair_io_context* ctx, uair_io_context_keys key, const uint64_t in)
{
     if (!ctx) return;

     EntryHeader header;
     header.is_unused = 0;
     header.is_valid = 1;
     header.type = ENTRY_TYPE_UINT64;
     header.id = static_cast<uint8_t>(key);
     header.reserved = 0xFFFF;
     header.data.ui32 = 0xFFFFFFFF;

     bool replaced;
     if (!entry_replace_or_invalidate(*ctx, header, &in, sizeof(uint64_t), replaced) || replaced)
          return;

     entries_write_entry(*ctx, header, &in, sizeof(uint64_t));
}

void UAIR_io_config_write_blob(uair_io_context* ctx, uair_io_context_keys key, const void* in, size_t in_size)
{
     if (!ctx) return;
}

void UAIR_io_config_remove(uair_io_context* ctx, uair_io_context_keys key)
{
     if (!ctx) return;

     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     //search the entry (type doesn't matter)
     EntryInfo entry_info;
     entries_find_key(*ctx, static_cast<uair_io_context_keys>(key), ENTRY_TYPE_INT8, entry_info);
     switch((int)ctx->error)
     {
     case UAIR_IO_CONTEXT_ERROR_NONE:
          break;
     case UAIR_IO_CONFIG_ERROR_KEY_TYPE_MISMATCH:
          ctx->error = UAIR_IO_CONTEXT_ERROR_NONE; //for this, we don't care about the type
          break;
     case UAIR_IO_CONFIG_ERROR_INVALID_KEY:
          //key doesn't exist, we can leave
          ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;
          return;
     default: return; //error out
     }

     //invalidate entry
     entry_info.header.is_valid = false;
     if (UAIR_BSP_flash_config_area_write(entry_info.page_address, (uint64_t*)&entry_info.header, 1) != 1)
          ctx->error = UAIR_IO_CONTEXT_ERROR_WRITE;
}

void UAIR_io_config_flush(uair_io_context* ctx)
{
     if (!ctx) return;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     //collect some stats

     struct
     {
          int16_t page_free = -1;

          int16_t page_to_clean = -1;
          size_t page_to_clean_used_size = 0;
          size_t page_to_clean_delete_size = 0;

          int16_t page_defrag = -1;
          size_t page_defrag_used_size = std::numeric_limits<size_t>::max();
     } stats;

     pages_iterate([&stats](const PageHeader& page_header, flash_page_t page_index) mutable
     {
          if (page_header.is_unused)
          {
               if (stats.page_free == -1)
                  stats.page_free = page_index;

               return true;
          }

          auto page_address = (static_cast<flash_address_t>(page_index) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
          auto page_address_end = page_address + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

          size_t size_used = 0, size_deleted = 0;
          while (page_address < page_address_end)
          {
               EntryHeader header;
               if (UAIR_BSP_flash_config_area_read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(EntryHeader)) != sizeof(EntryHeader))
                    return false;

               if (header.is_unused)
                    break; //no more entries written

               if (!header.is_valid)
                    size_deleted += EntryHeader::total_size(header.type);
               else
                    size_used += EntryHeader::total_size(header.type);

               page_address += EntryHeader::total_size(header.type);
          }

          //pick the best one to clean (with the most delete entries)
          if ((size_deleted > 0) && (size_deleted > stats.page_to_clean_delete_size))
          {
               stats.page_to_clean = page_index;
               stats.page_to_clean_used_size = size_used;
               stats.page_to_clean_delete_size = size_deleted;
          }

          //pick the best de-fragmented one (not an entry deleted and with the least used space)
          if ((size_deleted <= 0) && (size_used < stats.page_defrag_used_size))
          {
               stats.page_defrag = page_index;
               stats.page_defrag_used_size = size_used;
          }

          return true;

     }, false);

     //there's nothing to clean (assume success)
     if (stats.page_to_clean == -1)
          return;

     //if the amount of space to clean fits into an existing page, do it...
     if ((stats.page_defrag >= 0) && (stats.page_to_clean_used_size <= (BSP_FLASH_PAGE_SIZE - sizeof(PageHeader) - stats.page_defrag_used_size)))
     {
          auto src_begin = (static_cast<flash_address_t>(stats.page_to_clean) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
          auto src_end = src_begin + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

          auto dst_begin = (static_cast<flash_address_t>(stats.page_defrag) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader) + stats.page_defrag_used_size;
          auto dst_end = (static_cast<flash_address_t>(stats.page_defrag) * BSP_FLASH_PAGE_SIZE) + BSP_FLASH_PAGE_SIZE;

          while (src_begin < src_end)
          {
               assert(src_begin < src_end);
               assert(dst_begin < dst_end);

               EntryHeader header;
               if (UAIR_BSP_flash_config_area_read(src_begin, reinterpret_cast<uint8_t*>(&header), sizeof(EntryHeader)) != sizeof(EntryHeader))
                    return;

               if (header.is_unused)
                    break; //no more entries to read

               if (header.is_valid)
               {
                    //copy entry
                    if (!entry_copy(*ctx, src_begin, dst_begin, header))
                         return; //something went wrong

                    dst_begin += EntryHeader::total_size(header.type);
               }

               src_begin += EntryHeader::total_size(header.type);
          }

          //finally erase the page we read from

          if (UAIR_BSP_flash_config_area_erase_page(stats.page_to_clean) != BSP_ERROR_NONE)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_INTERNAL;
               return;
          }
     }
     else
     {
          //reaching this point, we need a free page to work with
          if (stats.page_free == -1)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_FLUSH_NO_FREE_PAGE;
               return;
          }

          auto dst_begin = (static_cast<flash_address_t>(stats.page_free) * BSP_FLASH_PAGE_SIZE);
          auto dst_end = dst_begin + BSP_FLASH_PAGE_SIZE;

          //write the page header
          {
               PageHeader page_header;
               page_header.is_unused = false;
               page_header.reserved = 0x7FFFFFFFFFFFFFF;

               if (UAIR_BSP_flash_config_area_write(dst_begin, (uint64_t*)&page_header, 1) != 1)
               {
                    ctx->error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return;
               }

               dst_begin += sizeof(PageHeader);
          }

          //copy everything still valid to the new page

          auto src_begin = (static_cast<flash_address_t>(stats.page_to_clean) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
          auto src_end = src_begin + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

          while (src_begin < src_end)
          {
               assert(src_begin < src_end);
               assert(dst_begin < dst_end);

               EntryHeader header;
               if (UAIR_BSP_flash_config_area_read(src_begin, reinterpret_cast<uint8_t*>(&header), sizeof(EntryHeader)) != sizeof(EntryHeader))
               {
                    ctx->error = UAIR_IO_CONTEXT_ERROR_READ;
                    return;
               }

               EntryHeader::print(header);

               if (header.is_unused)
                    break; //no more entries to read

               if (header.is_valid)
               {
                    //copy entry
                    if (!entry_copy(*ctx, src_begin, dst_begin, header))
                         return; //something went wrong

                    dst_begin += EntryHeader::total_size(header.type);
               }

               src_begin += EntryHeader::total_size(header.type);
          }

          //finally erase the page we read from

          if (UAIR_BSP_flash_config_area_erase_page(stats.page_to_clean) != BSP_ERROR_NONE)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_INTERNAL;
               return;
          }
     }

     //all done... to make sure everything is optimal, just try again
     //it should detect there's nothing to clean, and simply leave
     UAIR_io_config_flush(ctx);
}
