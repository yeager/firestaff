#include "audio_sdl_m11.h"

#include <stdio.h>

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

int main(void) {
    M11_AudioState state;
    ProbeTally tally = {0, 0};
    int master = -1;
    int sfx = -1;
    int music = -1;
    int ui = -1;
    int emitResult = -1;

    probe_record(&tally,
                 "INV_M11_AUDIO_01",
                 M11_Audio_Init(&state) == 1 && state.initialized == 1,
                 "audio subsystem initializes safely in fallback mode");

    emitResult = M11_Audio_EmitMarker(&state, M11_AUDIO_MARKER_COMBAT);
    probe_record(&tally,
                 "INV_M11_AUDIO_02",
                 emitResult == 0 &&
                     state.lastMarker == M11_AUDIO_MARKER_COMBAT &&
                     state.playedMarkerCount == 0,
                 "marker emission is accepted without backend playback");

    M11_Audio_SetVolumes(&state, 200, -10, 64, 80);
    probe_record(&tally,
                 "INV_M11_AUDIO_03",
                 M11_Audio_GetVolumes(&state, &master, &sfx, &music, &ui) == 1 &&
                     master == 128 && sfx == 0 && music == 64 && ui == 80,
                 "volume updates clamp to valid mixer range");

    probe_record(&tally,
                 "INV_M11_AUDIO_04",
                 M11_Audio_IsAvailable(&state) == 0,
                 "missing SDL3_mixer backend degrades gracefully");

    M11_Audio_Shutdown(&state);

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
