#include "redmcsb_f0813_play_midi_music_pc34_compat.h"

void ReDMCSB_F0813_PlayMIDIMusic_PC34_Compat(
    const ReDMCSB_F0813_MidiDriver *driver,
    const char *midi_data)
{
    driver->iodrv_27(0, midi_data, driver->context);
}
