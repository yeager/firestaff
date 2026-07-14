#include "redmcsb_f7061_save_header_pc34_compat.h"

#include "redmcsb_f7055_saveutil_pc34_compat.h"

#define REDMCSB_F7061_HEADER_BYTES 512U
#define REDMCSB_F7061_CLEAR_BYTES 256U
#define REDMCSB_F7061_WORDS_PER_HALF 128U

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

int redmcsb_f7061_is_read_save_header_successful_pc34(
    uint8_t *header, size_t header_size, uint16_t key_word_index)
{
    uint16_t expected_checksum = 0;
    uint16_t actual_checksum = 0;
    uint16_t key;
    size_t word_index;

    if (header == NULL || header_size != REDMCSB_F7061_HEADER_BYTES ||
        key_word_index >= REDMCSB_F7061_WORDS_PER_HALF) {
        return 0;
    }

    for (word_index = 0; word_index < REDMCSB_F7061_WORDS_PER_HALF;
         word_index += 4U) {
        expected_checksum = (uint16_t)(expected_checksum +
            read_le16(header + word_index * 2U));
        expected_checksum = (uint16_t)(expected_checksum ^
            read_le16(header + (word_index + 1U) * 2U));
        expected_checksum = (uint16_t)(expected_checksum -
            read_le16(header + (word_index + 2U) * 2U));
        expected_checksum = (uint16_t)(expected_checksum ^
            read_le16(header + (word_index + 3U) * 2U));
    }

    key = read_le16(header + (size_t)key_word_index * 2U);
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        header + REDMCSB_F7061_CLEAR_BYTES, REDMCSB_F7061_CLEAR_BYTES, key);

    for (word_index = 0; word_index < REDMCSB_F7061_WORDS_PER_HALF;
         ++word_index) {
        actual_checksum = (uint16_t)(actual_checksum + read_le16(
            header + REDMCSB_F7061_CLEAR_BYTES + word_index * 2U));
    }

    return expected_checksum == actual_checksum;
}

const char *redmcsb_f7061_save_header_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINC6.C F7061/F0429";
}
