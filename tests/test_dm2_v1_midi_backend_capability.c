#include "dm2_v1_midi_backend.h"

#include <stdio.h>

int main(void)
{
    DM2_V1_MidiBackendState state;
#ifdef __APPLE__
    if (!dm2_v1_midi_backend_is_compiled()) {
        fprintf(stderr, "macOS build omitted the DM2 CoreMIDI backend\n");
        return 1;
    }
#else
    if (dm2_v1_midi_backend_is_compiled() ||
        dm2_v1_midi_backend_open() != DM2_V1_MIDI_BACKEND_UNAVAILABLE) {
        fprintf(stderr, "non-macOS build must expose explicit MIDI unavailability\n");
        return 1;
    }
#endif
    state = dm2_v1_midi_backend_open();
    if (state != DM2_V1_MIDI_BACKEND_UNAVAILABLE &&
        state != DM2_V1_MIDI_BACKEND_READY) {
        fprintf(stderr, "DM2 MIDI backend entered an invalid initial state\n");
        return 1;
    }
    dm2_v1_midi_backend_close();
    if (dm2_v1_midi_backend_state() != DM2_V1_MIDI_BACKEND_UNAVAILABLE) {
        fprintf(stderr, "DM2 MIDI backend did not restore unavailable state\n");
        return 1;
    }
    puts("PASS DM2 native MIDI backend capability is explicit");
    return 0;
}
