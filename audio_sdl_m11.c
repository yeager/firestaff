#include "audio_sdl_m11.h"

#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define M11_AUDIO_VOLUME_MIN 0
#define M11_AUDIO_VOLUME_MAX 128

/*
 * Guard: SDL3 headers are only included when we actually attempt real audio.
 * The build always links SDL3 via CMake, so this is safe.
 */
#ifdef FIRESTAFF_NO_SDL_AUDIO
#define M11_HAVE_SDL_AUDIO 0
#else
#include <SDL3/SDL.h>
#define M11_HAVE_SDL_AUDIO 1
#endif

/* ── helpers ─────────────────────────────────────────────────────── */

static int m11_clamp_volume(int value) {
    if (value < M11_AUDIO_VOLUME_MIN) return M11_AUDIO_VOLUME_MIN;
    if (value > M11_AUDIO_VOLUME_MAX) return M11_AUDIO_VOLUME_MAX;
    return value;
}

/*
 * Generate a sine-wave tone with exponential decay.
 *   freq_hz   — base frequency
 *   dur_ms    — duration in milliseconds
 *   amplitude — peak amplitude (0.0 .. 1.0)
 *   decay     — exponential decay rate (higher = faster fade)
 */
static void m11_gen_tone(M11_SoundBuffer* buf,
                         float freq_hz,
                         int dur_ms,
                         float amplitude,
                         float decay) {
    int count;
    int i;
    if (!buf) return;
    count = (M11_AUDIO_SAMPLE_RATE * dur_ms) / 1000;
    if (count > M11_AUDIO_MAX_SAMPLES) count = M11_AUDIO_MAX_SAMPLES;
    buf->sampleCount = count;
    for (i = 0; i < count; ++i) {
        float t = (float)i / (float)M11_AUDIO_SAMPLE_RATE;
        float env = amplitude * expf(-decay * t);
        buf->samples[i] = env * sinf(2.0f * (float)M_PI * freq_hz * t);
    }
}

/*
 * Generate a noise burst with exponential decay (for percussive sounds).
 */
static void m11_gen_noise(M11_SoundBuffer* buf,
                          int dur_ms,
                          float amplitude,
                          float decay) {
    int count;
    int i;
    unsigned int seed = 0xDEAD;
    if (!buf) return;
    count = (M11_AUDIO_SAMPLE_RATE * dur_ms) / 1000;
    if (count > M11_AUDIO_MAX_SAMPLES) count = M11_AUDIO_MAX_SAMPLES;
    buf->sampleCount = count;
    for (i = 0; i < count; ++i) {
        float t = (float)i / (float)M11_AUDIO_SAMPLE_RATE;
        float env = amplitude * expf(-decay * t);
        /* Simple LCG pseudo-random */
        seed = seed * 1103515245u + 12345u;
        float noise = ((float)(int)(seed >> 16) / 32768.0f) - 1.0f;
        buf->samples[i] = env * noise;
    }
}

/*
 * Mix: add src into dst (additive blend, clamped later by SDL).
 * If dst is shorter, extends it.
 */
static void m11_mix_into(M11_SoundBuffer* dst, const M11_SoundBuffer* src) {
    int i;
    if (!dst || !src) return;
    if (src->sampleCount > dst->sampleCount)
        dst->sampleCount = src->sampleCount;
    for (i = 0; i < src->sampleCount; ++i) {
        dst->samples[i] += src->samples[i];
    }
}

/*
 * Pre-generate all marker sounds.  These are short procedural effects
 * designed to be recognizable placeholders for original DM1 sounds.
 */
static void m11_generate_sounds(M11_AudioState* state) {
    M11_SoundBuffer tmp;

    /* FOOTSTEP: low thud — 80 Hz sine + noise, 120 ms, fast decay */
    memset(&state->sounds[M11_AUDIO_MARKER_FOOTSTEP], 0, sizeof(M11_SoundBuffer));
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_FOOTSTEP], 80.0f, 120, 0.5f, 12.0f);
    memset(&tmp, 0, sizeof(tmp));
    m11_gen_noise(&tmp, 60, 0.25f, 20.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_FOOTSTEP], &tmp);

    /* DOOR: creak — 200 Hz + 150 Hz beating, 200 ms */
    memset(&state->sounds[M11_AUDIO_MARKER_DOOR], 0, sizeof(M11_SoundBuffer));
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_DOOR], 200.0f, 200, 0.4f, 6.0f);
    memset(&tmp, 0, sizeof(tmp));
    m11_gen_tone(&tmp, 150.0f, 200, 0.3f, 6.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_DOOR], &tmp);

    /* COMBAT: sharp metallic hit — 400 Hz + noise, 80 ms, fast decay */
    memset(&state->sounds[M11_AUDIO_MARKER_COMBAT], 0, sizeof(M11_SoundBuffer));
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_COMBAT], 400.0f, 80, 0.6f, 18.0f);
    memset(&tmp, 0, sizeof(tmp));
    m11_gen_noise(&tmp, 40, 0.4f, 25.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_COMBAT], &tmp);

    /* CREATURE: low growl — 100 Hz + modulated noise, 150 ms */
    memset(&state->sounds[M11_AUDIO_MARKER_CREATURE], 0, sizeof(M11_SoundBuffer));
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_CREATURE], 100.0f, 150, 0.5f, 8.0f);
    memset(&tmp, 0, sizeof(tmp));
    m11_gen_noise(&tmp, 100, 0.3f, 10.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_CREATURE], &tmp);

    /* SPELL: shimmer — 800 Hz high sine, 250 ms, gentle decay */
    memset(&state->sounds[M11_AUDIO_MARKER_SPELL], 0, sizeof(M11_SoundBuffer));
    m11_gen_tone(&state->sounds[M11_AUDIO_MARKER_SPELL], 800.0f, 250, 0.35f, 4.0f);
    memset(&tmp, 0, sizeof(tmp));
    m11_gen_tone(&tmp, 1200.0f, 200, 0.2f, 5.0f);
    m11_mix_into(&state->sounds[M11_AUDIO_MARKER_SPELL], &tmp);
}

/* ── public API ──────────────────────────────────────────────────── */

int M11_Audio_Init(M11_AudioState* state) {
    if (!state) return 0;
    memset(state, 0, sizeof(*state));
    state->initialized = 1;
    state->masterVolume = M11_AUDIO_VOLUME_MAX;
    state->sfxVolume    = M11_AUDIO_VOLUME_MAX;
    state->musicVolume  = M11_AUDIO_VOLUME_MAX;
    state->uiVolume     = M11_AUDIO_VOLUME_MAX;
    state->lastMarker   = M11_AUDIO_MARKER_NONE;
    state->sdlStream    = NULL;

    /* Pre-generate procedural sounds regardless of backend */
    m11_generate_sounds(state);

#if M11_HAVE_SDL_AUDIO
    {
        SDL_AudioSpec spec;
        SDL_AudioStream* stream;

        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            /* Audio init failed — stay in fallback mode */
            state->backend = M11_AUDIO_BACKEND_NONE;
            return 1;
        }

        spec.format   = SDL_AUDIO_F32;
        spec.channels = 1;
        spec.freq     = M11_AUDIO_SAMPLE_RATE;

        stream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            &spec,
            NULL,  /* no callback — we push data */
            NULL
        );

        if (!stream) {
            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            state->backend = M11_AUDIO_BACKEND_NONE;
            return 1;
        }

        SDL_ResumeAudioStreamDevice(stream);
        state->sdlStream = stream;
        state->backend   = M11_AUDIO_BACKEND_SDL3;
    }
#else
    state->backend = M11_AUDIO_BACKEND_NONE;
#endif

    return 1;
}

void M11_Audio_Shutdown(M11_AudioState* state) {
    if (!state) return;

#if M11_HAVE_SDL_AUDIO
    if (state->sdlStream) {
        SDL_DestroyAudioStream((SDL_AudioStream*)state->sdlStream);
        state->sdlStream = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
#endif

    state->initialized = 0;
    state->backend = M11_AUDIO_BACKEND_NONE;
}

int M11_Audio_IsAvailable(const M11_AudioState* state) {
    if (!state || !state->initialized) return 0;
    return state->backend == M11_AUDIO_BACKEND_SDL3 ? 1 : 0;
}

int M11_Audio_SetVolumes(M11_AudioState* state,
                         int masterVolume,
                         int sfxVolume,
                         int musicVolume,
                         int uiVolume) {
    if (!state || !state->initialized) return 0;
    state->masterVolume = m11_clamp_volume(masterVolume);
    state->sfxVolume    = m11_clamp_volume(sfxVolume);
    state->musicVolume  = m11_clamp_volume(musicVolume);
    state->uiVolume     = m11_clamp_volume(uiVolume);

#if M11_HAVE_SDL_AUDIO
    if (state->sdlStream) {
        float gain = (float)state->masterVolume / (float)M11_AUDIO_VOLUME_MAX;
        float sfxGain = (float)state->sfxVolume / (float)M11_AUDIO_VOLUME_MAX;
        SDL_SetAudioStreamGain((SDL_AudioStream*)state->sdlStream,
                               gain * sfxGain);
    }
#endif

    return 1;
}

int M11_Audio_GetVolumes(const M11_AudioState* state,
                         int* outMaster,
                         int* outSfx,
                         int* outMusic,
                         int* outUi) {
    if (!state || !state->initialized ||
        !outMaster || !outSfx || !outMusic || !outUi) {
        return 0;
    }
    *outMaster = state->masterVolume;
    *outSfx    = state->sfxVolume;
    *outMusic  = state->musicVolume;
    *outUi     = state->uiVolume;
    return 1;
}

int M11_Audio_EmitMarker(M11_AudioState* state, M11_AudioMarker marker) {
    if (!state || !state->initialized) return 0;
    if (marker <= M11_AUDIO_MARKER_NONE || marker >= M11_AUDIO_MARKER_COUNT) return 0;

    state->lastMarker = marker;

    if (state->backend != M11_AUDIO_BACKEND_SDL3) {
        /* Fallback: record marker but don't play */
        return 0;
    }

#if M11_HAVE_SDL_AUDIO
    {
        const M11_SoundBuffer* snd = &state->sounds[marker];
        if (snd->sampleCount > 0 && state->sdlStream) {
            int byteLen = snd->sampleCount * (int)sizeof(float);
            SDL_PutAudioStreamData(
                (SDL_AudioStream*)state->sdlStream,
                snd->samples,
                byteLen
            );
            state->queuedSampleCount += snd->sampleCount;
        }
    }
#endif

    state->playedMarkerCount += 1;
    return 1;
}
