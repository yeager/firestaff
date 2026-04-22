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
 * Pre-generated PCM buffer for one sound effect.
 * Stored as float samples at 22050 Hz mono.
 */
#define M11_AUDIO_SAMPLE_RATE 22050
#define M11_AUDIO_MAX_SOUND_MS 300
#define M11_AUDIO_MAX_SAMPLES  ((M11_AUDIO_SAMPLE_RATE * M11_AUDIO_MAX_SOUND_MS) / 1000)

typedef struct {
    float samples[M11_AUDIO_MAX_SAMPLES];
    int   sampleCount;
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

    /* SDL3 native audio — opaque pointer to avoid exposing SDL headers */
    void* sdlStream;  /* SDL_AudioStream* */

    /* Pre-generated sounds, one per marker type */
    M11_SoundBuffer sounds[M11_AUDIO_MARKER_COUNT];
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

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AUDIO_SDL_M11_H */
