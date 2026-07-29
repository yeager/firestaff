/* ReDMCSB PC3.4 IBMIO F8119 transport: accept only authenticated raw sample
 * bytes and preserve the source PIT divisor. This checks the host transport,
 * not a replacement sound asset. */
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
    M11_AudioState state;
    unsigned char source[] = { 0x00u, 0x07u, 0xfau, 0x0fu, 0xf1u, 0x1au };
    unsigned int hash = fnv1a(source, (int)sizeof(source));
    int ok = 1;

    memset(&state, 0, sizeof(state));
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    ok &= expect(M11_Audio_PlayCsbPc34RuntimePcm(
                     &state, source, (int)sizeof(source), 112, hash),
                 "authenticated PC3.4 source bytes are accepted");
    ok &= expect(state.csbPc34RuntimeSoundAccepted &&
                     state.csbPc34RuntimeSoundByteCount == (int)sizeof(source) &&
                     state.csbPc34RuntimeSoundTimerDivisor == 112 &&
                     state.csbPc34RuntimeSoundHash == hash &&
                     state.csbPc34RuntimePcm.sampleCount > 0,
                 "source identity and PIT divisor survive the transport");
    ok &= expect(state.csbPc34RuntimeSoundSourceVolume == 3,
                 "legacy transport defaults to full PC3.4 source volume");

    ok &= expect(M11_Audio_PlayCsbPc34RuntimePcmAtSourceVolume(
                     &state, source, (int)sizeof(source), 112, hash, 1),
                 "quiet PC3.4 source volume is accepted");
    ok &= expect(state.csbPc34RuntimeSoundSourceVolume == 1,
                 "transport retains source distance volume");
    ok &= expect(!M11_Audio_PlayCsbPc34RuntimePcmAtSourceVolume(
                     &state, source, (int)sizeof(source), 112, hash, 4),
                 "out-of-domain PC3.4 source volume is rejected");

    source[0] ^= 0x80u;
    ok &= expect(!M11_Audio_PlayCsbPc34RuntimePcm(
                     &state, source, (int)sizeof(source), 112, hash),
                 "changed source is rejected without marker fallback");
    ok &= expect(!state.csbPc34RuntimeSoundAccepted &&
                     state.csbPc34RuntimePcm.sampleCount == 0,
                 "rejection clears stale PC3.4 sound data");
    ok &= expect(!M11_Audio_PlayCsbPc34RuntimePcm(
                     &state, source, (int)sizeof(source), 0,
                     fnv1a(source, (int)sizeof(source))),
                 "zero PIT divisor is rejected");
    ok &= expect(!M11_Audio_PlayCsbPc34RuntimePcm(
                     &state, source, (int)sizeof(source), 2000000,
                     fnv1a(source, (int)sizeof(source))),
                 "PIT divisor above the source clock is rejected");
    M11_Audio_Shutdown(&state);
    return ok ? 0 : 1;
}
