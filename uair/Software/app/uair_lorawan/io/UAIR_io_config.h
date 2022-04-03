#ifndef UAIR_IO_CONFIG_H__
#define UAIR_IO_CONFIG_H__

#include "UAIR_io_base.h"

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UAIR io context config extended errors */
typedef enum {
    /* Error: the key doesn't exist */
    UAIR_IO_CONFIG_ERROR_INVALID_KEY = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 0,
    /* Error: the key (already) exists but the type doesn't match */
    UAIR_IO_CONFIG_ERROR_KEY_TYPE_MISMATCH = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 1,
    /* Error: the key exists and the type is valid but an error occurred trying to read its data */
    UAIR_IO_CONFIG_ERROR_DATA_ERROR = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 2,
} uair_io_context_config_errors;

/* UAIR io config supported key types */
typedef enum {
    /* Key doesn't exist */
    UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE = 0,
    /* Key exists but has an unknown type */
    UAIR_IO_CONFIG_KEY_TYPE_UNKNOWN = 1,
    /* Key exists and has type uint8_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT8 = 10,
    /* Key exists and has type uint16_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT16 = 11,
    /* Key exists and has type uint32_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT32 = 12,
    /* Key exists and has type uint64_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT64 = 13,
    /* Key exists and has type blob (generic buffer) */
    UAIR_IO_CONFIG_KEY_TYPE_BLOB = 14
} uair_io_config_key_type;

/* UAIR io config key identifiers */
typedef enum {
    UAIR_IO_CONTEXT_KEY_CONFIG_TX_POLICY = 1,
    UAIR_IO_CONTEXT_KEY_CONFIG_FAIR_RATIO = 2
    // ...
} uair_io_context_keys;

/**
 * Checks if a key exists and/or returns its type.
 * 
 * If the key doesn't exist, no error is set in the context (it returns IO_CONFIG_KEY_TYPE_NOT_AVAILABLE)
 * @return the type of key
 */
uair_io_config_key_type UAIR_io_config_check_key(uair_io_context* ctx, uair_io_context_keys key);

/**
 * Returns some statistics about the state of the config dictionary
 */
void UAIR_io_config_stats(uair_io_context* ctx, size_t* num_keys, size_t* used_space, size_t* free_space, size_t* recyclable_space);

/**
 * Reads a key of type uint8_t into out.
 */
void UAIR_io_config_read_uint8(uair_io_context* ctx, uair_io_context_keys key, uint8_t* out);
/**
 * Reads a key of type uint16_t into out.
 */
void UAIR_io_config_read_uint16(uair_io_context* ctx, uair_io_context_keys key, uint16_t* out);
/**
 * Reads a key of type uint32_t into out.
 */
void UAIR_io_config_read_uint32(uair_io_context* ctx, uair_io_context_keys key, uint32_t* out);
/**
 * Reads a key of type uint64_t into out.
 */
void UAIR_io_config_read_uint64(uair_io_context* ctx, uair_io_context_keys key, uint64_t* out);
/**
 * Reads a key of type blob into out, reading at most out_max_size bytes.
 * 
 * @return the actual number of bytes read (e.g.: if the stored blob size is less than out_max_size)
 */
size_t UAIR_io_config_read_blob(uair_io_context* ctx, uair_io_context_keys key, void* out, size_t out_max_size);
/**
 * Reads the stored size of a key of type blob.
 * 
 * @return the size of the stored blob key (returns 0 if the key doesn't exist)
 */
size_t UAIR_io_config_read_blob_size(uair_io_context* ctx, uair_io_context_keys key);

/**
 * Writes (or updates if it exists) a key of type uint8_t.
 */
void UAIR_io_config_write_uint8(uair_io_context* ctx, uair_io_context_keys key, const uint8_t in);
/**
 * Writes (or updates if it exists) a key of type uint16_t.
 */
void UAIR_io_config_write_uint16(uair_io_context* ctx, uair_io_context_keys key, const uint16_t in);
/**
 * Writes (or updates if it exists) a key of type uint32_t.
 */
void UAIR_io_config_write_uint32(uair_io_context* ctx, uair_io_context_keys key, const uint32_t in);
/**
 * Writes (or updates if it exists) a key of type uint64_t.
 */
void UAIR_io_config_write_uint64(uair_io_context* ctx, uair_io_context_keys key, const uint64_t in);
/**
 * Writes (or updates if it exists) a key of type blob.
 * 
 * If in is NULL or in_size is 0, the buffer is cleared.
 */
void UAIR_io_config_write_blob(uair_io_context* ctx, uair_io_context_keys key, const void* in, size_t in_size);

/**
 * Flushes the config data.
 *
 * This should be used when the UAIR_IO_CONTEXT_FLAG_FLUSH is set.
 */
void UAIR_io_config_flush(uair_io_context* ctx);

#ifdef __cplusplus
}
#endif

#endif
