#ifndef FIRESTAFF_DM1_V1_F0707_RESET_SOUND_BUFFER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0707_RESET_SOUND_BUFFER_PC34_COMPAT_H

/*
 * ReDMCSB IO.C:3771-3785, F0707_ResetSoundBufferAddress (I34E PC 3.4).
 *
 * The sound descriptor has a source GRAPHICS.DAT index and a lazily-loaded
 * decoded buffer.  Invalid sound indices and absent source graphics are
 * rejected; a valid descriptor only drops its cached buffer.
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t graphicIndex;
    void *buffer;
} DM1V1_F0707_SoundDescriptorPc34;

int DM1V1_F0707_ResetSoundBufferAddressPc34(
    DM1V1_F0707_SoundDescriptorPc34 *soundDescriptors,
    int16_t soundIndex);

#ifdef __cplusplus
}
#endif

#endif
