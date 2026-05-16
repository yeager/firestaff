#ifndef FIRESTAFF_AUDIO_SDL_M11_H
#define FIRESTAFF_AUDIO_SDL_M11_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_AUDIO_BACKEND_NONE = 0,
    M11_AUDIO_BACKEND_SDL3 = 1
} M11_AudioBackend;

typedef enum {
    M11_AUDIO_MARKER_NONE = 0,
    M11_AUDIO_MARKER_FOOTSTEP,
    M11_AUDIO_MARKER_DOOR,
    M11_AUDIO_MARKER_COMBAT,
    M11_AUDIO_MARKER_CREATURE,
    M11_AUDIO_MARKER_SPELL,
    M11_AUDIO_MARKER_COUNT
} M11_AudioMarker;

/*
 * PCM buffer for one sound effect.
 * Stored as mono float samples at the SDL stream rate (22050 Hz).
 * Procedural fallbacks and decoded V1 SND3 assets share this format.
 */
#define M11_AUDIO_SAMPLE_RATE 22050
#define M11_AUDIO_SOURCE_SND3_SAMPLE_RATE 6000
#define M11_AUDIO_SOURCE_SND8_SAMPLE_RATE 11025
#define M11_AUDIO_MAX_SOUND_MS 300
#define M11_AUDIO_MAX_SAMPLES  ((M11_AUDIO_SAMPLE_RATE * M11_AUDIO_MAX_SOUND_MS) / 1000)
#define M11_AUDIO_ORIGINAL_SOUND_COUNT 35

typedef struct {
    float* samples;
    int    sampleCount;
    int    capacity;
} M11_SoundBuffer;

typedef struct {
    int initialized;
    M11_AudioBackend backend;
    int masterVolume; /* 0..128 */
    int sfxVolume;    /* 0..128 */
    int musicVolume;  /* 0..128 */
    int uiVolume;     /* 0..128 */
    int playedMarkerCount;
    int queuedSampleCount;    /* total samples queued to device */
    M11_AudioMarker lastMarker;
    int lastSoundIndex;       /* DM PC v3.4 sound event index, or -1 */

    /* Original V1 SND3 bank: loaded from GRAPHICS.DAT when available. */
    int originalSnd3Available;
    int originalSnd3LoadedCount;

    /* Original V1 title music: loaded from SONG.DAT when available. */
    int originalSongAvailable;
    int originalSongPartCount;
    int originalSongSequenceWordCount;
    int originalSongPlayablePartCount;
    int originalSongLoopTargetPart;
    int titleMusicQueuedCount;

    /* SDL3 native audio — opaque pointer to avoid exposing SDL headers */
    void* sdlStream;  /* SDL_AudioStream* */

    /* Pre-generated procedural sounds, one per marker type. */
    M11_SoundBuffer sounds[M11_AUDIO_MARKER_COUNT];

    /* Decoded/resampled original sounds, one per DM sound event index. */
    M11_SoundBuffer originalSounds[M11_AUDIO_ORIGINAL_SOUND_COUNT];

    /* Decoded/resampled SEQ2 title-music phrase, one pre-loop cycle. */
    M11_SoundBuffer titleMusic;
} M11_AudioState;

int  M11_Audio_Init(M11_AudioState* state);
void M11_Audio_Shutdown(M11_AudioState* state);
int  M11_Audio_IsAvailable(const M11_AudioState* state);
int  M11_Audio_SetVolumes(M11_AudioState* state,
                          int masterVolume,
                          int sfxVolume,
                          int musicVolume,
                          int uiVolume);
int  M11_Audio_GetVolumes(const M11_AudioState* state,
                          int* outMaster,
                          int* outSfx,
                          int* outMusic,
                          int* outUi);
int  M11_Audio_EmitMarker(M11_AudioState* state, M11_AudioMarker marker);
int  M11_Audio_EmitSoundIndex(M11_AudioState* state, int soundIndex, M11_AudioMarker fallbackMarker);
int  M11_Audio_PlayTitleMusic(M11_AudioState* state);
int  M11_Audio_OriginalSnd3Available(const M11_AudioState* state);
int  M11_Audio_OriginalSongAvailable(const M11_AudioState* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AUDIO_SDL_M11_H */
