/* ReDMCSB IO.C:4181-4186, PC-98 MIDI transition route. */
#include "redmcsb_f0814_transition_midi_music_pc34_compat.h"

void redmcsb_f0814_transition_midi_music_pc34_compat(
    redmcsb_f0814_midi_music_control_pc34_compat midi_music_control,
    void *context)
{
    midi_music_control(2, context);
}

const char *redmcsb_f0814_transition_midi_music_source_evidence_pc34(void)
{
    return "ReDMCSB_WIP20210206/Toolchains/Common/Source/IO.C:4181-4186";
}
