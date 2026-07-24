/* CSB PC34 SWSHSND.C F0908: only authenticated raw sample bytes may reach
 * the host mixer.  This is deliberately independent of SDL availability. */
#include "audio_sdl_m11.h"

#include <stdio.h>
#include <string.h>

enum { kCsbSwshBytes = 9078, kCsbSwshPeriod = 334 };

static unsigned int fnv1a(const unsigned char* bytes, int count) {
    unsigned int hash = 2166136261u;
    int index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static int expect(int condition, const char* message) {
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void) {
    M11_AudioState state;
    unsigned char raw[kCsbSwshBytes];
    unsigned int hash;
    int index;
    int ok = 1;

    /* A caller supplies bytes extracted from its selected CSB package. The
     * test uses a deterministic opaque payload to exercise the transport and
     * integrity gate, not a fabricated replacement waveform. */
    for (index = 0; index < kCsbSwshBytes; ++index) {
        raw[index] = (unsigned char)((index * 37 + 19) & 0xff);
    }
    hash = fnv1a(raw, kCsbSwshBytes);
    memset(&state, 0, sizeof(state));
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    ok &= expect(M11_Audio_PlayCsbSwshPcm(&state, raw, kCsbSwshBytes,
                                           kCsbSwshPeriod, hash),
                 "exact raw package sample is accepted");
    ok &= expect(state.csbSwshSourceAccepted &&
                     state.csbSwshSourceByteCount == kCsbSwshBytes &&
                     state.csbSwshSourcePeriod == kCsbSwshPeriod &&
                     state.csbSwshSourceHash == hash &&
                     state.csbSwshPcm.sampleCount > 0,
                 "accepted source retains its own bytes, period and output");

    raw[0] ^= 0x80u;
    ok &= expect(!M11_Audio_PlayCsbSwshPcm(&state, raw, kCsbSwshBytes,
                                            kCsbSwshPeriod, hash),
                 "changed package data is rejected instead of substituted");
    ok &= expect(!state.csbSwshSourceAccepted &&
                     state.csbSwshPcm.sampleCount == 0,
                 "rejection clears stale raw source audio");
    ok &= expect(!M11_Audio_PlayCsbSwshPcm(&state, raw,
                                            kCsbSwshBytes - 1,
                                            kCsbSwshPeriod, fnv1a(raw,
                                                                    kCsbSwshBytes - 1)),
                 "wrong PC34 source size is rejected");
    ok &= expect(!M11_Audio_PlayCsbSwshPcm(&state, raw, kCsbSwshBytes,
                                            kCsbSwshPeriod + 1,
                                            fnv1a(raw, kCsbSwshBytes)),
                 "wrong PC34 DMA period is rejected");
    M11_Audio_Shutdown(&state);
    return ok ? 0 : 1;
}
