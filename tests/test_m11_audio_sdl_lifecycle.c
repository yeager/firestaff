#include "audio_sdl_m11.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>

#include <SDL3/SDL.h>

static int g_fail_next_open;

SDL_AudioStream *m11_test_open_audio_device_stream(
    SDL_AudioDeviceID device, const SDL_AudioSpec *spec,
    SDL_AudioStreamCallback callback, void *userdata)
{
    if (g_fail_next_open) {
        g_fail_next_open = 0;
        return NULL;
    }
    return SDL_OpenAudioDeviceStream(device, spec, callback, userdata);
}

int main(void)
{
    SDL_AudioSpec owner_spec;
    SDL_AudioStream *owner_stream;
    M11_AudioState state;

    assert(SDL_InitSubSystem(SDL_INIT_AUDIO));
    owner_spec.format = SDL_AUDIO_F32;
    owner_spec.channels = 1;
    owner_spec.freq = 22050;
    owner_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &owner_spec, NULL, NULL);
    assert(owner_stream);
    assert(SDL_ResumeAudioStreamDevice(owner_stream));

    g_fail_next_open = 1;
    assert(M11_Audio_Init(&state));
    assert(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO);
    assert(SDL_GetAudioStreamDevice(owner_stream) != 0u);
    assert(SDL_PutAudioStreamData(owner_stream, "\0\0\0\0", 4));

    M11_Audio_Shutdown(&state);
    assert(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO);
    assert(SDL_GetAudioStreamDevice(owner_stream) != 0u);
    assert(SDL_PutAudioStreamData(owner_stream, "\0\0\0\0", 4));

    SDL_DestroyAudioStream(owner_stream);
    puts("PASS: M11 audio failure preserves a shared SDL audio owner");
    return 0;
}
