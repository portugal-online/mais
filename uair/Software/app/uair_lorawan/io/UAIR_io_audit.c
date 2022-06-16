#include "UAIR_io_audit.h"

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
               assert(header.offset < header.part_size);
               assert(header.part_size > 0);

               return (part_offset != 0) || (header.size != header.part_size);
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
               AuditHeader::print(info.header);
               LIB_PRINTF("Audit page{ index: %u, address: %lu}\n", info.page_index, info.page_address);
          }

          static flash_address_t data_address(const AuditInfo& info)
          {
               return static_cast<flash_address_t>(page_address + sizeof(AuditHeader));
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

     bool audits_iterate(flash_page_t target_page_index, const std::function<bool(const AuditInfo& audit_info)>& cb)
     {
          assert((target_page_index >= 0) && (target_page_index < UAIR_BSP_flash_config_area_get_page_count()));

          auto page_address = (static_cast<flash_address_t>(target_page_index) * BSP_FLASH_PAGE_SIZE) + sizeof(PageHeader);
          auto page_address_end = page_address + BSP_FLASH_PAGE_SIZE - sizeof(PageHeader);

          while (page_address < page_address_end)
          {
               AuditHeader header;
               if (UAIR_BSP_flash_config_area_read(page_address, reinterpret_cast<uint8_t*>(&header), sizeof(AuditHeader)) != sizeof(AuditHeader))
                    return false;

               if (header.is_unused)
                    return true; //this audit was never written: the audit before this one was the last of the page

               AuditInfo info;
               info.header = header;
               info.page_index = target_page_index;
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
                    return false; //something went wrong

               return true; //continue to the next page
          });

          return true;
     }
}

int UAIR_io_audit_add(uair_io_context* ctx, const void* data, size_t size)
{
     if (!ctx) return 0;
     return 0;
}

size_t UAIR_io_audit_retrieve(uair_io_context* ctx, int id, void* data)
{
     if (!ctx) return;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     if (id <= 0)
     {
          ctx->error = UAIR_IO_AUDIT_ERROR_INVALID_ID;
          return 0;
     }

     //we just want to read the size of the data
     if (!data)
     {
          bool found{ false };
          size_t data_size{ 0 };
          audits_iterate([&found, &data_size](const AuditInfo& audit_info)
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
               ctx.error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_UNKNOWN_ID);
               return 0;
          }

          return data_size;
     }

     //we want to read everything

     struct
     {
          void* dest;
          bool found;
          size_t size_total;
          size_t size_written;
     } search_data;

     search_data.dest = data;
     search_data.found = false;
     search_data.size_total = 0;
     search_data.size_written = 0;

     audits_iterate([&search_data](const AuditInfo& audit_info)
     {
          auto& header = audit_info.header;

          if (!header.is_valid) return true;
          if (header.id != id) return true;

          if (!search_data.found)
          {
               search_data.found = true;
               search_data.size_total = header.size;
          }
          
          assert(search_data.size_total = header.size);

          if (UAIR_BSP_flash_config_area_read(AuditInfo::data_address(audit_info), reinterpret_cast<uint8_t*>(search_data.dest) + header.part_offset, header.part_size) != sizeof(header.part_size))
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_READ;
               return false;
          }

          search_data.size_written += header.part_size;

          //optimization: is not fragmented, we already read everything
          if (!AuditInfo::is_fragmented(header))
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
          ctx->error = UAIR_IO_AUDIT_ERROR_UNKNOWN_ID;
          return 0;
     }

     if (!search_data.size_total != search_data.size_written)
     {
          ctx->error = UAIR_IO_AUDIT_ERROR_DATA_CORRUPTION;
          return 0;
     }

     //all was well
     return search_data.size_written;
}

void UAIR_io_audit_dispose(uair_io_context* ctx, int id)
{
     if (!ctx) return;
     ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
     ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;

     audits_iterate([id](const AuditInfo& audit_info)
     {
          auto& header = audit_info.header;

          if (!header.is_valid) return true;
          if (header.id != id) return true;

          //NOTE: the "is_valid" is on the first 8 bytes of the header, so we only need to write that
          static_assert(offsetof(AuditHeader, is_valid) < 8);

          auto new_header = header;
          new_header.is_valid = false;
          if (UAIR_BSP_flash_config_area_write(audit_info.page_address, (uint64_t*)&new_header, 1) != 1)
          {
               ctx->error = UAIR_IO_CONTEXT_ERROR_WRITE;
               return false;
          }

          //optimization: is not fragmented, no need to search for more parts
          if (!AuditInfo::is_fragmented(header))
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
          ctx->error = UAIR_IO_AUDIT_ERROR_INVALID_ID;
          return 0;
     }

     int next_part_id = 0;
     audits_iterate([&previous_id](const AuditInfo& audit_info)
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
          ctx.error = static_cast<uair_io_context_errors>(UAIR_IO_AUDIT_ERROR_UNKNOWN_ID);
          return -1;
     }

     return next_part_id;
}

void UAIR_io_audit_flush(uair_io_context* ctx)
{
     if (!ctx) return;
}