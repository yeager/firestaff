
#include "firestaff_audio.h"
#include <stdio.h>

/* Compile-time SDL_mixer detection */
#ifdef HAVE_SDL_MIXER
#include <SDL2/SDL_mixer.h>
static Mix_Music *g_current_music = NULL;
static int g_audio_ready = 0;

int fs_audio_init(void) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        printf("Audio: SDL_mixer init failed: %s\n", Mix_GetError());
        return -1;
    }
    g_audio_ready = 1;
    return 0;
}

void fs_audio_shutdown(void) {
    if (g_current_music) { Mix_FreeMusic(g_current_music); g_current_music = NULL; }
    if (g_audio_ready) Mix_CloseAudio();
    g_audio_ready = 0;
}

int fs_audio_play_track(const char *path) {
    if (!g_audio_ready || !path) return -1;
    if (g_current_music) Mix_FreeMusic(g_current_music);
    g_current_music = Mix_LoadMUS(path);
    if (!g_current_music) return -1;
    return Mix_PlayMusic(g_current_music, -1);
}

void fs_audio_stop(void) {
    if (g_audio_ready) Mix_HaltMusic();
}

void fs_audio_set_volume(int percent) {
    if (g_audio_ready) Mix_VolumeMusic(percent * MIX_MAX_VOLUME / 100);
}

int fs_audio_is_playing(void) {
    return g_audio_ready ? Mix_PlayingMusic() : 0;
}

int fs_audio_play_sfx(const char *path) {
    Mix_Chunk *chunk;
    if (!g_audio_ready || !path) return -1;
    chunk = Mix_LoadWAV(path);
    if (!chunk) return -1;
    Mix_PlayChannel(-1, chunk, 0);
    return 0;
}

void fs_audio_sfx_set_volume(int percent) {
    if (g_audio_ready) Mix_Volume(-1, percent * MIX_MAX_VOLUME / 100);
}

#else
/* Stubs when SDL_mixer is not available */
int fs_audio_init(void) { printf("Audio: no SDL_mixer, audio disabled\n"); return 0; }
void fs_audio_shutdown(void) {}
int fs_audio_play_track(const char *path) { (void)path; return -1; }
void fs_audio_stop(void) {}
void fs_audio_set_volume(int percent) { (void)percent; }
int fs_audio_is_playing(void) { return 0; }
int fs_audio_play_sfx(const char *path) { (void)path; return -1; }
void fs_audio_sfx_set_volume(int percent) { (void)percent; }
#endif

