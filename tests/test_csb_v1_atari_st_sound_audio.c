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
    M11_AudioState state;
    unsigned int hash = fnv1a(snd1, (int)sizeof(snd1));
    int ok = 1;

    memset(&state, 0, sizeof(state));
    ok &= expect(M11_Audio_Init(&state), "audio state initializes");
    ok &= expect(M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 112, hash),
                 "source-owned Atari SND1 accepts at its Timer-A period");
    ok &= expect(state.csbAtariStSoundAccepted &&
                     state.csbAtariStSoundPeriod == 112 &&
                     state.csbAtariStSoundHash == hash &&
                     state.csbAtariStPsg.sampleCount > 0,
                 "decoded PSG stream retains source identity and output");
    snd1[2] ^= 0x80u;
    ok &= expect(!M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 112, hash),
                 "changed SND1 stream is rejected rather than substituted");
    ok &= expect(!M11_Audio_PlayCsbAtariStPsg(&state, snd1,
                     (int)sizeof(snd1), 10, fnv1a(snd1, (int)sizeof(snd1))),
                 "invalid Timer-A period is rejected");
    M11_Audio_Shutdown(&state);
    return ok ? 0 : 1;
}
