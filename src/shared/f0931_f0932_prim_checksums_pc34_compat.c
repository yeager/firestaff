#include "f0931_f0932_prim_checksums_pc34_compat.h"

uint16_t f0931_checksum_words_pc34_compat(
    const int16_t *words, size_t byte_count)
{
    uint16_t checksum = 0u;

    if (!words) return 0u;
    while (byte_count > 0u) {
        checksum = (uint16_t)(checksum + (uint16_t)*words++);
        byte_count -= byte_count >= 2u ? 2u : byte_count;
    }
    return checksum;
}

uint16_t f0932_checksum_bytes_pc34_compat(
    const uint8_t *bytes, size_t byte_count)
{
    uint16_t checksum = 0u;

    if (!bytes) return 0u;
    while (byte_count-- > 0u) {
        checksum = (uint16_t)(checksum + *bytes++);
    }
    return checksum;
}
