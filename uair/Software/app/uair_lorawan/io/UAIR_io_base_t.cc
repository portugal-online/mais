#include "UAIR_io_base.h"

#include <catch2/catch.hpp>

TEST_CASE( "UAIR IO base", "init" )
{    
    UAIR_io_init_ctx(nullptr);

    uair_io_context ctx;
    UAIR_io_init_ctx(&ctx);

    CHECK(ctx.flags == UAIR_IO_CONTEXT_FLAG_NONE);
    CHECK(ctx.error == UAIR_IO_CONTEXT_ERROR_NONE);
}
