#ifndef PAYLOAD_H__
#define PAYLOAD_H__

struct generic_payload
{
    uint8_t rsvd:6;
    uint8_t payload_type:2;
};

struct payload_type0
{
    uint8_t max_oaq_msb:1;
    uint8_t epa_oaq_msb:1;
    uint8_t health_int_temp_hum:1;
    uint8_t health_ext_temp_hum:1;
    uint8_t health_microphone:1;
    uint8_t health_oaq:1;
    uint8_t payload_type:2;
    uint8_t avg_ext_temp;
    uint8_t max_sound_level_msb:1;
    uint8_t avg_ext_hum:7;

    uint8_t epa_oaq_lsb;
    uint8_t max_oaq_lsb;
    uint8_t avg_sound_level_lsb:4;
    uint8_t max_sound_level_lsb:4;
    uint8_t max_int_temp;
    uint8_t avg_sound_level_msb:1;
    uint8_t max_int_hum:7;
} __attribute__((packed));

struct payload_type1
{
    struct payload_type0 p0;
    uint8_t battery_level:7;
    uint8_t system_restart:1;
    uint8_t errors_logged;
} __attribute__((packed));

#endif
