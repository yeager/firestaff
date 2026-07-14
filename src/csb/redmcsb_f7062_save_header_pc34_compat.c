#include "redmcsb_f7062_save_header_pc34_compat.h"

#include "redmcsb_f7055_saveutil_pc34_compat.h"

#include <string.h>

#define REDMCSB_F7062_HALF_BYTES 256U
#define REDMCSB_F7062_HALF_WORDS 128U

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

int redmcsb_f7062_prepare_obfuscated_save_header_pc34(
    uint8_t *header, size_t header_size, uint16_t key_word_index,
    const uint16_t *random_words, size_t random_word_count,
    uint8_t *encoded_header, size_t encoded_header_size)
{
    uint16_t tail_checksum = 0;
    uint16_t last_word = 0;
    uint16_t key;
    size_t word_index;
    size_t random_index = 0;

    if (header == NULL || encoded_header == NULL || random_words == NULL ||
        header == encoded_header || header_size != REDMCSB_F7062_HEADER_BYTES ||
        encoded_header_size != REDMCSB_F7062_HEADER_BYTES ||
        key_word_index >= REDMCSB_F7062_HALF_WORDS ||
        random_word_count != REDMCSB_F7062_RANDOM_WORDS) {
        return 0;
    }

    for (word_index = 0; word_index < REDMCSB_F7062_HALF_WORDS; ++word_index) {
        tail_checksum = (uint16_t)(tail_checksum + read_le16(
            header + REDMCSB_F7062_HALF_BYTES + word_index * 2U));
    }

    for (word_index = 0; word_index < 32U; ++word_index) {
        uint16_t word;

        word = random_words[random_index++];
        write_le16(header + (word_index * 4U) * 2U, word);
        last_word = (uint16_t)(last_word + word);

        word = random_words[random_index++];
        write_le16(header + (word_index * 4U + 1U) * 2U, word);
        last_word = (uint16_t)(last_word ^ word);

        word = random_words[random_index++];
        write_le16(header + (word_index * 4U + 2U) * 2U, word);
        last_word = (uint16_t)(last_word - word);

        if (word_index != 31U) {
            word = random_words[random_index++];
            write_le16(header + (word_index * 4U + 3U) * 2U, word);
            last_word = (uint16_t)(last_word ^ word);
        }
    }
    write_le16(header + 127U * 2U, (uint16_t)(last_word ^ tail_checksum));

    key = read_le16(header + (size_t)key_word_index * 2U);
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        header + REDMCSB_F7062_HALF_BYTES, REDMCSB_F7062_HALF_BYTES, key);
    memcpy(encoded_header, header, REDMCSB_F7062_HEADER_BYTES);
    (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        header + REDMCSB_F7062_HALF_BYTES, REDMCSB_F7062_HALF_BYTES, key);

    return 1;
}

const char *redmcsb_f7062_save_header_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINC6.C F7062/F0430";
}
