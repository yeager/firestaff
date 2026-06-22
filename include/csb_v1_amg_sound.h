/*
 * csb_v1_amg_sound.h
 *
 * CSB Utility Disk AMG sound-effect container metadata.
 *
 * Observed CSB Utility Disk sound-effect AMG files use:
 *   u16be sampleByteCount
 *   u16be controlWord
 *   i8    sampleBytes[sampleByteCount]
 *
 * NAKED.AMG under the Utility Disk MIDI directory is not this
 * sound-effect container and is intentionally rejected by this parser.
 */
#ifndef FIRESTAFF_CSB_V1_AMG_SOUND_H
#define FIRESTAFF_CSB_V1_AMG_SOUND_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CsbV1AmgSoundInfo {
    uint16_t sampleByteCount;
    uint16_t controlWord;
    size_t sampleOffset;
    int8_t minSample;
    int8_t maxSample;
} CsbV1AmgSoundInfo;

int csb_v1_amg_sound_parse(const uint8_t* data,
                           size_t size,
                           CsbV1AmgSoundInfo* outInfo);

int16_t csb_v1_amg_sound_sample_to_s16(uint8_t sampleByte);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_AMG_SOUND_H */
