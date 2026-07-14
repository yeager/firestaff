#include "redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat.h"

void redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat(
    RedmcsbF0799SoundDataPc34 sounds[REDMCSB_F0799_PC34_SOUND_COUNT],
    const uint8_t memory_flags[REDMCSB_F0799_PC34_MEMORY_FLAG_COUNT])
{
    int16_t sound_index;

    if (!memory_flags[REDMCSB_F0799_PC34_MEMORY_BASE_SOUNDS]) {
        for (sound_index = 0;
             sound_index < REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK;
             sound_index++) {
            sounds[sound_index].graphic_index = REDMCSB_F0799_PC34_SOUND_NONE;
        }
    }
    if (!memory_flags[REDMCSB_F0799_PC34_MEMORY_ATTACK_SOUNDS]) {
        for (sound_index = 0;
             sound_index < REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT -
                               REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK;
             sound_index++) {
            sounds[sound_index + REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK].graphic_index =
                REDMCSB_F0799_PC34_SOUND_NONE;
        }
    }
    if (!memory_flags[REDMCSB_F0799_PC34_MEMORY_MOVEMENT_SOUNDS]) {
        for (sound_index = 0;
             sound_index < REDMCSB_F0799_PC34_SOUND_COUNT -
                               REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT;
             sound_index++) {
            sounds[sound_index + REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT].graphic_index =
                REDMCSB_F0799_PC34_SOUND_NONE;
        }
    }
}

const char *redmcsb_f0799_sound_disable_unavailable_sounds_source_evidence_pc34(void)
{
    return "ReDMCSB SOUND.C:1397-1419; DEFS.H M513_SOUND_COUNT=35 for I34E/I34M";
}
