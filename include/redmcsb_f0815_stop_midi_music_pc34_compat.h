/*
 * ReDMCSB IO.C F0815_StopMIDIMusic, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0815_STOP_MIDI_MUSIC_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0815_STOP_MIDI_MUSIC_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO.C dispatches command 1 through IODRV_27. The host owns any real MIDI
 * backend; this bridge neither decodes music nor creates audio state.
 */
typedef void (*redmcsb_f0815_midi_driver_pc34_compat)(int command, void *context);

void redmcsb_f0815_stop_midi_music_pc34_compat(
    redmcsb_f0815_midi_driver_pc34_compat midi_driver,
    void *context);

const char *redmcsb_f0815_stop_midi_music_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
