#ifndef UAIR_CONFIG_API_H__
#define UAIR_CONFIG_API_H__

#include "UAIR_io_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The different config ids / keys that can be used in the API.
 * 
 * For each item, a corresponding entry in "uair_io_context_keys" should exist.
 */
typedef enum {
    UAIR_CONFIG_ID_TX_POLICY = UAIR_IO_CONTEXT_KEY_CONFIG_TX_POLICY,
    UAIR_CONFIG_ID_FAIR_RATIO = UAIR_IO_CONTEXT_KEY_CONFIG_FAIR_RATIO
} uair_config_id;


/**
 * A pair consisting of a config id and a unsigned int8 value.
 */ 
typedef struct {
   uair_config_id id;
   uint8_t value;
} uair_config_pair_uint8;

/**
 * Sets and returns the cache size.
 * 
 * If \p new_size is negative, then the size isn't changed and it simply returns
 * the current size. If 0, then it disables the cache. Otherwise, it
 * updates the cache size (a positive \p new_size, even if equal to the current
 * value, always resets the cache).
 * 
 * @param new_size the new size the cache should take.
 * @return the actual value of the cache
 */ 
int8_t uair_config_cache_size(int8_t new_size);

/**
 * Sets or updates the value of a config of type unsigned int8.
 * 
 * @param id the target config id
 * @param value the new value of the config
 * @return 0 if successful, otherwise the action failed (the value
 * matches with the values defined in "uair_io_context_errors")
 */
int uair_config_write_uint8(uair_config_id id, uint8_t value);
/**
 * Retrieves the value of a config of type unsigned int8.
 * 
 * The param \p value can be NULL, in which case the method simply checks
 * if the key is present and if it's of the correct type.
 * 
 * @param id the target config id
 * @param value where to store the value of the config
 * @return 0 if successful, otherwise the action failed (the value
 * matches with the values defined in "uair_io_context_errors")
 */
int uair_config_read_uint8(uair_config_id id, uint8_t *value);

/**
 * Returns the defaults values for certain config ids (not necessarily
 * all of them).
 * 
 * @param id if not NULL, stores the number of pairs returned
 * @return A list of pairs of config id / values
 */
const uair_config_pair_uint8* config_defaults_uint8(int* size);

/**
 * Sets or updates the value of multiple configs of type unsigned int8.
 * 
 * @param pairs the pairs of config id / value to write
 * @param size the number of pairs in \p pairs
 * @return 0 if successful, otherwise the action failed (the value
 * matches with the values defined in "uair_io_context_errors")
 */
int uair_config_write_uint8s(const uair_config_pair_uint8* pairs, int size);
/**
 * Retrieves the values of multiple configs of type unsigned int8.
 * 
 * The value for each entry is written into the pair. Any value stored is ignored
 * and overwritten.
 * 
 * @param pairs the pairs of config id to read
 * @param size the number of pairs in \p pairs
 * @return 0 if successful, otherwise the action failed (the value
 * matches with the values defined in "uair_io_context_errors")
 */
int uair_config_read_uint8s(uair_config_pair_uint8* pairs, int size);

#ifdef __cplusplus
}
#endif

#endif
