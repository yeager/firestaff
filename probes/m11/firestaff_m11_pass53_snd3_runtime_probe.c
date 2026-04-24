#include "audio_sdl_m11.h"
#include "graphics_dat_snd3_loader_v1.h"
#include "sound_event_snd3_map_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int file_exists(const char* path) {
    FILE* f;
    if (!path || !*path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static const char* find_graphics_dat(char* buf, size_t cap) {
    const char* envPath = getenv("FIRESTAFF_GRAPHICS_DAT");
    const char* home;
    if (file_exists(envPath)) return envPath;
    if (file_exists("GRAPHICS.DAT")) return "GRAPHICS.DAT";
    home = getenv("HOME");
    if (home && buf && cap > 0) {
        int n = snprintf(buf, cap, "%s/.firestaff/data/GRAPHICS.DAT", home);
        if (n > 0 && (size_t)n < cap && file_exists(buf)) return buf;
    }
    return NULL;
}

int main(void) {
    ProbeTally tally = {0, 0};
    M11_AudioState state;
    char graphicsPathBuf[1024];
    const char* graphicsPath;

    setenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3", "1", 1);
    M11_Audio_Init(&state);
    probe_record(&tally,
                 "P53_SND3_RUNTIME_01",
                 state.originalSnd3Available == 0 && state.originalSnd3LoadedCount == 0,
                 "fallback path does not require GRAPHICS.DAT/SND3 assets");
    {
        int beforeQueued = state.queuedSampleCount;
        int emitResult = M11_Audio_EmitSoundIndex(&state, 2, M11_AUDIO_MARKER_DOOR);
        int ok;
        if (state.backend == M11_AUDIO_BACKEND_SDL3) {
            ok = (emitResult == 1 && state.lastMarker == M11_AUDIO_MARKER_DOOR &&
                  state.queuedSampleCount > beforeQueued);
        } else {
            ok = (emitResult == 0 && state.lastMarker == M11_AUDIO_MARKER_DOOR &&
                  state.queuedSampleCount == beforeQueued);
        }
        probe_record(&tally,
                     "P53_SND3_RUNTIME_02",
                     ok,
                     "EmitSoundIndex falls back to procedural marker playback when original SND3 is disabled/unavailable");
    }
    M11_Audio_Shutdown(&state);
    unsetenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3");

    graphicsPath = find_graphics_dat(graphicsPathBuf, sizeof(graphicsPathBuf));
    if (!graphicsPath) {
        printf("SKIP P53_SND3_RUNTIME_ASSET no GRAPHICS.DAT found for original-SND3 branch\n");
    } else {
        V1_GraphicsSnd3Manifest manifest;
        V1_GraphicsSnd3Buffer raw;
        const V1_SoundEventSnd3MapEntry* doorEntry;
        char err[256];
        int expectedDoorSamples = 0;
        int actualDoorSamples = 0;
        int beforeQueued;
        int emitResult;
        setenv("FIRESTAFF_GRAPHICS_DAT", graphicsPath, 1);
        memset(&raw, 0, sizeof(raw));
        doorEntry = V1_SoundEventSnd3_Find(2);
        if (doorEntry &&
            V1_GraphicsSnd3_ParseManifest(graphicsPath, &manifest, err, sizeof(err)) &&
            V1_GraphicsSnd3_DecodeItem(graphicsPath, &manifest, doorEntry->snd3ItemIndex,
                                      &raw, err, sizeof(err))) {
            expectedDoorSamples = (int)(((long long)raw.decodedSampleCount * M11_AUDIO_SAMPLE_RATE +
                                         (M11_AUDIO_SOURCE_SND3_SAMPLE_RATE - 1)) /
                                        M11_AUDIO_SOURCE_SND3_SAMPLE_RATE);
            V1_GraphicsSnd3_FreeBuffer(&raw);
        }

        M11_Audio_Init(&state);
        probe_record(&tally,
                     "P53_SND3_RUNTIME_03",
                     state.originalSnd3Available == 1 &&
                         state.originalSnd3LoadedCount == M11_AUDIO_ORIGINAL_SOUND_COUNT,
                     "original GRAPHICS.DAT SND3 bank loads all 35 event-index buffers when asset is present");
        actualDoorSamples = state.originalSounds[2].sampleCount;
        probe_record(&tally,
                     "P53_SND3_RUNTIME_04",
                     expectedDoorSamples > 0 && actualDoorSamples == expectedDoorSamples,
                     "6000 Hz SND3 source is linearly resampled once to the fixed 22050 Hz SDL stream rate");
        beforeQueued = state.queuedSampleCount;
        emitResult = M11_Audio_EmitSoundIndex(&state, 2, M11_AUDIO_MARKER_DOOR);
        if (state.backend == M11_AUDIO_BACKEND_SDL3) {
            probe_record(&tally,
                         "P53_SND3_RUNTIME_05",
                         emitResult == 1 && state.lastSoundIndex == 2 &&
                             state.queuedSampleCount == beforeQueued + actualDoorSamples,
                         "SDL3 runtime queues the decoded/resampled SND3 buffer for a mapped sound event");
        } else {
            probe_record(&tally,
                         "P53_SND3_RUNTIME_05",
                         emitResult == 0 && state.lastMarker == M11_AUDIO_MARKER_DOOR,
                         "no-audio backend preserves procedural fallback while original SND3 buffers remain loaded");
        }
        M11_Audio_Shutdown(&state);
    }

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
