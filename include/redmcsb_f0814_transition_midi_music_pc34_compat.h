/*
 * ReDMCSB IO.C F0814_TRansition_MIDIMusic, PC-98 MIDI route.
 */
#ifndef FIRESTAFF_REDMCSB_F0814_TRANSITION_MIDI_MUSIC_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0814_TRANSITION_MIDI_MUSIC_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * IO.C passes command 2 directly to IODRV_27_.  The platform driver owns
 * the actual fade/transition; this compatibility boundary does not create
 * audio or emulate a MIDI state machine.
 */
typedef void (*redmcsb_f0814_midi_music_control_pc34_compat)(
    int16_t command,
    void *context);

void redmcsb_f0814_transition_midi_music_pc34_compat(
    redmcsb_f0814_midi_music_control_pc34_compat midi_music_control,
    void *context);

const char *redmcsb_f0814_transition_midi_music_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
