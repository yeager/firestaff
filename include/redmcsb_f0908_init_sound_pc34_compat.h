#ifndef FIRESTAFF_REDMCSB_F0908_INIT_SOUND_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0908_INIT_SOUND_PC34_COMPAT_H

/*
 * ReDMCSB SWSHSND.C F0908_InitSound.
 *
 * The MEDIA428 path allocates a chip-memory copy of G0746, assigns that
 * same copy, its byte count, and G0744_i_Period to both swoosh channels,
 * then copies the original bytes. This host boundary preserves only that
 * data ownership and channel-descriptor contract. It does not open an
 * audio device, allocate audio channels, or start playback.
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *data;
    size_t length;
    int16_t period;
} RedmcsbF0908SoundChannelPc34;

typedef struct {
    RedmcsbF0908SoundChannelPc34 left;
    RedmcsbF0908SoundChannelPc34 right;
} RedmcsbF0908SoundStatePc34;

/*
 * Copies the real G0746 swoosh bytes into caller-owned storage and binds the
 * result to both channels as F0908 does. Returns 1 on success and 0 when the
 * supplied host storage cannot represent the source operation.
 */
int RedmcsbF0908_InitSoundPc34(const uint8_t *swoosh_sound_data,
                               size_t swoosh_sound_data_byte_count,
                               int16_t period,
                               uint8_t *owned_sound_data,
                               size_t owned_sound_data_capacity,
                               RedmcsbF0908SoundStatePc34 *out_state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0908_INIT_SOUND_PC34_COMPAT_H */
