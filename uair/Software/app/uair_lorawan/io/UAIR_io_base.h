#ifndef UAIR_IO_BASE_H__
#define UAIR_IO_BASE_H__

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UAIR io context flags */
typedef enum {
    /* No action is required */
    UAIR_IO_CONTEXT_FLAG_NONE = 0,
    /* Flush and try again */
    UAIR_IO_CONTEXT_FLAG_FLUSH = (1 << 0)
} uair_io_context_flags;

/* UAIR io context errors */
typedef enum {
    /* No error present (last action was successfull) */
    UAIR_IO_CONTEXT_ERROR_NONE = 0,
    /* Error: no action was performed because context was in an invalid state */
    UAIR_IO_CONTEXT_ERROR_CTX_INVALID = 1,
    /* Error: action was canceled but can be retried (check flags on how to proceed) */
    UAIR_IO_CONTEXT_ERROR_CTX_CHECK = 2,
    /* Error: the flush command couldn't run because there's no free page to use as buffer */
    UAIR_IO_CONTEXT_ERROR_CTX_FLUSH_NO_FREE_PAGE = 3,

    UAIR_IO_CONTEXT_ERROR_EXT_BASE = 10
} uair_io_context_errors;

/**
 * Stores the current state of IO operations.
 *
 * Call UAIR_io_init_ctx() before first use.
 * Do *not* change this values directly, they're just for reading.
 * 
 */
typedef struct {
    uair_io_context_flags flags;
    uair_io_context_errors error;
} uair_io_context;

/**
 * Initializes an io_context struct.
 * 
 * All existing data is ignored and removed.
 */
void UAIR_io_init_ctx(uair_io_context* ctx);

#ifdef __cplusplus
}
#endif

#endif
