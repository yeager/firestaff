#include "audio_sdl_m11.h"
#include "song_dat_loader_v1.h"

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

static char* dup_env_value(const char* value) {
    size_t len;
    char* copy;
    if (!value) return NULL;
    len = strlen(value) + 1u;
    copy = (char*)malloc(len);
    if (!copy) return NULL;
    memcpy(copy, value, len);
    return copy;
}

static void restore_env_value(const char* name, const char* value) {
    if (!name) return;
    if (value) {
        setenv(name, value, 1);
    } else {
        unsetenv(name);
    }
}

static int has_arg(int argc, char** argv, const char* needle) {
    int i;
    if (!needle) return 0;
    for (i = 1; i < argc; ++i) {
        if (argv[i] && strcmp(argv[i], needle) == 0) return 1;
    }
    return 0;
}

static const char* find_song_dat(char* buf, size_t cap) {
    const char* envPath = getenv("FIRESTAFF_SONG_DAT");
    const char* legacyEnvPath = getenv("SONG_DAT_PATH");
    const char* home;
    if (file_exists(envPath)) return envPath;
    if (file_exists(legacyEnvPath)) return legacyEnvPath;
    if (file_exists("SONG.DAT")) return "SONG.DAT";
    home = getenv("HOME");
    if (home && buf && cap > 0) {
        int n = snprintf(buf, cap, "%s/.firestaff/data/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && file_exists(buf)) return buf;
        n = snprintf(buf, cap, "%s/.firestaff/data/dm1-multilingual/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && file_exists(buf)) return buf;
        n = snprintf(buf, cap, "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && file_exists(buf)) return buf;
        n = snprintf(buf, cap, "%s/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && file_exists(buf)) return buf;
    }
    if (file_exists("/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT")) {
        return "/tmp/fs_pass50_extract/dm_dos/DungeonMasterPC34/DATA/SONG.DAT";
    }
    return NULL;
}

static int expected_title_samples(const char* path) {
    V1_SongManifest manifest;
    V1_SongSequence seq;
    char err[256];
    int total = 0;
    unsigned int i;
    if (!V1_Song_ParseManifest(path, &manifest, err, sizeof(err)) ||
        !V1_Song_DecodeSequence(path, &manifest, &seq, err, sizeof(err))) {
        return 0;
    }
    for (i = 0; i < seq.wordCount; ++i) {
        unsigned int word = seq.words[i];
        unsigned int itemIndex = word & 0x7FFFu;
        V1_SndBuffer raw;
        if (word & 0x8000u) break;
        memset(&raw, 0, sizeof(raw));
        if (itemIndex < V1_SONG_DAT_FIRST_SND8_INDEX ||
            itemIndex > V1_SONG_DAT_LAST_SND8_INDEX ||
            !V1_Song_DecodeSnd8(path, &manifest, itemIndex, &raw, err, sizeof(err))) {
            V1_Song_FreeSndBuffer(&raw);
            return 0;
        }
        total += (int)(((long long)raw.decodedSampleCount * M11_AUDIO_SAMPLE_RATE +
                        (M11_AUDIO_SOURCE_SND8_SAMPLE_RATE - 1)) /
                       M11_AUDIO_SOURCE_SND8_SAMPLE_RATE);
        V1_Song_FreeSndBuffer(&raw);
    }
    return total;
}

static void run_live_sdl_queue_probe(ProbeTally* tally) {
    M11_AudioState state;
    float* samples;
    int beforeQueued;
    int playResult;

    setenv("FIRESTAFF_AUDIO_ENABLE_SDL", "1", 1);
    if (!getenv("SDL_AUDIODRIVER")) {
        setenv("SDL_AUDIODRIVER", "dummy", 1);
    }
    setenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG", "1", 1);

    M11_Audio_Init(&state);
    probe_record(tally,
                 "P54_SONG_RUNTIME_07",
                 state.backend == M11_AUDIO_BACKEND_SDL3 && state.sdlStream != NULL,
                 "FIRESTAFF_AUDIO_ENABLE_SDL=1 opens an SDL3 stream under the dummy audio driver");

    samples = (float*)malloc(8u * sizeof(float));
    if (samples) {
        int i;
        for (i = 0; i < 8; ++i) {
            samples[i] = (i & 1) ? -0.125f : 0.125f;
        }
    }
    state.originalSongAvailable = samples ? 1 : 0;
    state.titleMusic.samples = samples;
    state.titleMusic.sampleCount = samples ? 8 : 0;
    state.titleMusic.capacity = samples ? 8 : 0;

    beforeQueued = state.queuedSampleCount;
    playResult = M11_Audio_PlayTitleMusic(&state);
    probe_record(tally,
                 "P54_SONG_RUNTIME_08",
                 playResult == 1 &&
                     state.titleMusicQueuedCount == 1 &&
                     state.queuedSampleCount == beforeQueued + 8,
                 "title-music queue path pushes deterministic PCM to the opt-in SDL stream without audible hardware");

    M11_Audio_Shutdown(&state);
    unsetenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG");
}

int main(int argc, char** argv) {
    ProbeTally tally = {0, 0};
    M11_AudioState state;
    char songPathBuf[1024];
    const char* songPath;
    char* savedAudioEnable = dup_env_value(getenv("FIRESTAFF_AUDIO_ENABLE_SDL"));
    char* savedAudioDriver = dup_env_value(getenv("SDL_AUDIODRIVER"));
    int expectLiveSdlQueue = has_arg(argc, argv, "--expect-sdl-title-queue");

    setenv("FIRESTAFF_AUDIO_ENABLE_SDL", "0", 1);
    setenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG", "1", 1);
    M11_Audio_Init(&state);
    probe_record(&tally,
                 "P54_SONG_RUNTIME_00",
                 state.backend == M11_AUDIO_BACKEND_NONE && state.sdlStream == NULL,
                 "SDL playback remains disabled unless FIRESTAFF_AUDIO_ENABLE_SDL opts in");
    probe_record(&tally,
                 "P54_SONG_RUNTIME_01",
                 state.originalSongAvailable == 0 && state.titleMusic.sampleCount == 0,
                 "fallback/no-op path does not require SONG.DAT assets");
    {
        int beforeQueued = state.queuedSampleCount;
        int playResult = M11_Audio_PlayTitleMusic(&state);
        probe_record(&tally,
                     "P54_SONG_RUNTIME_02",
                     playResult == 0 && state.queuedSampleCount == beforeQueued &&
                         state.titleMusicQueuedCount == 0,
                     "PlayTitleMusic is a safe no-op when original SONG.DAT is unavailable/disabled");
    }
    M11_Audio_Shutdown(&state);
    unsetenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG");
    restore_env_value("FIRESTAFF_AUDIO_ENABLE_SDL", savedAudioEnable);

    if (expectLiveSdlQueue) {
        run_live_sdl_queue_probe(&tally);
        restore_env_value("FIRESTAFF_AUDIO_ENABLE_SDL", savedAudioEnable);
    }

    M11_Audio_Init(&state);
    probe_record(&tally,
                 "P54_SONG_RUNTIME_03",
                 M11_Audio_TitleMusicEnabled(&state) == 1 &&
                     M11_Audio_SetTitleMusicEnabled(&state, 0) == 1 &&
                     M11_Audio_TitleMusicEnabled(&state) == 0,
                 "title music runtime gate tracks G2024_B_PendingMusicOn-style on/off state");
    M11_Audio_Shutdown(&state);

    songPath = find_song_dat(songPathBuf, sizeof(songPathBuf));
    if (!songPath) {
        printf("SKIP P54_SONG_RUNTIME_ASSET no SONG.DAT found for original-title-music branch\n");
    } else {
        int expectedSamples = expected_title_samples(songPath);
        int beforeQueued;
        int playResult;
        setenv("FIRESTAFF_SONG_DAT", songPath, 1);
        M11_Audio_Init(&state);
        probe_record(&tally,
                     "P54_SONG_RUNTIME_04",
                     state.originalSongAvailable == 1 &&
                         state.originalSongPartCount == V1_SONG_DAT_MUSIC_PART_COUNT,
                     "original SONG.DAT loads all 9 SND8 music-part buffers when asset is present");
        probe_record(&tally,
                     "P54_SONG_RUNTIME_05",
                     state.originalSongSequenceWordCount == 20 &&
                         state.originalSongPlayablePartCount == 19 &&
                         state.originalSongLoopTargetPart == 1,
                     "SEQ2 walk stops at the bit-15 loop-back marker and records loop target part 1");
        probe_record(&tally,
                     "P54_SONG_RUNTIME_06",
                     expectedSamples > 0 && state.titleMusic.sampleCount == expectedSamples,
                     "11025 Hz signed SND8 parts are linearly resampled/concatenated for the fixed 22050 Hz stream");
        beforeQueued = state.queuedSampleCount;
        playResult = M11_Audio_PlayTitleMusic(&state);
        if (state.backend == M11_AUDIO_BACKEND_SDL3) {
            probe_record(&tally,
                         "P54_SONG_RUNTIME_07",
                         playResult == 1 && state.titleMusicQueuedCount == 1 &&
                             state.queuedSampleCount == beforeQueued + state.titleMusic.sampleCount,
                         "SDL3 runtime queues the decoded/resampled one-cycle title-music phrase");
        } else {
            probe_record(&tally,
                         "P54_SONG_RUNTIME_07",
                         playResult == 0 && state.titleMusicQueuedCount == 0 &&
                             state.queuedSampleCount == beforeQueued,
                         "no-audio backend keeps title music loaded but does not queue samples");
        }
        M11_Audio_Shutdown(&state);
        unsetenv("FIRESTAFF_SONG_DAT");
    }

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    restore_env_value("FIRESTAFF_AUDIO_ENABLE_SDL", savedAudioEnable);
    restore_env_value("SDL_AUDIODRIVER", savedAudioDriver);
    free(savedAudioEnable);
    free(savedAudioDriver);
    return (tally.passed == tally.total) ? 0 : 1;
}
