/* ReDMCSB IO.C:3771-3785 F0707_ResetSoundBufferAddress (I34E PC 3.4). */
#include "dm1_v1_f0707_reset_sound_buffer_pc34_compat.h"

int DM1V1_F0707_ResetSoundBufferAddressPc34(
    DM1V1_F0707_SoundDescriptorPc34 *soundDescriptors,
    int16_t soundIndex) {
    if (soundIndex < 0 || soundDescriptors[soundIndex].graphicIndex < 0) {
        return 0;
    }

    soundDescriptors[soundIndex].buffer = 0;
    return 1;
}
