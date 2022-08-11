#include "UAIR_io_audit.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>

#include "UAIR_BSP_flash.h"
#include "UAIR_tracer.h"

namespace
{
     /* this limits the max size of an audit part: we don't want large parts (which
     would make management difficult) but also not small parts (it would cause a lot
     of fragmentation) */
     constexpr size_t MaxPartSize = BSP_FLASH_PAGE_SIZE / 4;
     static_assert((MaxPartSize % 8) == 0); //we need this to be multiple of 8 bytes

     #pragma pack(push, 1)
     struct PageHeader
     {
          bool is_unused : 1;
          uint64_t reserved : 63;
     };
     #pragma pack(pop)
     static_assert(sizeof(PageHeader) == 8); //the minimum that the flash can write

     #pragma pack(push, 1)
     struct AuditHeader
     {
          //first 8 bytes
          bool is_unused : 1;
          bool is_valid : 1;
          uint32_t reserved : 30;
          uint16_t id;
          uint16_t size;

          //second 8 bytes
          uint16_t part_offset;
          uint16_t part_size;
          uint32_t part_reserved;

          static void print(const AuditHeader& header)
          {
               LIB_PRINTF("Audit header{ unused: %d, valid: %d, id: %u, size: %lu, part_offset: %lu, part_size: %lu}\n", header.is_unused, header.is_valid, header.id, header.size, header.part_offset, header.part_size);
          }

          static AuditHeader create(size_t data_size) noexcept
          {
               AuditHeader header;
               std::memset(&header, 1, sizeof(AuditHeader));

               header.is_unused = 0;
               header.is_valid = 1;
               header.id = 0;
               header.size = static_cast<uint16_t>(data_size);
               header.part_offset = 0;
               header.part_size = 0;

               return header;
          }

          static size_t num_parts(size_t data_size) noexcept
          {
               if (data_size <= MaxPartSize) return 1;
               return (data_size / MaxPartSize) + ((data_size % MaxPartSize) ? 1 : 0);
          }

          static size_t padding_size(size_t data_size) noexcept
          {
               return (8 - (data_size % 8)) % 8; //must be multiple of 8 bytes (the minimum that the flash can write)
          }

          static size_t total_size(const AuditHeader& header) noexcept
          {
               return (sizeof(AuditHeader) + header.part_size + padding_size(header.part_size));
          }

          static bool is_fragmented(const AuditHeader& header) noexcept
          {
               assert(header.size >= header.part_size);
               assert(header.part_size > 0);

               return (header.part_offset != 0) || (header.size != header.part_size);
          }
     };
     #pragma pack(pop)
     static_assert(sizeof(AuditHeader) == 16); //flash writes in multiples of 8 bytes

     struct AuditInfo
     {
          AuditHeader header;
          flash_page_t page_index;
          flash_address_t page_address;

          static void print(const AuditInfo& info)
          {
               LIB_PRINTF("Audit page{ index: %u, address: %lu} -> ", info.page_index, info.page_address);
               AuditHeader::print(info.header);
          }

          static flash_address_t data_address(const AuditInfo& info)
          {
               return static_cast<flash_address_t>(info.page_address + sizeof(AuditHeader));
          }
     };

     bool pages_iterate(const std::function<bool(const PageHeader& page_header, flash_page_t page_index)>& cb, bool ignore_unused = true)
     {
          PageHeader page_header;

          auto num_pages = UAIR_BSP_flash_audit_area_get_page_count();
          for (decltype(num_pages) i = 0; i < num_pages; i++)
          {
               if (UAIR_BSP_flash_audit_area_read(i * BSP_FLASH_PAGE_SIZE, reinterpret_cast<uint8_t*>(&page_header), sizeof(PageHeader)) != sizeof(PageHeader))
                    return false;

               if (ignore_unused && page_header.is_unused)
                    continue; //this page was never written to (it's empty)

               if (!cb(page_header, i)) break;
          }

          return true;
     }

     bool audits_iterate(flash_page_t page_index, const std::function<bool(const AuditInfo& audit_info)>& cb)
     {
          assert((page_index >= 0) && (page_index < UAIR_BSP_flash_audit_area_get_page_count()));

          auto page_address = (static_cast<flash_address_t>(page_index) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
          auto page_address_end = page_address + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

          while (page_address < page_address_end)
          {
               AuditHeader header;
               if (UAIR_BSP_flash_audit_area_read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(AuditHeader)) != sizeof(AuditHeader))
                    return false;

               if (header.is_unused)
                    return true; //this audit was never written: the audit before this one was the last of the page

               AuditInfo info;
               info.header = header;
               info.page_index = page_index;
               info.page_address = page_address;
               if (!cb(info)) return false;

               page_address += AuditHeader::total_size(header);
          }

          return true;
     }

     bool audits_iterate(const std::function<bool(const AuditInfo& audit_info)>& cb)
     {
          pages_iterate([&cb](const PageHeader&, flash_page_t page_index)
          {
               if (!audits_iterate(page_index, cb))
                    return false; //if audits stopped, we can stop

               return true; //continue to the next page
          });

          return true;
     }

     bool audit_copy(uair_io_context& ctx, flash_address_t source, flash_address_t dest, const AuditHeader& header)
     {
          //we can write the header already (optimization)
          assert(sizeof(AuditHeader) == 16);
          if (UAIR_BSP_flash_audit_area_write(dest, (uint64_t*)&header, 2) != 2)
          {
               ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
               return false;
          }

          auto remaining = AuditHeader::total_size(header) - sizeof(AuditHeader);
          if (remaining <= 0)
               return true;

          source += sizeof(AuditHeader);
          dest += sizeof(AuditHeader);
          assert((remaining % 8) == 0);

          uint64_t buffer[32];
          static_assert((sizeof(uint64_t) % 8) == 0);

          while(remaining > 0)
          {
               auto size = std::min(remaining, sizeof(uint64_t) * 32); //size is guaranteed to be multiple of 8 bytes (uint64_t)

               if (UAIR_BSP_flash_audit_area_read(source, reinterpret_cast<uint8_t*>(&buffer), size) != (int)size)
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_READ;
                    return false;
               }

               if (UAIR_BSP_flash_audit_area_write(dest, (uint64_t*)&buffer, size / 8) != (int)(size / 8))
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }

               remaining -= size;
               source += size;
               dest += size;
          }

          return true;
     }

     bool audit_write(uair_io_context& ctx, flash_address_t target_address, const AuditHeader& header, const void* data)
     {
          [[maybe_unused]] auto data_num_parts = AuditHeader::num_parts(header.size);
          assert(data_num_parts >= 1);

          if (data_num_parts == 1)
          {
               assert(header.part_offset == 0);
               assert(header.part_size == header.size);
          }
          else
          {
               assert(header.part_offset >= 0);
               assert(header.part_size < header.size);
          }     

          //write header
          {
               assert(sizeof(AuditHeader) / 8 == 2);
               if (UAIR_BSP_flash_audit_area_write(target_address, (uint64_t*)&header, 2) != 2)
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }

               target_address += sizeof(AuditHeader);
          }

          //write data

          //if we have data and the size is a multiple of 8 bytes (uint64_t)
          if (data && ((header.part_size % 8) == 0))
          {
               if (UAIR_BSP_flash_audit_area_write(target_address, (uint64_t*)data, header.part_size / 8) != (header.part_size / 8))
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }

               return true;
          }

          //either we don't have data or it needs to be padded

          uint64_t buffer[32];
          static_assert((sizeof(uint64_t) % 8) == 0);

          auto part_size = header.part_size;
          while (part_size > 0)
          {
               auto size = part_size;
               if (size > static_cast<uint16_t>(sizeof(uint64_t) * 32))
                    size = static_cast<uint16_t>(sizeof(uint64_t) * 32);

               if (!data)
                    std::memset(&buffer, 0, size);
               else
               {
                    std::memcpy(&buffer, data, size);
                    data = static_cast<const uint8_t*>(data) + size;
               }

               assert(size <= part_size);
               part_size -= size;

               if ((size % 8) == 0)
               {
                    if (UAIR_BSP_flash_audit_area_write(target_address, (uint64_t*)&buffer, size / 8) != (size / 8))
                    {
                         ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                         return false;
                    }

                    target_address += size;
                    continue;
               }

               //we need to add padding, which means that there can't be no more parts
               assert(part_size <= 0);

               auto extra_size = AuditHeader::padding_size(size);
               std::memset(reinterpret_cast<uint8_t*>(&buffer) + size, 1, extra_size);

               size += extra_size;
               assert((size % 8) == 0);
               assert(size <= sizeof(uint64_t) * 32);

               if (UAIR_BSP_flash_audit_area_write(target_address, (uint64_t*)&buffer, size / 8) != size / 8)
               {
                    ctx.error = UAIR_IO_CONTEXT_ERROR_WRITE;
                    return false;
               }

               target_address += size;
          }

          return true;
     }
}

int UAIR_io_audit_add(uair_io_context* ctx, const void* data, size_t size)
{
     if (!ctx) return 0;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     if (!data || (size <= 0) || (size >= std::numeric_limits<uint16_t>::max()))
     {
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_INVALID_DATA_SIZE);
          return 0;
     }

     auto data_num_parts = AuditHeader::num_parts(size);

     auto new_header = AuditHeader::create(size);
     new_header.part_size = new_header.size; //assume

     struct
     {
          uint16_t id_max_used = 0;
          uint16_t id_recicled = 0;

          size_t num_free_pages = 0;
          size_t invalidated_entries_size = 0;

          bool has_last_entry = false;
          size_t last_entry_size = 0;
          flash_page_t last_entry_page_index = 0;
          flash_address_t last_entry_page_address = 0;
     } page_info;

     //gather information

     pages_iterate([&page_info, &new_header, &data_num_parts](const PageHeader& page_header, flash_page_t page_index) mutable
     {
          if (page_header.is_unused)
          {
               page_info.num_free_pages++;
               return true; //next page
          }

          audits_iterate(page_index, [&page_info](const AuditInfo& audit_info)
          {
               if (!audit_info.header.is_valid)
               {
                    if (page_info.id_recicled <= 0)
                         page_info.id_recicled = audit_info.header.id;
                    page_info.invalidated_entries_size += AuditHeader::total_size(audit_info.header);
               }
               else
               {
                    page_info.id_max_used = (page_info.id_max_used <= 0) ? audit_info.header.id : std::max(audit_info.header.id, page_info.id_max_used);
               }

               page_info.has_last_entry = true;
               page_info.last_entry_size = AuditHeader::total_size(audit_info.header);
               page_info.last_entry_page_index = audit_info.page_index;
               page_info.last_entry_page_address = audit_info.page_address;

               return true;
          });

          //if we only need 1 part and if this page has room, we're done
          if ((data_num_parts == 1) && page_info.has_last_entry)
          {
               auto page_remaining_space = ((page_info.last_entry_page_index + 1) * BSP_FLASH_PAGE_SIZE) - (page_info.last_entry_page_address + page_info.last_entry_size);
               if (page_remaining_space >= AuditHeader::total_size(new_header))
                    return false; //found where we can write
          }

          //must move on to the next page
          page_info.has_last_entry = false;
          return true;

     }, false);

     //choose id
     if (page_info.id_recicled > 0)
          new_header.id = page_info.id_recicled;
     else if (page_info.id_max_used > 0)
          new_header.id = page_info.id_max_used + 1;
     else
          new_header.id = 1;

     //optimization: we only have 1 part to write and we can do it after the last audit
     if (page_info.has_last_entry)
     {
          assert(data_num_parts == 1);
          if (!audit_write(*ctx, page_info.last_entry_page_address + page_info.last_entry_size, new_header, data))
               return 0;

          return new_header.id;
     }

     //check if we have enough space
     {
          auto total_required_size = size + (sizeof(AuditHeader) + AuditHeader::padding_size(size)) * data_num_parts;

          auto total_available_size = (page_info.num_free_pages * (BSP_FLASH_PAGE_SIZE - sizeof(PageHeader))) + page_info.invalidated_entries_size;
          if (total_required_size >= total_available_size)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_NO_SPACE_AVAILABLE;
               return false;
          }

          auto total_free_size = (page_info.num_free_pages * (BSP_FLASH_PAGE_SIZE - sizeof(PageHeader)));
          if (total_required_size >= total_free_size)
          {
               //we can try after a flush
               ctx->flags = UAIR_IO_CONTEXT_FLAG_FLUSH;
               ctx->error = UAIR_IO_CONTEXT_ERROR_CTX_CHECK;
               return false;
          }
     }

     //reaching this point, we can write
     while (size > 0)
     {
          //the size of this part
          //NOTE! the actual written part can be less (e.g.: last audit at page_end)
          new_header.part_size = std::min(size, MaxPartSize);

          bool success{ false };
          pages_iterate([&ctx, &new_header, data, &success](const PageHeader& page_header, flash_page_t page_index) mutable
          {
               if (page_header.is_unused)
               {
                    //write a new page
                    PageHeader page_header;
                    page_header.is_unused = false;
                    page_header.reserved = 0x7FFFFFFFFFFFFFF;

                    if (UAIR_BSP_flash_audit_area_write(page_index * BSP_FLASH_PAGE_SIZE, (uint64_t*)&page_header, 1) != 1)
                    {
                         ctx->error = UAIR_IO_CONTEXT_ERROR_WRITE;
                         return false;
                    }

                    if (!audit_write(*ctx, page_index * BSP_FLASH_PAGE_SIZE + sizeof(PageHeader), new_header, data))
                         return false;

                    success = true;
                    return false;
               }

               //check if the page has room to write after the last audit
               {
                    AuditInfo last_audit;
                    last_audit.header.size = 0;
                    audits_iterate(page_index, [&last_audit](const AuditInfo& audit_info) mutable
                    {
                         last_audit = audit_info;
                         return true;
                    });

                    assert(last_audit.header.size > 0); //we have to have at least one audit

                    auto page_remaining_space = ((last_audit.page_index + 1) * BSP_FLASH_PAGE_SIZE) - (last_audit.page_address + AuditHeader::total_size(last_audit.header));
                    if (page_remaining_space >= AuditHeader::total_size(new_header))
                    {
                         //there's space for the requested amount
                         if (!audit_write(*ctx, last_audit.page_address + AuditHeader::total_size(last_audit.header), new_header, data))
                              return false;

                         success = true;
                         return false;
                    }
                    else if (page_remaining_space > (sizeof(AuditHeader) + 16)) //if we have a bit more free space enough for {header + possible padding}, use it
                    {
                         new_header.part_size = page_remaining_space - sizeof(AuditHeader) - 8;
                         assert(new_header.part_size <= MaxPartSize);
                         assert(page_remaining_space >= AuditHeader::total_size(new_header));

                         if (!audit_write(*ctx, last_audit.page_address + AuditHeader::total_size(last_audit.header), new_header, data))
                              return false;

                         success = true;
                         return false;
                    }
               }

               return true; //continue to the next page

          }, false);

          if (ctx->error != UAIR_IO_CONTEXT_ERROR_NONE)
               return 0;

          if (!success)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_NO_SPACE_AVAILABLE;
               return 0;
          }

          //the part was written, move on to the next
          assert(new_header.part_size <= size);

          data = static_cast<const uint8_t*>(data) + new_header.part_size;
          size -= new_header.part_size;
          new_header.part_offset += new_header.part_size;
     }

     //all went well
     return new_header.id;
}

size_t UAIR_io_audit_retrieve(uair_io_context* ctx, int id, void* data)
{
     if (!ctx) return 0;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     if (id <= 0)
     {
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_INVALID_ID);
          return 0;
     }

     //we just want to read the size of the data
     if (!data)
     {
          bool found{ false };
          size_t data_size{ 0 };
          audits_iterate([&found, &data_size, &id](const AuditInfo& audit_info)
          {
               auto& header = audit_info.header;

               if (!header.is_valid) return true;
               if (header.id != id) return true;

               found = true;
               data_size = header.size;

               return false; //stop iteration
          });

          if (!found)
          {
               ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_UNKNOWN_ID);
               return 0;
          }

          return data_size;
     }

     //we want to read everything

     struct
     {
          int id;
          void* dest;
          bool found;
          size_t size_total;
          size_t size_written;
     } search_data;

     search_data.id = id;
     search_data.dest = data;
     search_data.found = false;
     search_data.size_total = 0;
     search_data.size_written = 0;

     audits_iterate([&ctx, &search_data](const AuditInfo& audit_info)
     {
          auto& header = audit_info.header;

          if (!header.is_valid) return true;
          if (header.id != search_data.id) return true;

          if (!search_data.found)
          {
               search_data.found = true;
               search_data.size_total = header.size;
          }
          
          assert(search_data.size_total == header.size);

          if (UAIR_BSP_flash_audit_area_read(AuditInfo::data_address(audit_info), reinterpret_cast<uint8_t*>(search_data.dest) + header.part_offset, header.part_size) != (int)header.part_size)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_READ;
               return false;
          }

          search_data.size_written += header.part_size;

          //optimization: if not fragmented, we already read everything
          if (!AuditHeader::is_fragmented(header))
               return false;

          //optimization: we already read everything (all the parts)
          if (search_data.size_written == header.size)
               return false;

          return true; //read all parts
     });

     if (ctx->error != UAIR_IO_CONTEXT_ERROR_NONE)
          return 0; 
     
     if (!search_data.found)
     {
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_UNKNOWN_ID);
          return 0;
     }

     if (search_data.size_total != search_data.size_written)
     {
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_DATA_CORRUPTION);
          return 0;
     }

     //all was well
     assert(search_data.size_written == search_data.size_total);
     return search_data.size_written;
}

void UAIR_io_audit_dispose(uair_io_context* ctx, int id)
{
     if (!ctx) return;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     audits_iterate([&ctx, id](const AuditInfo& audit_info)
     {
          auto& header = audit_info.header;

          if (!header.is_valid) return true;
          if (header.id != id) return true;

          //NOTE: the "is_valid" is on the first 8 bytes of the header, so we only need to write that

          auto new_header = header;
          new_header.is_valid = false;
          if (UAIR_BSP_flash_audit_area_write(audit_info.page_address, (uint64_t*)&new_header, 1) != 1)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_WRITE;
               return false;
          }

          //optimization: if not fragmented, no need to search for more parts
          if (!AuditHeader::is_fragmented(header))
               return false;

          return true; //have to dispose of all the parts
     });
}

int UAIR_io_audit_iter_begin(uair_io_context* ctx)
{
     if (!ctx) return 0;

     int first_part_id = 0;
     audits_iterate([&first_part_id](const AuditInfo& audit_info)
     {
          if (!audit_info.header.is_valid) return true;
          if (audit_info.header.part_offset != 0) return true; //only the first parts

          first_part_id = audit_info.header.id;

          return false; //stop iteration
     });

     return first_part_id;
}

int UAIR_io_audit_iter_next(uair_io_context* ctx, int previous_id)
{
     if (!ctx) return 0;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     if (previous_id <= 0)
     {
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_INVALID_ID);
          return 0;
     }

     int next_part_id = 0;
     audits_iterate([&previous_id, &next_part_id](const AuditInfo& audit_info)
     {
          if (!audit_info.header.is_valid) return true;
          if (audit_info.header.part_offset != 0) return true; //only the first parts

          //still searching for the previous id
          if (previous_id > 0)
          {
               if (audit_info.header.id != previous_id)
                    return true; //continue until we find the previous id

               //found it
               previous_id = 0;
               return true; //continue the search for the next one
          }

          //reaching this point, we have a part and already found the previous               
          next_part_id = audit_info.header.id;
          return false; //stop iteration
     });

     //if we didn't find the previous id, then the data was changed
     if (previous_id > 0)
     {
          ctx->error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_UNKNOWN_ID);
          return -1;
     }

     return next_part_id;
}

void UAIR_io_audit_flush(uair_io_context* ctx)
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

     auto sucess = pages_iterate([&stats](const PageHeader& page_header, flash_page_t page_index) mutable
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
               AuditHeader header;
               if (UAIR_BSP_flash_audit_area_read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(AuditHeader)) != sizeof(AuditHeader))
                    return false;

               if (header.is_unused)
                    break; //no more entries written

               auto audit_size = AuditHeader::total_size(header);

               if (!header.is_valid)
                    size_deleted += audit_size;
               else
                    size_used += audit_size;

               page_address += audit_size;
          }

          //pick the best one to clean (with the most delete entries)
          if ((size_deleted > 0) && (size_deleted > stats.page_to_clean_delete_size))
          {
               stats.page_to_clean = page_index;
               stats.page_to_clean_used_size = size_used;
               stats.page_to_clean_delete_size = size_deleted;
          }

          //pick the best de-fragmented one (not a single audit deleted and with the least used space)
          if ((size_deleted <= 0) && (size_used < stats.page_defrag_used_size))
          {
               stats.page_defrag = page_index;
               stats.page_defrag_used_size = size_used;
          }

          return true;

     }, false);

     if (!sucess)
     {
          ctx->error = UAIR_IO_CONTEXT_ERROR_INTERNAL;
          return;
     }

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

               AuditHeader header;
               if (UAIR_BSP_flash_audit_area_read(src_begin, reinterpret_cast<uint8_t*>(&header), sizeof(AuditHeader)) != sizeof(AuditHeader))
                    return;

               if (header.is_unused)
                    break; //no more entries to read

               if (header.is_valid)
               {
                    //copy audit
                    if (!audit_copy(*ctx, src_begin, dst_begin, header))
                         return; //something went wrong

                    dst_begin += AuditHeader::total_size(header);
               }

               src_begin += AuditHeader::total_size(header);
          }

          //finally erase the page we read from

          if (UAIR_BSP_flash_audit_area_erase_page(stats.page_to_clean) != BSP_ERROR_NONE)
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

          //write the page header (if we are going to need it)
          if (stats.page_to_clean_used_size > 0)
          {
               PageHeader page_header;
               page_header.is_unused = false;
               page_header.reserved = 0x7FFFFFFFFFFFFFF;

               if (UAIR_BSP_flash_audit_area_write(dst_begin, (uint64_t*)&page_header, 1) != 1)
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

               AuditHeader header;
               if (UAIR_BSP_flash_audit_area_read(src_begin, reinterpret_cast<uint8_t*>(&header), sizeof(AuditHeader)) != sizeof(AuditHeader))
               {
                    ctx->error = UAIR_IO_CONTEXT_ERROR_READ;
                    return;
               }

               if (header.is_unused)
                    break; //no more entries to read

               if (header.is_valid)
               {
                    //to make sure the page header was written
                    assert(dst_begin > (static_cast<flash_address_t>(stats.page_free) * BSP_FLASH_PAGE_SIZE));
                    assert(stats.page_to_clean_used_size > 0);

                    //copy audit
                    if (!audit_copy(*ctx, src_begin, dst_begin, header))
                         return; //something went wrong

                    dst_begin += AuditHeader::total_size(header);
               }

               src_begin += AuditHeader::total_size(header);
          }

          //finally erase the page we read from

          if (UAIR_BSP_flash_audit_area_erase_page(stats.page_to_clean) != BSP_ERROR_NONE)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_INTERNAL;
               return;
          }
     }

     //all done... to make sure everything is optimal, just try again
     //it should detect there's nothing to clean, and simply leave
     UAIR_io_audit_flush(ctx);
}