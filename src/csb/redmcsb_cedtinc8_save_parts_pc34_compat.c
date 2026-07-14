#include "redmcsb_cedtinc8_save_parts_pc34_compat.h"

#include "redmcsb_f7055_saveutil_pc34_compat.h"

#include <string.h>

int redmcsb_cedtinc8_prepare_save_parts_pc34(
    RedmcsbCedtinc8SavePart parts[REDMCSB_CEDTINC8_SAVE_PART_COUNT],
    const uint16_t keys[REDMCSB_CEDTINC8_SAVE_HEADER_KEY_COUNT],
    uint16_t checksums[REDMCSB_CEDTINC8_SAVE_PART_COUNT])
{
    uint16_t part_index;

    if (parts == NULL || keys == NULL || checksums == NULL) {
        return 0;
    }

    for (part_index = 0; part_index < REDMCSB_CEDTINC8_SAVE_PART_COUNT;
         ++part_index) {
        if (parts[part_index].plaintext == NULL ||
            parts[part_index].written_bytes == NULL ||
            parts[part_index].plaintext == parts[part_index].written_bytes ||
            parts[part_index].byte_count == 0U ||
            (parts[part_index].byte_count & 1U) != 0U) {
            return 0;
        }
    }

    /* CEDTINC8.C calculates all five header checksums before writing parts. */
    for (part_index = 0; part_index < REDMCSB_CEDTINC8_SAVE_PART_COUNT;
         ++part_index) {
        checksums[part_index] =
            redmcsb_f7056_saveutil_get_checksum_pc34(
                parts[part_index].plaintext, parts[part_index].byte_count,
                keys[part_index]);
    }

    for (part_index = 0; part_index < REDMCSB_CEDTINC8_SAVE_PART_COUNT;
         ++part_index) {
        uint16_t written_checksum;

        written_checksum = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
            parts[part_index].plaintext, parts[part_index].byte_count,
            keys[part_index]);
        memcpy(parts[part_index].written_bytes, parts[part_index].plaintext,
               parts[part_index].byte_count);
        (void)redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
            parts[part_index].plaintext, parts[part_index].byte_count,
            keys[part_index]);
        if (written_checksum != checksums[part_index]) {
            return 0;
        }
    }

    return 1;
}

const char *redmcsb_cedtinc8_save_parts_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINC8.C SAVE_GAME five-part checksum/write sequence";
}
