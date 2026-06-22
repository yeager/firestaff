/*
 * csb_v1_amg_sound.c
 *
 * CSB Utility Disk sound-effect AMG parser.
 */

#include "csb_v1_amg_sound.h"

static uint16_t read_be16(const uint8_t* p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

int csb_v1_amg_sound_parse(const uint8_t* data,
                           size_t size,
                           CsbV1AmgSoundInfo* outInfo)
{
    uint16_t sampleBytes;
    uint16_t controlWord;
    size_t i;
    int8_t minSample;
    int8_t maxSample;

    if (!data || !outInfo || size < 5u || size > 0x10003u) {
        return -1;
    }

    sampleBytes = read_be16(data);
    controlWord = read_be16(data + 2);
    if (sampleBytes == 0u || (size_t)sampleBytes + 4u != size) {
        return -2;
    }
    if (controlWord == 0u) {
        return -3;
    }

    minSample = (int8_t)data[4];
    maxSample = (int8_t)data[4];
    for (i = 0; i < sampleBytes; ++i) {
        int8_t s = (int8_t)data[4u + i];
        if (s < minSample) minSample = s;
        if (s > maxSample) maxSample = s;
    }

    outInfo->sampleByteCount = sampleBytes;
    outInfo->controlWord = controlWord;
    outInfo->sampleOffset = 4u;
    outInfo->minSample = minSample;
    outInfo->maxSample = maxSample;
    return 0;
}

int16_t csb_v1_amg_sound_sample_to_s16(uint8_t sampleByte)
{
    return (int16_t)((int16_t)(int8_t)sampleByte << 8);
}
