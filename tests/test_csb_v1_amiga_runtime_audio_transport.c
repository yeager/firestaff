/* ReDMCSB SOUND.C F0709 transport regression for authenticated Amiga PCM. */
#include "audio_sdl_m11.h"

#include <stdio.h>
#include <string.h>

static unsigned int fnv1a(const unsigned char* bytes, int count)
{
    unsigned int hash = 2166136261u;
    int index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int expect(int condition, const char* message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    M11_AudioState state;
    unsigned char source[] = { 0x80u, 0x00u, 0x7fu, 0xffu };
    unsigned int hash = fnv1a(source, (int)sizeof(source));
    int ok = 1;

    memset(&state, 0, sizeof(state));
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    ok &= expect(M11_Audio_PlayCsbAmigaRuntimePcmAtSourceVolume(
                     &state, source, (int)sizeof(source), 112, hash, 3),
                 "authenticated Amiga PCM accepts at source period");
    ok &= expect(state.csbAmigaRuntimeSoundAccepted &&
                     state.csbAmigaRuntimeSoundByteCount == (int)sizeof(source) &&
                     state.csbAmigaRuntimeSoundPeriod == 112 &&
                     state.csbAmigaRuntimeSoundHash == hash &&
                     state.csbAmigaRuntimePcm.sampleCount > 0,
                 "Amiga source identity and period survive transport");
    /* SOUND.C F0060: 72800/112 = 650 Paula ticks per byte sample.
     * NTSC 3579545/650 = 5506 Hz, so four bytes occupy 17 host samples,
     * not 33 (the former erroneous extra clock division by two). */
    ok &= expect(state.csbAmigaRuntimePcm.sampleCount == 17,
                 "Paula byte cadence is not halved at a DMA word boundary");
    if (state.csbAmigaRuntimePcm.sampleCount == 17) {
        ok &= expect(state.csbAmigaRuntimePcm.samples[0] == -1.0f &&
                     state.csbAmigaRuntimePcm.samples[5] == 0.0f &&
                     state.csbAmigaRuntimePcm.samples[9] == 127.0f / 128.0f &&
                     state.csbAmigaRuntimePcm.samples[16] == -1.0f / 128.0f,
                     "each signed source byte reaches its correct host time");
    }
    ok &= expect(M11_Audio_PlayCsbAmigaRuntimePcmAtSourceVolume(
                     &state, source, (int)sizeof(source), 79, hash, 1),
                 "Amiga variant-specific period is accepted");
    ok &= expect(state.csbAmigaRuntimeSoundSourceVolume == 1,
                 "Amiga transport retains source volume");
    ok &= expect(M11_Audio_PlayCsbAmigaRuntimePcmAtPaulaVolume(
                     &state, source, (int)sizeof(source), 112, hash, 64),
                 "Amiga F0709 Paula full-scale volume accepts source PCM");
    ok &= expect(state.csbAmigaRuntimeSoundSourceVolume == 64,
                 "Amiga F0709 preserves its native 0..64 volume receipt");
    for (int volume = 0; volume <= 64; ++volume) {
        ok &= expect(M11_Audio_PlayCsbAmigaRuntimePcmAtPaulaVolume(
                         &state, source, (int)sizeof(source), 112, hash, volume) &&
                     state.csbAmigaRuntimeSoundSourceVolume == volume &&
                     state.csbAmigaRuntimeSoundMixerVolume ==
                         (state.sfxVolume * volume + 32) / 64 &&
                     state.csbAmigaRuntimePcm.sampleCount == 17,
                     "native Paula gain retains every level including silence");
    }
    ok &= expect(!M11_Audio_PlayCsbAmigaRuntimePcmAtPaulaVolume(
                     &state, source, (int)sizeof(source), 112, hash, 65),
                 "Amiga F0709 rejects an out-of-domain Paula volume");
    source[0] ^= 0x80u;
    ok &= expect(!M11_Audio_PlayCsbAmigaRuntimePcmAtSourceVolume(
                     &state, source, (int)sizeof(source), 112, hash, 3),
                 "changed Amiga source is rejected without fallback");
    ok &= expect(!state.csbAmigaRuntimeSoundAccepted &&
                     state.csbAmigaRuntimePcm.sampleCount == 0,
                 "rejection clears stale Amiga source data");
    M11_Audio_Shutdown(&state);
    return ok ? 0 : 1;
}
