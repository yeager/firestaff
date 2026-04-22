#include "audio_sdl_m11.h"

#include <stdio.h>
#include <string.h>

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
    int master = -1, sfx = -1, music = -1, ui = -1;
    int emitResult = -1;
    int i;

    /* INV 01: Init succeeds (may get SDL3 backend or fallback) */
    probe_record(&tally,
                 "INV_M11_AUDIO_01",
                 M11_Audio_Init(&state) == 1 && state.initialized == 1,
                 "audio subsystem initializes (SDL3 or fallback)");

    printf("  backend: %s\n",
           state.backend == M11_AUDIO_BACKEND_SDL3 ? "SDL3" : "NONE (fallback)");

    /* INV 02: Sound buffers are pre-generated */
    {
        int allGenerated = 1;
        for (i = 1; i < M11_AUDIO_MARKER_COUNT; ++i) {
            if (state.sounds[i].sampleCount <= 0) {
                allGenerated = 0;
                break;
            }
        }
        probe_record(&tally,
                     "INV_M11_AUDIO_02",
                     allGenerated,
                     "procedural sound buffers generated for all marker types");
    }

    /* INV 03: Marker emission records marker regardless of backend */
    emitResult = M11_Audio_EmitMarker(&state, M11_AUDIO_MARKER_COMBAT);
    probe_record(&tally,
                 "INV_M11_AUDIO_03",
                 state.lastMarker == M11_AUDIO_MARKER_COMBAT,
                 "marker emission records the marker type");

    /* INV 04: If backend is SDL3, emit returns 1 and increments played count.
     * If fallback, emit returns 0 and played count stays 0. */
    if (state.backend == M11_AUDIO_BACKEND_SDL3) {
        probe_record(&tally,
                     "INV_M11_AUDIO_04",
                     emitResult == 1 && state.playedMarkerCount == 1,
                     "SDL3 backend: emit plays sound and increments count");
    } else {
        probe_record(&tally,
                     "INV_M11_AUDIO_04",
                     emitResult == 0 && state.playedMarkerCount == 0,
                     "fallback backend: emit accepted but not played");
    }

    /* INV 05: Volume clamping */
    M11_Audio_SetVolumes(&state, 200, -10, 64, 80);
    probe_record(&tally,
                 "INV_M11_AUDIO_05",
                 M11_Audio_GetVolumes(&state, &master, &sfx, &music, &ui) == 1 &&
                     master == 128 && sfx == 0 && music == 64 && ui == 80,
                 "volume updates clamp to valid range");

    /* INV 06: IsAvailable reflects actual backend */
    probe_record(&tally,
                 "INV_M11_AUDIO_06",
                 M11_Audio_IsAvailable(&state) ==
                     (state.backend == M11_AUDIO_BACKEND_SDL3 ? 1 : 0),
                 "IsAvailable reflects backend state");

    /* INV 07: Multiple rapid emissions don't crash */
    {
        int beforeCount = state.playedMarkerCount;
        for (i = 0; i < 20; ++i) {
            M11_Audio_EmitMarker(&state,
                (M11_AudioMarker)(1 + (i % (M11_AUDIO_MARKER_COUNT - 1))));
        }
        if (state.backend == M11_AUDIO_BACKEND_SDL3) {
            probe_record(&tally,
                         "INV_M11_AUDIO_07",
                         state.playedMarkerCount == beforeCount + 20,
                         "20 rapid emissions succeed without crash (SDL3)");
        } else {
            probe_record(&tally,
                         "INV_M11_AUDIO_07",
                         state.playedMarkerCount == beforeCount,
                         "20 rapid emissions in fallback mode accepted safely");
        }
    }

    /* INV 08: If SDL3 backend, verify samples were queued */
    if (state.backend == M11_AUDIO_BACKEND_SDL3) {
        probe_record(&tally,
                     "INV_M11_AUDIO_08",
                     state.queuedSampleCount > 0,
                     "SDL3 backend: samples were queued to audio device");
    } else {
        probe_record(&tally,
                     "INV_M11_AUDIO_08",
                     state.queuedSampleCount == 0,
                     "fallback backend: no samples queued (expected)");
    }

    M11_Audio_Shutdown(&state);

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
