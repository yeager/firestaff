#ifndef FIRESTAFF_DM2_V1_MIDI_BACKEND_H
#define FIRESTAFF_DM2_V1_MIDI_BACKEND_H

#include "dm2_v1_sound.h"

/* Native output only. This adapter never renders MIDI into PCM. */
typedef enum DM2_V1_MidiBackendState {
    DM2_V1_MIDI_BACKEND_UNAVAILABLE = 0,
    DM2_V1_MIDI_BACKEND_READY,
    DM2_V1_MIDI_BACKEND_DELIVERY_FAILED
} DM2_V1_MidiBackendState;

int dm2_v1_midi_backend_is_compiled(void);
DM2_V1_MidiBackendState dm2_v1_midi_backend_open(void);
DM2_V1_MidiBackendState dm2_v1_midi_backend_state(void);
int dm2_v1_midi_backend_send(const DM2_V1_MusicScheduledEvent *event);
void dm2_v1_midi_backend_close(void);

#endif /* FIRESTAFF_DM2_V1_MIDI_BACKEND_H */
