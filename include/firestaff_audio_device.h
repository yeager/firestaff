#ifndef FIRESTAFF_AUDIO_DEVICE_H
#define FIRESTAFF_AUDIO_DEVICE_H

#include <SDL3/SDL_audio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Called on the host thread. Empty/NULL selects the system default.
 * Stores a name rather than a device ID so reconnects are resolved afresh. */
void Firestaff_AudioDevice_SetPreferredName(const char* name);
/* Call after SDL audio initialization, immediately before opening a stream.
 * Missing or disconnected named devices fall back to the system default. */
SDL_AudioDeviceID Firestaff_AudioDevice_ResolvePlayback(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AUDIO_DEVICE_H */
