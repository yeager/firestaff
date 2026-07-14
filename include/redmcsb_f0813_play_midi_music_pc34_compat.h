#ifndef FIRESTAFF_REDMCSB_F0813_PLAY_MIDI_MUSIC_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0813_PLAY_MIDI_MUSIC_PC34_COMPAT_H

/*
 * Source lock: ReDMCSB WIP IO.C:4175-4179 (PC 3.4).
 *
 * F0813 is deliberately only the IODRV_27 command-0 handoff.  The driver
 * owns MIDI decoding and playback; this boundary neither synthesizes audio
 * nor interprets the source-backed data passed to it.
 */
typedef void (*ReDMCSB_F0813_MidiDriverFn)(int command,
                                           const char *midi_data,
                                           void *context);

typedef struct ReDMCSB_F0813_MidiDriver {
    ReDMCSB_F0813_MidiDriverFn iodrv_27;
    void *context;
} ReDMCSB_F0813_MidiDriver;

void ReDMCSB_F0813_PlayMIDIMusic_PC34_Compat(
    const ReDMCSB_F0813_MidiDriver *driver,
    const char *midi_data);

#endif
