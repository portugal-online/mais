#ifndef UAIR_CONFIG_API_H__
#define UAIR_CONFIG_API_H__

#include "UAIR_io_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UAIR_CONFIG_ID_NONE = 0,
    
} uair_config_id;

typedef struct {
   uair_config_id id;
   uint8_t value;
} uair_config_pair_uint8;

int8_t uair_config_cache_size(int8_t new_size);

int uair_config_write_uint8(uair_config_id id, uint8_t value);
int uair_config_read_uint8(uair_config_id id, uint8_t *value);

const uair_config_pair_uint8* config_defaults_uint8(int* size);

int uair_config_write_uint8s(const uair_config_pair_uint8* pairs, int size);
int uair_config_read_uint8s(uair_config_pair_uint8* pairs, int size);

#ifdef __cplusplus
}
#endif

#endif
