#include "audio_sdl_m11.h"

#include <string.h>

#define M11_AUDIO_VOLUME_MIN 0
#define M11_AUDIO_VOLUME_MAX 128

static int m11_clamp_volume(int value) {
    if (value < M11_AUDIO_VOLUME_MIN) {
        return M11_AUDIO_VOLUME_MIN;
    }
    if (value > M11_AUDIO_VOLUME_MAX) {
        return M11_AUDIO_VOLUME_MAX;
    }
    return value;
}

int M11_Audio_Init(M11_AudioState* state) {
    if (!state) {
        return 0;
    }
    memset(state, 0, sizeof(*state));
    state->initialized = 1;

    /*
     * Safe M13 Slice 1A scaffold:
     * We intentionally run fallback-only until SDL3_mixer wiring lands.
     */
    state->backend = M11_AUDIO_BACKEND_NONE;
    state->masterVolume = M11_AUDIO_VOLUME_MAX;
    state->sfxVolume = M11_AUDIO_VOLUME_MAX;
    state->musicVolume = M11_AUDIO_VOLUME_MAX;
    state->uiVolume = M11_AUDIO_VOLUME_MAX;
    state->lastMarker = M11_AUDIO_MARKER_NONE;
    return 1;
}

void M11_Audio_Shutdown(M11_AudioState* state) {
    if (!state) {
        return;
    }
    state->initialized = 0;
    state->backend = M11_AUDIO_BACKEND_NONE;
}

int M11_Audio_IsAvailable(const M11_AudioState* state) {
    if (!state || !state->initialized) {
        return 0;
    }
    return state->backend == M11_AUDIO_BACKEND_SDL3_MIXER ? 1 : 0;
}

int M11_Audio_SetVolumes(M11_AudioState* state,
                         int masterVolume,
                         int sfxVolume,
                         int musicVolume,
                         int uiVolume) {
    if (!state || !state->initialized) {
        return 0;
    }
    state->masterVolume = m11_clamp_volume(masterVolume);
    state->sfxVolume = m11_clamp_volume(sfxVolume);
    state->musicVolume = m11_clamp_volume(musicVolume);
    state->uiVolume = m11_clamp_volume(uiVolume);
    return 1;
}

int M11_Audio_GetVolumes(const M11_AudioState* state,
                         int* outMaster,
                         int* outSfx,
                         int* outMusic,
                         int* outUi) {
    if (!state || !state->initialized || !outMaster || !outSfx || !outMusic || !outUi) {
        return 0;
    }
    *outMaster = state->masterVolume;
    *outSfx = state->sfxVolume;
    *outMusic = state->musicVolume;
    *outUi = state->uiVolume;
    return 1;
}

int M11_Audio_EmitMarker(M11_AudioState* state, M11_AudioMarker marker) {
    if (!state || !state->initialized) {
        return 0;
    }
    if (marker <= M11_AUDIO_MARKER_NONE || marker >= M11_AUDIO_MARKER_COUNT) {
        return 0;
    }
    state->lastMarker = marker;

    /*
     * Fallback mode records markers for verification but does not play audio.
     * Returning 0 means "accepted but not played by backend".
     */
    if (state->backend != M11_AUDIO_BACKEND_SDL3_MIXER) {
        return 0;
    }

    state->playedMarkerCount += 1;
    return 1;
}
