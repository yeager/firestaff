/* ReDMCSB SWSHSND.C:10-24 F0908_InitSound (MEDIA428). */
#include "redmcsb_f0908_init_sound_pc34_compat.h"

#include <string.h>

int RedmcsbF0908_InitSoundPc34(const uint8_t *swoosh_sound_data,
                               size_t swoosh_sound_data_byte_count,
                               int16_t period,
                               uint8_t *owned_sound_data,
                               size_t owned_sound_data_capacity,
                               RedmcsbF0908SoundStatePc34 *out_state)
{
    if (!out_state ||
        (swoosh_sound_data_byte_count != 0u &&
         (!swoosh_sound_data || !owned_sound_data)) ||
        owned_sound_data_capacity < swoosh_sound_data_byte_count) {
        return 0;
    }

    if (swoosh_sound_data_byte_count != 0u) {
        memcpy(owned_sound_data, swoosh_sound_data,
               swoosh_sound_data_byte_count);
    }

    out_state->left.data = owned_sound_data;
    out_state->left.length = swoosh_sound_data_byte_count;
    out_state->left.period = period;
    out_state->right = out_state->left;
    return 1;
}
