#include "redmcsb_f0815_stop_midi_music_pc34_compat.h"

void redmcsb_f0815_stop_midi_music_pc34_compat(
    redmcsb_f0815_midi_driver_pc34_compat midi_driver,
    void *context)
{
    midi_driver(1, context);
}

const char *redmcsb_f0815_stop_midi_music_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:4188-4192; IODRV_27 command 1";
}
