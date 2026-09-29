#include "audio_sdl_m11.h"
#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"
#include "fs_portable_compat.h"
#include "soundtrack_selector_m11.h"
#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define TEST_GETCWD _getcwd
#define TEST_CHDIR _chdir
#define TEST_RMDIR _rmdir
#define TEST_SETENV(k,v) _putenv_s((k),(v))
#define TEST_UNSETENV(k) _putenv_s((k), "")
#else
#include <unistd.h>
#define TEST_GETCWD getcwd
#define TEST_CHDIR chdir
#define TEST_RMDIR rmdir
#define TEST_SETENV(k,v) setenv((k),(v),1)
#define TEST_UNSETENV(k) unsetenv(k)
#endif

static int failures;
#define CHECK(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); ++failures; } } while (0)

static char* copy_env(const char* name)
{
    const char* value = getenv(name);
    char* copy;
    if (!value) return NULL;
    copy = (char*)malloc(strlen(value) + 1u);
    if (copy) strcpy(copy, value);
    return copy;
}

static void restore_env(const char* name, char* value)
{
    if (value) TEST_SETENV(name, value);
    else TEST_UNSETENV(name);
    free(value);
}

static void write_u16(FILE* file, unsigned value)
{
    fputc((int)(value & 255u), file);
    fputc((int)((value >> 8) & 255u), file);
}

static void write_u32(FILE* file, unsigned value)
{
    write_u16(file, value);
    write_u16(file, value >> 16);
}

static int write_original_pcm_wav(const char* path, const M11_SoundBuffer* pcm)
{
    FILE* file;
    int i;
    int count = pcm->sampleCount < 256 ? pcm->sampleCount : 256;
    int ok;
    if (count <= 0 || !(file = fopen(path, "wb"))) return 0;
    fwrite("RIFF", 1, 4, file);
    write_u32(file, 36u + (unsigned)count * 2u);
    fwrite("WAVEfmt ", 1, 8, file);
    write_u32(file, 16u);
    write_u16(file, 1u);
    write_u16(file, 1u);
    write_u32(file, M11_AUDIO_SAMPLE_RATE);
    write_u32(file, M11_AUDIO_SAMPLE_RATE * 2u);
    write_u16(file, 2u);
    write_u16(file, 16u);
    fwrite("data", 1, 4, file);
    write_u32(file, (unsigned)count * 2u);
    for (i = 0; i < count; ++i) {
        float sample = pcm->samples[i];
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        write_u16(file, (unsigned)(uint16_t)(int16_t)(sample * 32767.0f));
    }
    ok = !ferror(file);
    return fclose(file) == 0 && ok;
}

int main(void)
{
    const char* song = getenv("FIRESTAFF_SONG_DAT");
    const char* temp = getenv("TMPDIR");
    DM1_V1_F0740F0743MusicSourcePc34 source;
    M11_AudioState audio;
    char cwd[FSP_PATH_MAX], base[FSP_PATH_MAX], scratch[FSP_PATH_MAX];
    char resolved[FSP_PATH_MAX], expected[FSP_PATH_MAX], tiny[2];
    char leaf[80];
    char* savedEnable;
    char* savedDisable;
    int changed = 0;
    int created = 0;
    if (!song || !song[0]) {
        puts("SKIP: FIRESTAFF_SONG_DAT is required for original PCM path checks");
        return 77;
    }
    if (!dm1_v1_f0740_f0743_bind_song_dat_pc34(song, &source)) {
        fprintf(stderr, "FAIL: supplied SONG.DAT is not authenticated PC34 media\n");
        return 1;
    }
    if (!TEST_GETCWD(cwd, sizeof(cwd))) return 1;
    savedEnable = copy_env("FIRESTAFF_AUDIO_ENABLE_SDL");
    savedDisable = copy_env("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG");
    if ((getenv("FIRESTAFF_AUDIO_ENABLE_SDL") && !savedEnable) ||
        (getenv("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG") && !savedDisable)) {
        free(savedEnable);
        free(savedDisable);
        return 1;
    }
    TEST_SETENV("FIRESTAFF_AUDIO_ENABLE_SDL", "0");
    TEST_SETENV("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG", "1");
    memset(&audio, 0, sizeof(audio));
    CHECK(M11_Audio_Init(&audio), "initialize decoder without output hardware");
    TEST_UNSETENV("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG");
    CHECK(M11_Audio_BindOriginalSongPath(&audio, song), "decode admitted original SONG");
    if (failures) goto cleanup;
    /* Test-only PCM derivative; never used by a game runtime or committed. */
    if (!temp || !temp[0]) temp = "build";
    CHECK(FSP_CreateDirectoryRecursive(temp) &&
          FSP_ResolvePhysicalPath(base, sizeof(base), temp), "resolve test scratch parent");
    if (failures) goto cleanup;
    snprintf(leaf, sizeof(leaf), "soundtrack-original-%llu",
             (unsigned long long)SDL_GetTicksNS());
    CHECK(FSP_JoinPath(scratch, sizeof(scratch), base, leaf) &&
          !FSP_PathExists(scratch) && FSP_CreateDirectory(scratch), "create unique scratch directory");
    if (failures) goto cleanup;
    created = 1;
    CHECK(TEST_CHDIR(base) == 0, "enter scratch parent for relative resolution");
    if (failures) goto cleanup;
    changed = 1;
    CHECK(FSP_JoinPath(expected, sizeof(expected), scratch, "title.wav") &&
          write_original_pcm_wav(expected, &audio.titleMusic), "write WAV from actual decoded original PCM");
    if (failures) goto cleanup;
    CHECK(M11_Soundtrack_GetTrackPath(M11_SOUNDTRACK_MODE_CUSTOM, "title", leaf,
          resolved, sizeof(resolved)) == M11_SOUNDTRACK_RESULT_RESOLVED &&
          strcmp(resolved, expected) == 0 && FSP_FileExists(resolved),
          "relative custom folder returns the existing absolute filename");
    {
        SDL_AudioSpec spec;
        Uint8* bytes = NULL;
        Uint32 length = 0;
        CHECK(SDL_LoadWAV(expected, &spec, &bytes, &length) && length == 512u &&
              spec.freq == M11_AUDIO_SAMPLE_RATE && spec.channels == 1,
              "derived authentic PCM file is a readable WAV");
        SDL_free(bytes);
    }
    strcpy(tiny, "x");
    CHECK(M11_Soundtrack_GetTrackPath(M11_SOUNDTRACK_MODE_CUSTOM, "title", leaf,
          tiny, sizeof(tiny)) == M11_SOUNDTRACK_RESULT_FALLBACK && !tiny[0],
          "undersized track output fails closed and clears the buffer");
    strcpy(resolved, "stale");
    CHECK(M11_Soundtrack_GetTrackPath(M11_SOUNDTRACK_MODE_ORIGINAL, "title", leaf,
          resolved, sizeof(resolved)) == M11_SOUNDTRACK_RESULT_ORIGINAL && !resolved[0],
          "Original mode clears the path");
    strcpy(resolved, "stale");
    CHECK(M11_Soundtrack_GetTrackPath(M11_SOUNDTRACK_MODE_CUSTOM, "absent", leaf,
          resolved, sizeof(resolved)) == M11_SOUNDTRACK_RESULT_FALLBACK && !resolved[0],
          "missing track falls back with no stale path");
    CHECK(FSP_ResolvePhysicalPath(resolved, sizeof(resolved), ".") &&
          strcmp(resolved, base) == 0, "physical current directory resolves absolutely");
    strcpy(tiny, "x");
    CHECK(!FSP_ResolvePhysicalPath(tiny, sizeof(tiny), ".") && !tiny[0],
          "undersized physical path output fails and clears");
cleanup:
    if (created) {
        if (FSP_JoinPath(expected, sizeof(expected), scratch, "title.wav")) remove(expected);
        TEST_RMDIR(scratch);
    }
    if (changed) CHECK(TEST_CHDIR(cwd) == 0, "restore working directory");
    M11_Audio_Shutdown(&audio);
    restore_env("FIRESTAFF_AUDIO_ENABLE_SDL", savedEnable);
    restore_env("FIRESTAFF_AUDIO_DISABLE_ORIGINAL_SONG", savedDisable);
    if (!failures) puts("PASS: authentic PCM soundtrack and physical-path contracts");
    return failures ? 1 : 0;
}
