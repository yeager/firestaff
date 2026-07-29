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
    size_t trailingByteCount;
    size_t i;
    int8_t minSample;
    int8_t maxSample;

    if (!data || !outInfo || size < 2u || size > 0x10003u) {
        return -1;
    }

    sampleBytes = read_be16(data);
    if ((size_t)sampleBytes + 2u > size) {
        return -2;
    }
    trailingByteCount = size - ((size_t)sampleBytes + 2u);
    if (trailingByteCount > 3u) {
        return -3;
    }

    if (sampleBytes == 0u) {
        minSample = 0;
        maxSample = 0;
    } else {
        minSample = (int8_t)data[2];
        maxSample = (int8_t)data[2];
    }
    for (i = 0; i < sampleBytes; ++i) {
        int8_t s = (int8_t)data[2u + i];
        if (s < minSample) minSample = s;
        if (s > maxSample) maxSample = s;
    }

    outInfo->sampleByteCount = sampleBytes;
    outInfo->trailingByteCount = trailingByteCount;
    outInfo->sampleOffset = 2u;
    outInfo->minSample = minSample;
    outInfo->maxSample = maxSample;
    return 0;
}

int16_t csb_v1_amg_sound_sample_to_s16(uint8_t sampleByte)
{
    return (int16_t)((int16_t)(int8_t)sampleByte << 8);
}
