#include <stdint.h>
#include <string.h>

#include "redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat.h"

static void initialize_sounds(
    RedmcsbF0799SoundDataPc34 sounds[REDMCSB_F0799_PC34_SOUND_COUNT])
{
    int16_t index;

    for (index = 0; index < REDMCSB_F0799_PC34_SOUND_COUNT; index++) {
        sounds[index].graphic_index = index;
    }
}

static int range_is_none(
    const RedmcsbF0799SoundDataPc34 sounds[REDMCSB_F0799_PC34_SOUND_COUNT],
    int16_t first,
    int16_t last)
{
    int16_t index;

    for (index = first; index < last; index++) {
        if (sounds[index].graphic_index != REDMCSB_F0799_PC34_SOUND_NONE) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    RedmcsbF0799SoundDataPc34 sounds[REDMCSB_F0799_PC34_SOUND_COUNT];
    uint8_t flags[REDMCSB_F0799_PC34_MEMORY_FLAG_COUNT] = {0};

    initialize_sounds(sounds);
    flags[REDMCSB_F0799_PC34_MEMORY_BASE_SOUNDS] = 1;
    flags[REDMCSB_F0799_PC34_MEMORY_ATTACK_SOUNDS] = 1;
    flags[REDMCSB_F0799_PC34_MEMORY_MOVEMENT_SOUNDS] = 1;
    redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat(sounds, flags);
    if (sounds[0].graphic_index != 0 ||
        sounds[REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK].graphic_index !=
            REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK ||
        sounds[REDMCSB_F0799_PC34_SOUND_COUNT - 1].graphic_index !=
            REDMCSB_F0799_PC34_SOUND_COUNT - 1) {
        return 1;
    }

    initialize_sounds(sounds);
    memset(flags, 0, sizeof(flags));
    flags[REDMCSB_F0799_PC34_MEMORY_ATTACK_SOUNDS] = 1;
    if (!range_is_none(sounds, 0, 0)) {
        return 1;
    }
    redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat(sounds, flags);
    if (!range_is_none(sounds, 0, REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK) ||
        sounds[REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK].graphic_index !=
            REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK ||
        !range_is_none(
            sounds,
            REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT,
            REDMCSB_F0799_PC34_SOUND_COUNT)) {
        return 1;
    }

    initialize_sounds(sounds);
    memset(flags, 0, sizeof(flags));
    flags[REDMCSB_F0799_PC34_MEMORY_BASE_SOUNDS] = 1;
    flags[REDMCSB_F0799_PC34_MEMORY_MOVEMENT_SOUNDS] = 1;
    redmcsb_f0799_sound_disable_unavailable_sounds_pc34_compat(sounds, flags);
    if (sounds[0].graphic_index != 0 ||
        !range_is_none(
            sounds,
            REDMCSB_F0799_PC34_SOUND_FIRST_ATTACK,
            REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT) ||
        sounds[REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT].graphic_index !=
            REDMCSB_F0799_PC34_SOUND_FIRST_MOVEMENT) {
        return 1;
    }

    if (strcmp(
            redmcsb_f0799_sound_disable_unavailable_sounds_source_evidence_pc34(),
            "ReDMCSB SOUND.C:1397-1419; DEFS.H M513_SOUND_COUNT=35 for I34E/I34M") !=
        0) {
        return 1;
    }

    return 0;
}
