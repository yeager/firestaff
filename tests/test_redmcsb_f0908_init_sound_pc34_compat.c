/* ReDMCSB SWSHSND.C:10-24 F0908_InitSound (MEDIA428) contract test. */
#include "redmcsb_f0908_init_sound_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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
    static const uint8_t source[] = { 0x80u, 0x00u, 0x7fu, 0x55u };
    uint8_t owned[sizeof(source)] = { 0u };
    uint8_t too_small[sizeof(source) - 1u] = { 0u };
    RedmcsbF0908SoundStatePc34 state;
    int passed = 1;

    memset(&state, 0, sizeof(state));
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source), 357,
                                                owned, sizeof(owned),
                                                &state) == 1,
                    "F0908 accepts caller-owned chip-copy storage");
    passed &= check(memcmp(owned, source, sizeof(source)) == 0,
                    "F0908 copies G0746 swoosh bytes");
    passed &= check(state.left.data == owned && state.right.data == owned,
                    "both channels use the same copied swoosh buffer");
    passed &= check(state.left.length == sizeof(source) &&
                    state.right.length == sizeof(source),
                    "both channels receive G0745 byte count");
    passed &= check(state.left.period == 357 && state.right.period == 357,
                    "both channels receive G0744 period");
    passed &= check(RedmcsbF0908_InitSoundPc34(source, sizeof(source), 357,
                                                too_small, sizeof(too_small),
                                                &state) == 0,
                    "insufficient host storage is rejected");
    passed &= check(RedmcsbF0908_InitSoundPc34(NULL, 0u, -1, NULL, 0u,
                                                &state) == 1,
                    "zero-byte source keeps the exact period value");
    passed &= check(state.left.data == NULL && state.right.data == NULL &&
                    state.left.length == 0u && state.right.length == 0u &&
                    state.left.period == -1 && state.right.period == -1,
                    "zero-byte channel descriptors are copied symmetrically");

    return passed ? 0 : 1;
}
