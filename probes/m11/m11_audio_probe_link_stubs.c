/*
 * Link-only stubs for the standalone DM1 SND3 probe.
 *
 * audio_sdl_m11.c also exposes CSB PSG and DM1 swoosh routes.  This probe
 * exercises neither route, so keeping their full graphics-memory dependency
 * graph out of the small original-SND3 harness is deliberate.
 */
#include "csb_v1_audio_runtime_pc34_compat.h"

int SWSH_Compat_ValidatePc34DosoundProgram(const unsigned char* program,
                                           unsigned int byteCount) {
    (void)program;
    (void)byteCount;
    return 0;
}

CsbV1PsgChannelAmplitudes
csb_v1_audio_runtime_channel_amplitudes(int16_t amplitudeIndex) {
    CsbV1PsgChannelAmplitudes amplitudes = {0, 0, 0};
    (void)amplitudeIndex;
    return amplitudes;
}

int csb_v1_audio_runtime_decode_st_sound(const uint8_t* encoded,
                                         size_t encodedSize,
                                         uint8_t initialLevel,
                                         uint8_t* outLevels,
                                         size_t outLevelCapacity,
                                         CsbV1StSoundDecodeResult* outResult) {
    (void)encoded;
    (void)encodedSize;
    (void)initialLevel;
    (void)outLevels;
    (void)outLevelCapacity;
    (void)outResult;
    return -1;
}
