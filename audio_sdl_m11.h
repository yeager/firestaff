#ifndef FIRESTAFF_AUDIO_SDL_M11_H
#define FIRESTAFF_AUDIO_SDL_M11_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_AUDIO_BACKEND_NONE = 0,
    M11_AUDIO_BACKEND_SDL3_MIXER = 1
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

typedef struct {
    int initialized;
    M11_AudioBackend backend;
    int masterVolume; /* 0..128 */
    int sfxVolume;    /* 0..128 */
    int musicVolume;  /* 0..128 */
    int uiVolume;     /* 0..128 */
    int playedMarkerCount;
    M11_AudioMarker lastMarker;
} M11_AudioState;

int M11_Audio_Init(M11_AudioState* state);
void M11_Audio_Shutdown(M11_AudioState* state);
int M11_Audio_IsAvailable(const M11_AudioState* state);
int M11_Audio_SetVolumes(M11_AudioState* state,
                         int masterVolume,
                         int sfxVolume,
                         int musicVolume,
                         int uiVolume);
int M11_Audio_GetVolumes(const M11_AudioState* state,
                         int* outMaster,
                         int* outSfx,
                         int* outMusic,
                         int* outUi);
int M11_Audio_EmitMarker(M11_AudioState* state, M11_AudioMarker marker);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AUDIO_SDL_M11_H */
