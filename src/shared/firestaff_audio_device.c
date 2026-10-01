#include "firestaff_audio_device.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

/* Matches the launcher's persisted audio device name capacity. */
#define FIRESTAFF_AUDIO_DEVICE_NAME_CAPACITY 128
static char g_preferred_name[FIRESTAFF_AUDIO_DEVICE_NAME_CAPACITY];

void Firestaff_AudioDevice_SetPreferredName(const char* name)
{
    snprintf(g_preferred_name, sizeof(g_preferred_name), "%s", name ? name : "");
}

int Firestaff_AudioDevice_PreparePlayback(void)
{
#ifdef __APPLE__
    /* The playback category avoids CoreAudio's ambient head-tracking route.
     * Set it before the first audio init; the event pump then runs the HAL
     * proxy initializer on the host thread before a device thread starts. */
    SDL_SetHint(SDL_HINT_AUDIO_CATEGORY, "playback");
#endif
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) &&
        !SDL_InitSubSystem(SDL_INIT_AUDIO))
        return 0;
#ifdef __APPLE__
    SDL_PumpEvents();
#endif
    return 1;
}

SDL_AudioDeviceID Firestaff_AudioDevice_ResolvePlayback(void)
{
    SDL_AudioDeviceID* devices;
    SDL_AudioDeviceID result = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
    int count = 0;
    int index;
    if (!g_preferred_name[0]) return result;
    devices = SDL_GetAudioPlaybackDevices(&count);
    if (!devices) return result;
    for (index = 0; index < count; ++index) {
        const char* name = SDL_GetAudioDeviceName(devices[index]);
        if (name && strcmp(name, g_preferred_name) == 0) {
            result = devices[index];
            break;
        }
    }
    SDL_free(devices);
    return result;
}
