#include "audio_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif

typedef struct {
    int total;
    int passed;
} ProbeTally;

static int probe_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name, value) ? 0 : -1;
#else
    return setenv(name, value, 1);
#endif
}

static int probe_unsetenv(const char* name) {
#ifdef _WIN32
    return SetEnvironmentVariableA(name, NULL) ? 0 : -1;
#else
    return unsetenv(name);
#endif
}

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


static int write_u16_le(FILE* f, unsigned int v) {
    unsigned char b[2];
    b[0] = (unsigned char)(v & 0xFFu);
    b[1] = (unsigned char)((v >> 8) & 0xFFu);
    return fwrite(b, 1, sizeof(b), f) == sizeof(b);
}

static int write_u32_le(FILE* f, unsigned int v) {
    unsigned char b[4];
    b[0] = (unsigned char)(v & 0xFFu);
    b[1] = (unsigned char)((v >> 8) & 0xFFu);
    b[2] = (unsigned char)((v >> 16) & 0xFFu);
    b[3] = (unsigned char)((v >> 24) & 0xFFu);
    return fwrite(b, 1, sizeof(b), f) == sizeof(b);
}

static int write_test_wav(const char* path) {
    FILE* f;
    unsigned int sampleRate = 11025u;
    unsigned int sampleCount = 110u;
    unsigned int i;
    if (!path) return 0;
    f = fopen(path, "wb");
    if (!f) return 0;
    if (fwrite("RIFF", 1, 4, f) != 4 ||
        !write_u32_le(f, 36u + sampleCount) ||
        fwrite("WAVEfmt ", 1, 8, f) != 8 ||
        !write_u32_le(f, 16u) ||
        !write_u16_le(f, 1u) ||
        !write_u16_le(f, 1u) ||
        !write_u32_le(f, sampleRate) ||
        !write_u32_le(f, sampleRate) ||
        !write_u16_le(f, 1u) ||
        !write_u16_le(f, 8u) ||
        fwrite("data", 1, 4, f) != 4 ||
        !write_u32_le(f, sampleCount)) {
        fclose(f);
        return 0;
    }
    for (i = 0; i < sampleCount; ++i) {
        unsigned char sample = (unsigned char)(96u + (i % 64u));
        if (fwrite(&sample, 1, 1, f) != 1) {
            fclose(f);
            return 0;
        }
    }
    return fclose(f) == 0;
}

int main(int argc, char** argv) {
    M11_AudioState state;
    ProbeTally tally = {0, 0};
    int master = -1, sfx = -1, music = -1, ui = -1;
    int emitResult = -1;
    int i;
    int expectSdlSourceIndexQueue = 0;
    char packDirTemplate[] = "/tmp/firestaff-sound-pack-XXXXXX";
    char* packDir = mkdtemp(packDirTemplate);
    char packPath[256];
    int packFixtureReady = 0;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--expect-sdl-source-index-queue") == 0) {
            expectSdlSourceIndexQueue = 1;
        }
    }

    if (packDir) {
        int n = snprintf(packPath, sizeof(packPath), "%s/13.wav", packDir);
        if (n > 0 && (size_t)n < sizeof(packPath)) {
            packFixtureReady = write_test_wav(packPath);
        }
    }
    if (packFixtureReady) {
        probe_setenv("FIRESTAFF_SOUND_PACK_DIR", packDir);
        probe_setenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3", "1");
    }

    /* INV 01: Init succeeds (may get SDL3 backend or fallback) */
    probe_record(&tally,
                 "INV_M11_AUDIO_01",
                 M11_Audio_Init(&state) == 1 && state.initialized == 1,
                 "audio subsystem initializes (SDL3 or fallback)");

    printf("  backend: %s\n",
           state.backend == M11_AUDIO_BACKEND_SDL3 ? "SDL3" : "NONE (fallback)");

    probe_record(&tally,
                 "INV_M11_AUDIO_00",
                 !packFixtureReady ||
                     (M11_Audio_SoundPackAvailable(&state) == 1 &&
                      state.soundPackLoadedCount == 1 &&
                      state.originalSounds[13].sampleCount > 0 &&
                      state.originalSounds[2].sampleCount == 0),
                 "optional sound-pack WAV overrides are source-index keyed");

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

    if (expectSdlSourceIndexQueue) {
        int beforeQueued = state.queuedSampleCount;
        int beforePlayed = state.playedMarkerCount;
        int sourceResult;

        probe_record(&tally,
                     "INV_M11_AUDIO_06B",
                     state.backend == M11_AUDIO_BACKEND_SDL3 &&
                         packFixtureReady &&
                         M11_Audio_SoundPackAvailable(&state) == 1,
                     "SDL dummy backend has source-index sound-pack data ready");

        sourceResult = M11_Audio_EmitSourceSoundIndex(&state, 13);
        probe_record(&tally,
                     "INV_M11_AUDIO_06C",
                     sourceResult == 1 &&
                         state.lastSoundIndex == 13 &&
                         state.lastMarker == M11_AUDIO_MARKER_COMBAT &&
                         state.queuedSampleCount > beforeQueued &&
                         state.playedMarkerCount == beforePlayed + 1,
                     "source sound index 13 queues PCM through SDL dummy backend");
    }

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

    if (packFixtureReady) {
        probe_unsetenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SND3");
        probe_unsetenv("FIRESTAFF_SOUND_PACK_DIR");
        unlink(packPath);
    }
    if (packDir) rmdir(packDir);

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
