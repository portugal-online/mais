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
    /* Error: audit data has become corrupt */
    UAIR_IO_AUDIT_ERROR_DATA_CORRUPTION = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 2,
    /* Error: data isn't valid */
    UAIR_IO_AUDIT_ERROR_INVALID_DATA_SIZE = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 3,
} uair_io_context_audit_errors;


/**
 * Adds a new audit entry.
 * 
 * If any error occurs (the method returns <= 0), the reason is stored in the IO context (\p ctx).
 * 
 * @param ctx the IO context
 * @param data data associated with the audit (can be NULL, in which case the data is filled with 0s)
 * @param data_size size of the data associated with the audit
 * @return the id of the new audit entry (<= 0 indicates an error)
 */
int UAIR_io_audit_add(uair_io_context* ctx, const void* data, size_t data_size);

/**
 * Retrieves the data associated with an audit.
 * 
 * If any error occurs (the method returns <= 0), the reason is stored in the IO context (\p ctx).
 * 
 * @param ctx the IO context
 * @param id the id of the target audit
 * @param data where to write the audit data (can be null)
 * @return the size of the audit data or 0 in case of an error
 */
size_t UAIR_io_audit_retrieve(uair_io_context* ctx, int id, void* data);

/**
 * Removes / deletes a specific audit entry.
 * 
 * @param ctx the IO context
 * @param id the id of the target audit to remove
 */
void UAIR_io_audit_dispose(uair_io_context* ctx, int id);

/**
 * Begins a new iterations over all the audit entries available.
 *  
 * @param ctx the IO context
 * @return the id of an audit or <= 0 in case of an error
 */
int UAIR_io_audit_iter_begin(uair_io_context* ctx);
/**
 * Moves to the next audit entry of an interaction.
 *  
 * @param ctx the IO context
 * @param previous_id the id of the previous audit (returned by either \ref UAIR_io_audit_iter_begin(uair_io_context*) or \ref UAIR_io_audit_iter_next(uair_io_context*))
 * @return the id of an audit or <= 0 in case of an error or if the iteration stopped
 */
int UAIR_io_audit_iter_next(uair_io_context* ctx, int previous_id);

/**
 * Flushes the audit data.
 *
 * This can be called at any time, but for optimal results, if should only be
 * used when the UAIR_IO_CONTEXT_FLAG_FLUSH flag is set in "uair_io_context".
 * 
 * @param ctx the IO context
 */
void UAIR_io_audit_flush(uair_io_context* ctx);

#ifdef __cplusplus
}
#endif

#endif
