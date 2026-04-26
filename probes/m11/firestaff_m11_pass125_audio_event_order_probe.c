#include "audio_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32)
#define setenv(k,v,o) _putenv_s((k),(v))
#define unsetenv(k) _putenv_s((k),"")
#endif

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
    ProbeTally tally = {0, 0};
    M11_AudioState state;
    int beforeQueued;
    int result;

    /* Keep the probe deterministic and headless: event ordering/identity only,
     * not SDL device playback and not waveform/cadence parity. */
    setenv("FIRESTAFF_AUDIO_ENABLE_SDL", "0", 1);
    setenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3", "1", 1);

    probe_record(&tally,
                 "P125_AUDIO_EVENT_ORDER_01",
                 M11_Audio_Init(&state) == 1 && state.backend == M11_AUDIO_BACKEND_NONE,
                 "headless fallback backend initializes without opening SDL audio");

    beforeQueued = state.queuedSampleCount;
    result = M11_Audio_EmitSoundIndex(&state, 13, M11_AUDIO_MARKER_COMBAT);
    probe_record(&tally,
                 "P125_AUDIO_EVENT_ORDER_02",
                 result == 0 && state.lastSoundIndex == 13 &&
                     state.lastMarker == M11_AUDIO_MARKER_COMBAT &&
                     state.queuedSampleCount == beforeQueued,
                 "mapped combat event preserves source sound index while falling back to marker/no-audio path");

    result = M11_Audio_EmitMarker(&state, M11_AUDIO_MARKER_SPELL);
    probe_record(&tally,
                 "P125_AUDIO_EVENT_ORDER_03",
                 result == 0 && state.lastSoundIndex == -1 &&
                     state.lastMarker == M11_AUDIO_MARKER_SPELL,
                 "direct marker emission remains distinguishable from source sound-event emission");

    result = M11_Audio_EmitSoundIndex(&state, 999, M11_AUDIO_MARKER_DOOR);
    probe_record(&tally,
                 "P125_AUDIO_EVENT_ORDER_04",
                 result == 0 && state.lastSoundIndex == -1 &&
                     state.lastMarker == M11_AUDIO_MARKER_DOOR,
                 "out-of-range sound index uses marker fallback without recording a false source event");

    M11_Audio_Shutdown(&state);
    unsetenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3");
    unsetenv("FIRESTAFF_AUDIO_ENABLE_SDL");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
