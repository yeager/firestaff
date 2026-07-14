#include "redmcsb_f0709_start_sound_pc34_compat.h"

#include <stddef.h>

bool redmcsb_f0709_start_sound_pc34_compat(
    const redmcsb_f0709_sound_route_pc34_compat *route,
    int16_t sound_index,
    int16_t sound_volume)
{
    const redmcsb_f0709_sound_descriptor_pc34_compat *sound;

    if (route == NULL || route->lookup == NULL || route->play_cpsx == NULL) {
        return false;
    }

    sound = route->lookup(route->context, sound_index);
    if (sound == NULL) {
        return false;
    }

    /* IO.C:3840 admits negative indices or a non-negative graphic index. */
    if (sound_index < 0 || sound->graphic_index >= 0) {
        route->play_cpsx(route->context, sound->buffer, sound_volume, 6000);
        return true;
    }

    return false;
}

const char *redmcsb_f0709_start_sound_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:3832-3843, MEDIA483_P20JB_I34E_I34M_X31J_P31J: "
           "F0709 reads G0060 sound buffer and calls F0060_SOUND_Play_CPSX "
           "with the original volume and period 6000 when the source guard "
           "admits the descriptor.";
}
