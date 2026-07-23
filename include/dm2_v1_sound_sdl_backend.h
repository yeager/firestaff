#ifndef FIRESTAFF_DM2_V1_SOUND_SDL_BACKEND_H
#define FIRESTAFF_DM2_V1_SOUND_SDL_BACKEND_H

#include <stdint.h>

#include "dm2_v1_sound.h"

/* dm2_v1_sound_sdl_backend.h — DM2-008 SDL3 playback backend (cycle 16).
 *
 * Source: skproject/SKWIN/SkwinSDL.cpp — OpenAudio() opens SDL at
 * PLAYBACK_FREQUENCY 6000 Hz, sdlAudMix() mixes up to MAX_SB = 16 SndBuf
 * voices additively into the unsigned 8-bit stream.  This backend is the
 * concrete DM2_V1_SoundPlaybackBackend binding for runtime targets; headless
 * verification binds it with SDL_AUDIODRIVER=dummy.
 *
 * The backend never fabricates audio: start_voice only accepts PCM buffers
 * decoded by dm2_v1_sound_decode_gdat_pcm() from a verified GDAT entry. */

#ifdef __cplusplus
extern "C" {
#endif

/* Fill *out_backend with the SDL3 callback table (static storage, no ctx). */
void dm2_v1_sound_sdl_backend_describe(DM2_V1_SoundPlaybackBackend *out_backend);

/* Observability for probes/tests. */
int dm2_v1_sound_sdl_backend_is_ready(void);
uint64_t dm2_v1_sound_sdl_backend_mixed_frames(void);
uint32_t dm2_v1_sound_sdl_backend_started_voice_count(void);
void dm2_v1_sound_sdl_backend_close(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SOUND_SDL_BACKEND_H */
