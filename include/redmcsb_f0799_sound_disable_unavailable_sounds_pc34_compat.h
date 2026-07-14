/*
 * ReDMCSB SOUND.C F0799_SOUND_DisableUnavailableSounds, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0799_SOUND_DISABLE_UNAVAILABLE_SOUNDS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0799_SOUND_DISABLE_UNAVAILABLE_SOUNDS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0799_PC34_SOUND_COUNT = 35,
    REDMCSB_F0799_PC34_MEMORY_FLAG_COUNT = 6,
    REDMCSB_F0799_PC34_MEMORY_BASE_SOUNDS = 3,
    REDMCSB_F0799_PC34_MEMORY_ATTACK_SOUNDS = 4,
    REDMCSB_F0799_PC34_MEMORY_MOVEMENT_SOUNDS = 5,
    REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK = 19,
    REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT = 28,
    REDMCSB_F0799_PC34_SOUND_NONE = -1
};

typedef struct RedmcsbF0799SoundDataPc34 {
    int16_t graphic_index;
} RedmcsbF0799SoundDataPc34;

/*
 * Mirrors the I34E/I34M PC 3.4 SOUND.C routine. The caller supplies the
 * original memory-availability flags and the already-loaded sound table.
 * No sound decoding, loading, or playback is performed here.
 */
void redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat(
    RedmcsbF0799SoundDataPc34 sounds[REDMCSB_F0799_PC34_SOUND_COUNT],
    const uint8_t memory_flags[REDMCSB_F0799_PC34_MEMORY_FLAG_COUNT]);

const char *redmcsb_f0799_sound_disable_unavailable_sounds_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
