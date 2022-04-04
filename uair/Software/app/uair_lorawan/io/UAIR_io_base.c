#include "UAIR_io_base.h"

void UAIR_io_init_ctx(uair_io_context* ctx)
{
    if (!ctx) return;
    
    ctx->flags = UAIR_IO_CONTEXT_FLAG_NONE;
    ctx->error = UAIR_IO_CONTEXT_ERROR_NONE;
}
