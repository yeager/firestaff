#include "audio_sdl_m11.h"
#include "song_dat_loader_v1.h"
#include "asset_find_by_hash.h"
#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"

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

static int song_path_valid(const char* path) {
    DM1_V1_F0740F0743MusicSourcePc34 source;
    return path && path[0] &&
        dm1_v1_f0740_f0743_bind_song_dat_pc34(path, &source);
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
    if (song_path_valid(envPath)) return envPath;
    if (song_path_valid(legacyEnvPath)) return legacyEnvPath;
    if (song_path_valid("SONG.DAT")) return "SONG.DAT";
    home = getenv("HOME");
    if (home && buf && cap > 0) {
        int n = snprintf(buf, cap, "%s/.firestaff/data/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && song_path_valid(buf)) return buf;
        n = snprintf(buf, cap, "%s/.firestaff/data/dm1-multilingual/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && song_path_valid(buf)) return buf;
        n = snprintf(buf, cap, "%s/.firestaff/data/firestaff-original-games/DM/_canonical/dm1/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && song_path_valid(buf)) return buf;
        n = snprintf(buf, cap, "%s/.firestaff/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/SONG.DAT", home);
        if (n > 0 && (size_t)n < cap && song_path_valid(buf)) return buf;
    }
    if (home && buf && cap > 0) {
        char search[1024];
        int n = snprintf(search, sizeof(search), "%s/.firestaff/data/dm1", home);
        if (n > 0 && (size_t)n < sizeof(search) &&
            asset_find_by_md5(search, "c20e5b8f756e360a631595cc9260f62d",
                              buf, (int)cap, 3) && song_path_valid(buf)) return buf;
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

static void run_live_sdl_queue_probe(ProbeTally* tally, const char* songPath) {
    M11_AudioState state;
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

    unsetenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG");
    probe_record(tally, "P54_SONG_RUNTIME_REAL_SOURCE",
        M11_Audio_BindOriginalSongPath(&state, songPath) &&
        state.originalSongAvailable && state.titleMusic.sampleCount > M11_AUDIO_TITLE_QUEUE_SAMPLES,
        "live queue uses decoded original SONG.DAT from the admitted path");
    (void)M11_Audio_SetHostPaused(&state, 1);
    beforeQueued = state.queuedSampleCount;
    playResult = M11_Audio_PlayTitleMusic(&state);
    probe_record(tally,
                 "P54_SONG_RUNTIME_08",
                 playResult == 1 &&
                     state.titleMusicQueuedCount == 1 &&
                     state.queuedSampleCount == beforeQueued + M11_AUDIO_TITLE_QUEUE_SAMPLES,
                 "title-music queue path pushes bounded authentic SONG PCM to the SDL stream");

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
    char* savedSongPath = dup_env_value(getenv("FIRESTAFF_SONG_DAT"));
    char* savedDisableSong = dup_env_value(getenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG"));
    int missingLiveMedia = 0;
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

    songPath = find_song_dat(songPathBuf, sizeof(songPathBuf));
    if (expectLiveSdlQueue && !songPath) {
        puts("SKIP: live title queue requires authentic SONG.DAT media");
        missingLiveMedia = 1;
        goto cleanup;
    }
    if (expectLiveSdlQueue) {
        run_live_sdl_queue_probe(&tally, songPath);
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

    if (!songPath) {
        printf("SKIP P54_SONG_RUNTIME_ASSET no SONG.DAT found for original-title-music branch\n");
    } else {
        int expectedSamples = expected_title_samples(songPath);
        int beforeQueued;
        int playResult;
        M11_Audio_Init(&state);
        (void)M11_Audio_BindOriginalSongPath(&state, songPath);
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
                     "SEQ2 walk stops at the bit-15 loop-back marker and records loop target sequence index 1");
        probe_record(&tally,
                     "P54_SONG_RUNTIME_06",
                     expectedSamples > 0 && state.titleMusic.sampleCount == expectedSamples,
                     "11126 Hz signed SND8 parts are linearly resampled/concatenated for the fixed 22050 Hz stream");
        beforeQueued = state.queuedSampleCount;
        playResult = M11_Audio_PlayTitleMusic(&state);
        if (state.backend == M11_AUDIO_BACKEND_SDL3) {
            probe_record(&tally,
                         "P54_SONG_RUNTIME_07",
                         playResult == 1 && state.titleMusicQueuedCount == 1 &&
                             state.queuedSampleCount == beforeQueued + M11_AUDIO_TITLE_QUEUE_SAMPLES,
                         "SDL3 runtime queues the decoded/resampled bounded title-music prefix");
        } else {
            probe_record(&tally,
                         "P54_SONG_RUNTIME_07",
                         playResult == 0 && state.titleMusicQueuedCount == 0 &&
                             state.queuedSampleCount == beforeQueued,
                         "no-audio backend keeps title music loaded but does not queue samples");
        }
        M11_Audio_Shutdown(&state);
    }

cleanup:
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    restore_env_value("FIRESTAFF_AUDIO_ENABLE_SDL", savedAudioEnable);
    restore_env_value("SDL_AUDIODRIVER", savedAudioDriver);
    restore_env_value("FIRESTAFF_SONG_DAT", savedSongPath);
    restore_env_value("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG", savedDisableSong);
    free(savedSongPath);
    free(savedDisableSong);
    free(savedAudioEnable);
    free(savedAudioDriver);
    return tally.passed != tally.total ? 1 : (missingLiveMedia ? 77 : 0);
}
