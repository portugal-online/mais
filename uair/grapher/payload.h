#ifndef PAYLOAD_H__
#define PAYLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

struct generic_payload
{
    uint8_t rsvd:6;
    uint8_t payload_type:2;
};


struct payload_type0
{
    uint8_t epa_oaq_msb:1;  // Order is wrong between the two OAQ. Fix controller!
    uint8_t max_oaq_msb:1;
    uint8_t health_oaq:1;
    uint8_t health_microphone:1;
    uint8_t health_ext_temp_hum:1;
    uint8_t health_int_temp_hum:1;
    uint8_t payload_type:2;

    uint8_t avg_ext_temp;

    uint8_t max_sound_level_msb:1;
    uint8_t avg_ext_hum:7;

    uint8_t epa_oaq_lsb; // 3
    uint8_t max_oaq_lsb; // 4
    uint8_t avg_sound_level_lsb:4;
    uint8_t max_sound_level_lsb:4;
    uint8_t max_int_temp;
    uint8_t avg_sound_level_msb:1;
    uint8_t max_int_hum:7;
} __attribute__((packed));

struct payload_type0_debug
{
    payload_type0 p0;
    uint16_t battery;
};

struct payload_type1
{
    struct payload_type0 p0;
    uint8_t battery_level:7;
    uint8_t system_restart:1;
    uint8_t errors_logged;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif
