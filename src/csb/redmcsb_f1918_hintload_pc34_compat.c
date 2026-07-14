#include "redmcsb_f1918_hintload_pc34_compat.h"

#include "redmcsb_f7055_saveutil_pc34_compat.h"
#include "redmcsb_f7061_save_header_pc34_compat.h"

#include <string.h>

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

int redmcsb_f1910_load_saved_game_part_pc34(RedmcsbF1910ReadExactPc34 read,
                                            void *context,
                                            uint8_t *destination,
                                            size_t byte_count)
{
    if (byte_count == 0U) {
        return 1;
    }
    if (read == NULL || destination == NULL) {
        return 0;
    }
    return read(context, destination, byte_count) != 0;
}

int redmcsb_f1913_load_and_deobfuscate_saved_game_part_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context, uint8_t *destination,
    size_t byte_count, uint16_t key, uint16_t checksum)
{
    uint16_t actual;

    if ((byte_count & 1U) != 0U ||
        !redmcsb_f1910_load_saved_game_part_pc34(read, context, destination,
                                                 byte_count)) {
        return 0;
    }
    if (byte_count == 0U) {
        return 1;
    }
    actual = redmcsb_f7055_saveutil_get_checksum_and_obfuscate_pc34(
        destination, byte_count, key);
    return actual == checksum;
}

int redmcsb_f1914_load_and_deobfuscate_saved_game_header_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context,
    uint8_t header[REDMCSB_F1918_PC34_HEADER_BYTES])
{
    if (!redmcsb_f1910_load_saved_game_part_pc34(
            read, context, header, REDMCSB_F1918_PC34_HEADER_BYTES)) {
        return 0;
    }
    return redmcsb_f7061_is_read_save_header_successful_pc34(
        header, REDMCSB_F1918_PC34_HEADER_BYTES,
        REDMCSB_F1918_PC34_HEADER_KEY_WORD_INDEX);
}

int redmcsb_f1918_load_initial_save_parts_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context,
    RedmcsbF1918LoadReceiptPc34 *receipt)
{
    unsigned int part;
    static const int failure_codes[REDMCSB_F1918_PC34_PART_COUNT] = {
        REDMCSB_F1918_PC34_RESULT_GLOBAL_DATA_FAILED,
        REDMCSB_F1918_PC34_RESULT_ACTIVE_GROUPS_FAILED,
        REDMCSB_F1918_PC34_RESULT_PARTY_FAILED
    };

    if (read == NULL || receipt == NULL) {
        return REDMCSB_F1918_PC34_RESULT_HEADER_READ_FAILED;
    }
    receipt->header_valid = 0;
    receipt->parts_loaded = 0U;
    if (!redmcsb_f1914_load_and_deobfuscate_saved_game_header_pc34(
            read, context, receipt->header)) {
        return REDMCSB_F1918_PC34_RESULT_HEADER_READ_FAILED;
    }
    receipt->header_valid = 1;
    for (part = 0U; part < REDMCSB_F1918_PC34_PART_COUNT; ++part) {
        const size_t key_offset = REDMCSB_F1918_PC34_HEADER_KEYS_OFFSET +
                                  (size_t)part * 2U;
        const size_t checksum_offset =
            REDMCSB_F1918_PC34_HEADER_CHECKSUMS_OFFSET + (size_t)part * 2U;

        if (receipt->parts[part].bytes == NULL ||
            receipt->parts[part].byte_count == 0U ||
            (receipt->parts[part].byte_count & 1U) != 0U) {
            return failure_codes[part];
        }
        receipt->keys[part] = read_le16(receipt->header + key_offset);
        receipt->checksums[part] =
            read_le16(receipt->header + checksum_offset);
        if (!redmcsb_f1913_load_and_deobfuscate_saved_game_part_pc34(
                read, context, receipt->parts[part].bytes,
                receipt->parts[part].byte_count, receipt->keys[part],
                receipt->checksums[part])) {
            return failure_codes[part];
        }
        ++receipt->parts_loaded;
    }
    return REDMCSB_F1918_PC34_RESULT_OK;
}

const char *redmcsb_f1918_hintload_pc34_source_evidence(void)
{
    return "ReDMCSB HINTLOAD.C F1910_LoadSavedGamePart; "
           "F1913_LoadAndDeobfuscateSavedGamePart; "
           "F1914_LoadAndDeobfuscateSavedGameHeader; "
           "F1918_LoadGame_CPSX; DEFS.H CSB_SAVE_HEADER offsets 0x138/0x158";
}
