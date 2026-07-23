/* ReDMCSB SWSHSND.C:10-24 F0908_InitSound (MEDIA428) contract test. */
#include "redmcsb_f0908_init_sound_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum { SOURCE_BYTE_COUNT = 9078 };

static uint32_t fnv1a(const uint8_t *data, size_t size)
{
    uint32_t value = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        value ^= data[i];
        value *= 16777619u;
    }
    return value;
}

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void)
{
    uint8_t source[SOURCE_BYTE_COUNT];
    uint8_t owned[SOURCE_BYTE_COUNT] = { 0u };
    uint8_t too_small[SOURCE_BYTE_COUNT - 1u] = { 0u };
    RedmcsbF0908SoundStatePc34 state;
    uint32_t source_hash;
    int passed = 1;
    size_t i;

    for (i = 0u; i < sizeof(source); ++i) {
        source[i] = (uint8_t)((i * 37u + 11u) & 0xffu);
    }
    source_hash = fnv1a(source, sizeof(source));

    memset(&state, 0, sizeof(state));
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source), 334,
                                                source_hash,
                                                owned, sizeof(owned),
                                                &state) == 1,
                    "F0908 accepts hash-bound source bytes at source timing");
    passed &= check(memcmp(owned, source, sizeof(source)) == 0,
                    "F0908 copies G0746 swoosh bytes");
    passed &= check(state.left.data == owned && state.right.data == owned,
                    "both channels use the same copied swoosh buffer");
    passed &= check(state.left.length == sizeof(source) &&
                    state.right.length == sizeof(source),
                    "both channels receive G0745 byte count");
    passed &= check(state.left.period == 334 && state.right.period == 334,
                    "both channels receive G0744 period");
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source), 334,
                                                source_hash,
                                                too_small, sizeof(too_small),
                                                &state) == 0,
                    "insufficient host storage is rejected");
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source) - 1u, 334,
                                                source_hash, owned, sizeof(owned),
                                                &state) == 0,
                    "non-source byte count is rejected");
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source), 357,
                                                source_hash, owned, sizeof(owned),
                                                &state) == 0,
                    "non-source period is rejected");
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source), 334,
                                                source_hash ^ 1u, owned, sizeof(owned),
                                                &state) == 0,
                    "mismatched source identity is rejected");
    passed &= check(RedmcsbF0908_InitSoundPc34(NULL, sizeof(source), 334,
                                                source_hash, owned, sizeof(owned),
                                                &state) == 0,
                    "missing source bytes are rejected");

    return passed ? 0 : 1;
}
