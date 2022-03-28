#ifndef UAIR_IO_AUDIT_H__
#define UAIR_IO_AUDIT_H__

#include "UAIR_io_base.h"

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UAIR io context audit extended errors */
typedef enum {
    /* Error: the ID provided is invalid */
    UAIR_IO_AUDIT_ERROR_INVALID_ID = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 0,
    /* Error: there's no audit data associated with the ID */
    UAIR_IO_AUDIT_ERROR_UNKNOWN_ID = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 1,
} uair_io_context_audit_errors;


int UAIR_io_audit_add(uair_io_context* ctx, const void* data, int size);

int UAIR_io_audit_retrieve(uair_io_context* ctx, int id, void* data);

void UAIR_io_audit_dispose(uair_io_context* ctx, int id);

int UAIR_io_audit_iter_begin(uair_io_context* ctx);
int UAIR_io_audit_iter_next(uair_io_context* ctx, int previous_id);

#ifdef __cplusplus
}
#endif

#endif
