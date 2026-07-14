/* Isolated F0707 contract test: ReDMCSB IO.C:3771-3785 (I34E PC 3.4). */
#include "dm1_v1_f0707_reset_sound_buffer_pc34_compat.h"

#include <stdio.h>

static int check(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    DM1V1_F0707_SoundDescriptorPc34 sounds[2] = {
        { 673, (void *)0x1 },
        { -1, (void *)0x2 }
    };
    int passed = 1;

    passed &= check(DM1V1_F0707_ResetSoundBufferAddressPc34(sounds, 0) == 1,
                    "valid graphic index returns true");
    passed &= check(sounds[0].buffer == 0,
                    "valid descriptor cache is cleared");
    passed &= check(DM1V1_F0707_ResetSoundBufferAddressPc34(sounds, 1) == 0,
                    "negative graphic index returns false");
    passed &= check(sounds[1].buffer == (void *)0x2,
                    "invalid descriptor cache remains intact");
    passed &= check(DM1V1_F0707_ResetSoundBufferAddressPc34(sounds, -1) == 0,
                    "negative sound index returns false");

    return passed ? 0 : 1;
}
