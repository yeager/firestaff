/* Atari ST ANIM.C opcode 12 -> SOUND.C F0060 host-audio transport. */
#include "audio_sdl_m11.h"
#include "csb_v1_atari_st_animation_assets.h"

#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int fnv1a(const unsigned char *bytes, int count)
{
    unsigned int hash = 2166136261u;
    int index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    /* Count 7, source F0060 high-nibble and repeat-run coding. */
    unsigned char snd1[] = {0x00, 0x07, 0x50, 0x19, 0x00};
    /* Two source samples held at amplitude index zero. */
    unsigned char silentSnd1[] = {0x00, 0x02, 0x00, 0x00};
    M11_AudioState state;
    unsigned int hash = fnv1a(snd1, (int)sizeof(snd1));
    unsigned int silentHash = fnv1a(silentSnd1, (int)sizeof(silentSnd1));
    int sourceVolume;
    int sample;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    ok &= expect(M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 112, hash),
                 "source-owned Atari SND1 accepts at its Timer-A period");
    ok &= expect(state.csbAtariStSoundAccepted &&
                     state.csbAtariStSoundPeriod == 112 &&
                     state.csbAtariStSoundSourceVolume == 1 &&
                     state.csbAtariStSoundHash == hash &&
                     state.csbAtariStPsg.sampleCount > 0,
                 "decoded PSG stream retains source identity and output");
    ok &= expect(state.csbAtariStPsg.samples[0] != 0.0f,
                 "nonzero source-format PSG amplitude remains audible");
    ok &= expect(M11_Audio_PlayCsbAtariStPsgAtSourceVolume(
                     &state, snd1, (int)sizeof(snd1), 112, hash, 0) &&
                     state.csbAtariStSoundSourceVolume == 0,
                 "soft-distance SND1 selects the original soft PSG table");
    for (sourceVolume = 0; sourceVolume <= 1; ++sourceVolume) {
        ok &= expect(M11_Audio_PlayCsbAtariStPsgAtSourceVolume(
                         &state, silentSnd1, (int)sizeof(silentSnd1), 112,
                         silentHash, sourceVolume),
                     sourceVolume
                         ? "zero-level Atari SND1 accepts loud source mode"
                         : "zero-level Atari SND1 accepts soft source mode");
        ok &= expect(state.csbAtariStPsg.sampleCount > 0,
                     sourceVolume
                         ? "loud zero-level Atari SND1 retains its output span"
                         : "soft zero-level Atari SND1 retains its output span");
        for (sample = 0; sample < state.csbAtariStPsg.sampleCount; ++sample) {
            if (state.csbAtariStPsg.samples[sample] != 0.0f) {
                ok &= expect(0, "three zero PSG registers render as silence");
                break;
            }
        }
    }
    snd1[2] ^= 0x80u;
    ok &= expect(!M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 112, hash),
                 "changed SND1 stream is rejected rather than substituted");
    ok &= expect(!state.csbAtariStSoundAccepted &&
                     state.csbAtariStPsg.sampleCount == 0 &&
                     state.csbAtariStSoundHash == 0 &&
                     state.csbAtariStSoundPeriod == 0,
                 "rejected Atari sound clears earlier successful provenance");
    snd1[2] ^= 0x80u;
    ok &= expect(M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 112, hash),
                 "valid sound recovers after rejection");
    snd1[0] = 0xffu;
    snd1[1] = 0xffu;
    ok &= expect(!M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 112, fnv1a(snd1, (int)sizeof(snd1))) &&
                     !state.csbAtariStSoundAccepted && state.csbAtariStPsg.sampleCount == 0,
                 "truncated SND1 decoding cannot inherit an accepted receipt");
    ok &= expect(!M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 10, fnv1a(snd1, (int)sizeof(snd1))),
                 "invalid Timer-A period is rejected");
    M11_Audio_Shutdown(&state);
    {
        const char *root = getenv("FIRESTAFF_CSB_ANIMATE_ROOT");
        const char *cache = getenv("FIRESTAFF_CSB_ANIMATE_CACHE");
        if (root && root[0] && cache && cache[0]) {
            FILE *media = fopen(root, "rb");
            unsigned char first[4096], second[4096];
            size_t first_bytes = 0u, second_bytes = 0u;
            uint16_t first_period = 0u, second_period = 0u;
            uint32_t first_vbl = 0u, second_vbl = 0u;
            CSB_V1_AtariStAnimationTraceReceipt trace;
            M11_AudioState real_state;
            int first_queued = 0, second_queued = 0;
            unsigned int first_samples;

            if (!media) {
                fprintf(stderr, "SKIP: authentic Atari title media unavailable\n");
                return 77;
            }
            fclose(media);
            memset(&trace, 0, sizeof(trace));
            memset(&real_state, 0, sizeof(real_state));
            ok &= expect(
                csb_v1_atari_st_animation_copy_played_sound_from_root(
                    root, cache, 0u, first, sizeof(first), &first_bytes,
                    &first_period, &first_vbl, &trace) &&
                csb_v1_atari_st_animation_copy_played_sound_from_root(
                    root, cache, 1u, second, sizeof(second), &second_bytes,
                    &second_period, &second_vbl, &trace),
                "authentic Atari title exposes both SND1 cues");
            first_samples = first_bytes >= 2u
                ? ((unsigned int)first[0] << 8) | first[1] : 0u;
            ok &= expect(first_period == 112u && second_period == 112u &&
                         first_vbl == 1107u && second_vbl == 1128u &&
                         first_samples == 3103u &&
                         first_samples * 50u >
                             (second_vbl - first_vbl) *
                             (2457600u / (4u * first_period)),
                         "authentic second title cue interrupts the first");
            ok &= expect(M11_Audio_Init(&real_state) &&
                         real_state.backend == M11_AUDIO_BACKEND_SDL3 &&
                         real_state.sdlStream &&
                         SDL_PauseAudioStreamDevice(
                             (SDL_AudioStream*)real_state.sdlStream),
                         "dummy SDL stream retains queued title PCM for inspection");
            if (real_state.sdlStream && first_bytes > 2u && second_bytes > 2u) {
                ok &= expect(M11_Audio_PlayCsbAtariStPsg(
                                 &real_state, first, (int)first_bytes,
                                 first_period, fnv1a(first, (int)first_bytes)),
                             "first authentic Atari title cue queues");
                first_queued = SDL_GetAudioStreamQueued(
                    (SDL_AudioStream*)real_state.sdlStream);
                ok &= expect(first_queued > 0,
                             "first authentic cue has queued PCM");
                ok &= expect(M11_Audio_PlayCsbAtariStPsg(
                                 &real_state, second, (int)second_bytes,
                                 second_period, fnv1a(second, (int)second_bytes)),
                             "second authentic Atari title cue queues");
                second_queued = SDL_GetAudioStreamQueued(
                    (SDL_AudioStream*)real_state.sdlStream);
                ok &= expect(second_queued ==
                                 real_state.csbAtariStPsg.sampleCount *
                                     (int)sizeof(float),
                             "new Atari Timer-A cue replaces the old SDL PSG tail");
            }
            M11_Audio_Shutdown(&real_state);
        }
    }
    return ok ? 0 : 1;
}
