/* Atari ST ANIM.C opcode 12 -> SOUND.C F0060 host-audio transport. */
#include "audio_sdl_m11.h"

#include <stdio.h>
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
    return ok ? 0 : 1;
}
