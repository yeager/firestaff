#ifndef DM2_V1_MAC_SOUND_H
#define DM2_V1_MAC_SOUND_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int valid;
    int16_t resource_id;
    uint16_t format;
    uint16_t data_type;
    uint32_t init_option;
    uint32_t sample_rate_fixed;
    uint32_t sample_length;
    uint32_t loop_start;
    uint32_t loop_end;
    uint8_t encode;
    uint8_t base_frequency;
    const uint8_t *sample_data;
    size_t sample_data_size;
} DM2_V1_MacSoundSample;

/* Inspect authentic Classic Mac Sound Manager resources in a borrowed
 * Resource Manager fork. No bytes are copied or converted. */
size_t dm2_v1_mac_sound_count(const uint8_t *fork, size_t fork_size);
int dm2_v1_mac_sound_find(const uint8_t *fork, size_t fork_size,
                          int16_t resource_id,
                          DM2_V1_MacSoundSample *out);

#endif
