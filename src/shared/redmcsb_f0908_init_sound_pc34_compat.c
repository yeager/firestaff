/* ReDMCSB SWSHSND.C:10-24 F0908_InitSound (MEDIA428). */
#include "redmcsb_f0908_init_sound_pc34_compat.h"

#include <string.h>

enum {
    REDMCSB_F0908_SWOOSH_BYTE_COUNT_PC34 = 9078,
    REDMCSB_F0908_SWOOSH_PERIOD_PC34 = 334
};

static uint32_t redmcsb_f0908_fnv1a(const uint8_t *data, size_t size)
{
    uint32_t value = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        value ^= data[i];
        value *= 16777619u;
    }
    return value;
}

int RedmcsbF0908_InitSoundPc34(const uint8_t *swoosh_sound_data,
                               size_t swoosh_sound_data_byte_count,
                               int16_t period,
                               uint32_t source_sample_fnv1a,
                               uint8_t *owned_sound_data,
                               size_t owned_sound_data_capacity,
                               RedmcsbF0908SoundStatePc34 *out_state)
{
    if (!out_state || !swoosh_sound_data || !owned_sound_data ||
        swoosh_sound_data_byte_count != REDMCSB_F0908_SWOOSH_BYTE_COUNT_PC34 ||
        period != REDMCSB_F0908_SWOOSH_PERIOD_PC34 ||
        source_sample_fnv1a == 0u ||
        redmcsb_f0908_fnv1a(swoosh_sound_data,
                             swoosh_sound_data_byte_count) != source_sample_fnv1a ||
        owned_sound_data_capacity < swoosh_sound_data_byte_count) {
        return 0;
    }

    memcpy(owned_sound_data, swoosh_sound_data, swoosh_sound_data_byte_count);

    out_state->left.data = owned_sound_data;
    out_state->left.length = swoosh_sound_data_byte_count;
    out_state->left.period = period;
    out_state->right = out_state->left;
    return 1;
}
