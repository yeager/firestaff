/* dm2_v1_sound_sdl_backend.c — DM2-008 SDL3 playback backend (cycle 16)
 *
 * Source: skproject/SKWIN/SkwinSDL.cpp
 *   OpenAudio()  — SDL audio at PLAYBACK_FREQUENCY = 6000 Hz
 *   MAX_SB = 16  — simultaneous SndBuf voices
 *   sdlAudMix()  — additive mix of active voices into the unsigned 8-bit
 *                  stream (samples converted 0x80 + raw_byte at alloc time)
 *
 * The mixing contract is source-shaped: voices hold unsigned 8-bit mono PCM
 * at 6000 Hz, each active voice contributes (sample - 128) * volume / 255,
 * and the mix is clamped back into the unsigned 8-bit stream.  SDL3 converts
 * the 6000 Hz U8 stream to the device format.  Nothing is synthesized: the
 * only PCM ever queued is decoded from verified GDAT sound entries by
 * dm2_v1_sound.c. */

#include "dm2_v1_sound_sdl_backend.h"

#include <string.h>

#include <SDL3/SDL.h>

typedef struct {
    const uint8_t *pcm;
    uint32_t length;
    uint32_t position;
    uint8_t volume;
    uint8_t active;
} DM2_V1_SdlSoundVoice;

static SDL_AudioStream *g_dm2_sdl_stream;
static DM2_V1_SdlSoundVoice g_dm2_sdl_voices[DM2_V1_SOUND_VOICE_MAX];
static uint64_t g_dm2_sdl_mixed_frames;
static uint32_t g_dm2_sdl_started_voices;
static int g_dm2_sdl_ready;

/* sdlAudMix-shaped mixer: additive per-voice contribution, clamped. */
static void dm2_v1_sdl_mix(uint8_t *out, int frames)
{
    int f;
    unsigned v;
    memset(out, 0x80, (size_t)frames);
    for (f = 0; f < frames; ++f) {
        int acc = 0;
        for (v = 0; v < DM2_V1_SOUND_VOICE_MAX; ++v) {
            DM2_V1_SdlSoundVoice *voice = &g_dm2_sdl_voices[v];
            int sample;
            if (!voice->active || voice->position >= voice->length)
                continue;
            sample = (int)voice->pcm[voice->position] - 128;
            acc += (sample * (int)voice->volume) / 255;
            voice->position++;
            if (voice->position >= voice->length)
                voice->active = 0;
        }
        if (acc > 127) acc = 127;
        if (acc < -128) acc = -128;
        out[f] = (uint8_t)(0x80 + acc);
    }
    g_dm2_sdl_mixed_frames += (uint64_t)frames;
}

static void SDLCALL dm2_v1_sdl_stream_callback(void *userdata,
                                               SDL_AudioStream *stream,
                                               int additional_amount,
                                               int total_amount)
{
    uint8_t chunk[512];
    (void)userdata;
    (void)total_amount;
    while (additional_amount > 0) {
        int frames = additional_amount;
        if (frames > (int)sizeof(chunk))
            frames = (int)sizeof(chunk);
        dm2_v1_sdl_mix(chunk, frames);
        if (!SDL_PutAudioStreamData(stream, chunk, frames))
            return;
        additional_amount -= frames;
    }
}

static int dm2_v1_sdl_backend_open(void *ctx)
{
    SDL_AudioSpec spec;
    (void)ctx;
    if (g_dm2_sdl_ready)
        return 1;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
        return 0;
    memset(g_dm2_sdl_voices, 0, sizeof(g_dm2_sdl_voices));
    spec.format = SDL_AUDIO_U8;
    spec.channels = 1;
    spec.freq = (int)DM2_V1_SOUND_PCM_SAMPLE_RATE_HZ;
    g_dm2_sdl_stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
        dm2_v1_sdl_stream_callback, NULL);
    if (!g_dm2_sdl_stream) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return 0;
    }
    if (!SDL_ResumeAudioStreamDevice(g_dm2_sdl_stream)) {
        SDL_DestroyAudioStream(g_dm2_sdl_stream);
        g_dm2_sdl_stream = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return 0;
    }
    g_dm2_sdl_ready = 1;
    return 1;
}

static int dm2_v1_sdl_backend_is_ready(void *ctx)
{
    (void)ctx;
    return g_dm2_sdl_ready && g_dm2_sdl_stream != NULL;
}

static int dm2_v1_sdl_backend_start_voice(void *ctx, unsigned voice_slot,
                                          const uint8_t *pcm,
                                          uint32_t sample_count,
                                          uint8_t volume)
{
    int started = 0;
    (void)ctx;
    if (!g_dm2_sdl_ready || !g_dm2_sdl_stream || !pcm ||
        sample_count == 0u || voice_slot >= DM2_V1_SOUND_VOICE_MAX)
        return 0;
    if (!SDL_LockAudioStream(g_dm2_sdl_stream))
        return 0;
    if (!g_dm2_sdl_voices[voice_slot].active) {
        g_dm2_sdl_voices[voice_slot].pcm = pcm;
        g_dm2_sdl_voices[voice_slot].length = sample_count;
        g_dm2_sdl_voices[voice_slot].position = 0u;
        g_dm2_sdl_voices[voice_slot].volume = volume;
        g_dm2_sdl_voices[voice_slot].active = 1u;
        g_dm2_sdl_started_voices++;
        started = 1;
    }
    SDL_UnlockAudioStream(g_dm2_sdl_stream);
    return started;
}

static int dm2_v1_sdl_backend_voice_active(void *ctx, unsigned voice_slot)
{
    int active = 0;
    (void)ctx;
    if (!g_dm2_sdl_stream || voice_slot >= DM2_V1_SOUND_VOICE_MAX)
        return 0;
    if (!SDL_LockAudioStream(g_dm2_sdl_stream))
        return 0;
    active = g_dm2_sdl_voices[voice_slot].active ? 1 : 0;
    SDL_UnlockAudioStream(g_dm2_sdl_stream);
    return active;
}

static void dm2_v1_sdl_backend_stop_all(void *ctx)
{
    (void)ctx;
    if (!g_dm2_sdl_stream)
        return;
    if (SDL_LockAudioStream(g_dm2_sdl_stream)) {
        memset(g_dm2_sdl_voices, 0, sizeof(g_dm2_sdl_voices));
        SDL_UnlockAudioStream(g_dm2_sdl_stream);
    }
}

static void dm2_v1_sdl_backend_close(void *ctx)
{
    (void)ctx;
    if (g_dm2_sdl_stream) {
        SDL_DestroyAudioStream(g_dm2_sdl_stream);
        g_dm2_sdl_stream = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
    memset(g_dm2_sdl_voices, 0, sizeof(g_dm2_sdl_voices));
    g_dm2_sdl_ready = 0;
}

void dm2_v1_sound_sdl_backend_describe(DM2_V1_SoundPlaybackBackend *out_backend)
{
    if (!out_backend)
        return;
    out_backend->ctx = NULL;
    out_backend->open = dm2_v1_sdl_backend_open;
    out_backend->is_ready = dm2_v1_sdl_backend_is_ready;
    out_backend->start_voice = dm2_v1_sdl_backend_start_voice;
    out_backend->voice_active = dm2_v1_sdl_backend_voice_active;
    out_backend->stop_all = dm2_v1_sdl_backend_stop_all;
    out_backend->close = dm2_v1_sdl_backend_close;
}

int dm2_v1_sound_sdl_backend_is_ready(void)
{
    return g_dm2_sdl_ready && g_dm2_sdl_stream != NULL;
}

uint64_t dm2_v1_sound_sdl_backend_mixed_frames(void)
{
    return g_dm2_sdl_mixed_frames;
}

uint32_t dm2_v1_sound_sdl_backend_started_voice_count(void)
{
    return g_dm2_sdl_started_voices;
}

void dm2_v1_sound_sdl_backend_close(void)
{
    dm2_v1_sdl_backend_close(NULL);
}
