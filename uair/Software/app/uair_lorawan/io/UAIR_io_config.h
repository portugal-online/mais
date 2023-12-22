#ifndef UAIR_IO_CONFIG_H__
#define UAIR_IO_CONFIG_H__

#include "UAIR_io_base.h"

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** UAIR io context config extended errors */
typedef enum {
    /** Error: the key doesn't exist */
    UAIR_IO_CONFIG_ERROR_INVALID_KEY = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 0,
    /** Error: the key (already) exists but the type doesn't match */
    UAIR_IO_CONFIG_ERROR_KEY_TYPE_MISMATCH = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 1,
    /** Error: the key exists and the type is valid but an error occurred trying to read its data */
    UAIR_IO_CONFIG_ERROR_DATA_ERROR = UAIR_IO_CONTEXT_ERROR_EXT_BASE + 2,
} uair_io_context_config_errors;

/** UAIR io config supported key types */
typedef enum {
    /** Key doesn't exist */
    UAIR_IO_CONFIG_KEY_TYPE_NOT_AVAILABLE = 0,
    /** Key exists but has an unknown type */
    UAIR_IO_CONFIG_KEY_TYPE_UNKNOWN = 1,
    /** Key exists and has type uint8_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT8 = 10,
    /** Key exists and has type uint16_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT16 = 11,
    /** Key exists and has type uint32_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT32 = 12,
    /** Key exists and has type uint64_t */
    UAIR_IO_CONFIG_KEY_TYPE_UINT64 = 13,
    /** Key exists and has type blob (generic buffer) */
    UAIR_IO_CONFIG_KEY_TYPE_BLOB = 14
} uair_io_config_key_type;

/** UAIR io config key identifiers */
typedef enum {
    UAIR_IO_CONTEXT_KEY_CONFIG_TX_POLICY = 1,
    UAIR_IO_CONTEXT_KEY_CONFIG_FAIR_RATIO = 2
    // ...
} uair_io_context_keys;

/**
 * Checks if a key exists and/or returns its type.
 * 
 * If the key doesn't exist, no error is set in the context (it returns IO_CONFIG_KEY_TYPE_NOT_AVAILABLE).
 * 
 * @return the type of key
 */
uair_io_config_key_type UAIR_io_config_check_key(uair_io_context* ctx, uair_io_context_keys key);

/**
 * Returns some statistics about the state of the config dictionary.
 * 
 * @param ctx the IO context
 * @param num_keys the total number of keys in the config
 * @param used_space the amount of space use by the config
 * @param free_space the amount of free space available for the config
 * @param recyclable_space the amount of space that can be recycled / reclaimed
 */
void UAIR_io_config_stats(uair_io_context* ctx, size_t* num_keys, size_t* used_space, size_t* free_space, size_t* recyclable_space);

/**
 * Reads the value of a key of type uint8.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out where to store the value of the key (can be NULL, in which case, it only
 * checks if the key exists and is of the correct type)
 */
void UAIR_io_config_read_uint8(uair_io_context* ctx, uair_io_context_keys key, uint8_t* out);
/**
 * Reads the value of a key of type uint16.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out where to store the value of the key (can be NULL, in which case, it only
 * checks if the key exists and is of the correct type)
 */
void UAIR_io_config_read_uint16(uair_io_context* ctx, uair_io_context_keys key, uint16_t* out);
/**
 * Reads the value of a key of type uint32.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out where to store the value of the key (can be NULL, in which case, it only
 * checks if the key exists and is of the correct type)
 */
void UAIR_io_config_read_uint32(uair_io_context* ctx, uair_io_context_keys key, uint32_t* out);
/**
 * Reads the value of a key of type uint64.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out where to store the value of the key (can be NULL, in which case, it only
 * checks if the key exists and is of the correct type)
 */
void UAIR_io_config_read_uint64(uair_io_context* ctx, uair_io_context_keys key, uint64_t* out);
/**
 * Reads the bytes of a key of type blob (buffers).
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out where to store the value of the key
 * @param out_max_size the max amount of bytes that can be written to into \p out
 * @return the actual size written into \p out
 */
size_t UAIR_io_config_read_blob(uair_io_context* ctx, uair_io_context_keys key, void* out, size_t out_max_size);
/**
 * Reads the number of bytes of a key of type blob (buffers).
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @return the size of the stored buffer
 */
size_t UAIR_io_config_read_blob_size(uair_io_context* ctx, uair_io_context_keys key);

/**
 * Writes or updates the value of a key of type uint8.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out the new value of the key
 */
void UAIR_io_config_write_uint8(uair_io_context* ctx, uair_io_context_keys key, const uint8_t in);
/**
 * Writes or updates the value of a key of type uint16.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out the new value of the key
 */
void UAIR_io_config_write_uint16(uair_io_context* ctx, uair_io_context_keys key, const uint16_t in);
/**
 * Writes or updates the value of a key of type uint32.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out the new value of the key
 */
void UAIR_io_config_write_uint32(uair_io_context* ctx, uair_io_context_keys key, const uint32_t in);
/**
 * Writes or updates the value of a key of type uint64.
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param out the new value of the key
 */
void UAIR_io_config_write_uint64(uair_io_context* ctx, uair_io_context_keys key, const uint64_t in);
/**
 * Writes or updates the value of a key of type blob (buffer).
 * 
 * Any error is returned in the IO contex (\p ctx).
 * 
 * @param ctx the IO context
 * @param key the config key to read
 * @param in the bytes to write to the key
 * @param in_size the number of bytes in \p in
 */
void UAIR_io_config_write_blob(uair_io_context* ctx, uair_io_context_keys key, const void* in, size_t in_size);

/**
 * Removes a key from the config.
 * 
 * A non-existing key is considered a no-op.
 * 
 * @param ctx the IO context
 * @param key the config key to remove
 */
void UAIR_io_config_remove(uair_io_context* ctx, uair_io_context_keys key);

/**
 * Flushes the config data.
 *
 * This can be called at any time, but for optimal results, if should only be
 * used when the UAIR_IO_CONTEXT_FLAG_FLUSH flag is set in "uair_io_context".
 * 
 * @param ctx the IO context
 */
void UAIR_io_config_flush(uair_io_context* ctx);

#ifdef __cplusplus
}
#endif

#endif
