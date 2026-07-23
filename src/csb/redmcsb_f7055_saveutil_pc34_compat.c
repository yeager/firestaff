#include "redmcsb_f7055_saveutil_pc34_compat.h"

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void write_le16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

uint16_t redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
    uint8_t *buffer, size_t byte_count, uint16_t key)
{
    uint16_t checksum;
    size_t word_count;
    size_t i;

    if (!buffer || byte_count == 0u || (byte_count & 1u) != 0u) {
        return 0u;
    }
    word_count = byte_count >> 1;
    checksum = key;
    for (i = 0u; i < word_count; ++i) {
        uint16_t word = read_le16(buffer + i * 2u);

        checksum = (uint16_t)(checksum + word);
        word = (uint16_t)(word ^ key);
        write_le16(buffer + i * 2u, word);
        checksum = (uint16_t)(checksum + word);
        key = (uint16_t)(key + (uint16_t)(word_count - i));
    }
    return checksum;
}

uint16_t redmcsb_f7056_saveutil_get_checksum_pc34(
    const uint8_t *buffer, size_t byte_count, uint16_t key)
{
    uint16_t checksum;
    size_t word_count;
    size_t i;

    if (!buffer || byte_count == 0u || (byte_count & 1u) != 0u) {
        return 0u;
    }
    word_count = byte_count >> 1;
    checksum = key;
    for (i = 0u; i < word_count; ++i) {
        const uint16_t word = read_le16(buffer + i * 2u);

        checksum = (uint16_t)(checksum + word);
        checksum = (uint16_t)(checksum + (uint16_t)(word ^ key));
        key = (uint16_t)(key + (uint16_t)(word_count - i));
    }
    return checksum;
}

int redmcsb_f7057_read_save_part_with_checksum_pc34(
    uint8_t *buffer, size_t byte_count, uint16_t key, uint16_t checksum)
{
    uint16_t actual;

    if (!buffer || byte_count == 0u || (byte_count & 1u) != 0u) {
        return 0;
    }
    actual = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        buffer, byte_count, key);
    return actual == checksum;
}

int redmcsb_f7058_write_save_part_with_checksum_pc34(
    uint8_t *buffer, size_t byte_count, uint16_t key, uint16_t *checksum)
{
    uint16_t actual;

    if (!buffer || !checksum || byte_count == 0u ||
        (byte_count & 1u) != 0u) {
        return 0;
    }
    actual = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        buffer, byte_count, key);
    *checksum = actual;
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        buffer, byte_count, key);
    return 1;
}

const char *redmcsb_f7055_saveutil_source_evidence_pc34(void)
{
    return "ReDMCSB CEDTINC6.C F7055/F7056/F7057/F7058";
}
