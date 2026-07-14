#include "redmcsb_f0434_dungeon_tail_pc34_compat.h"

#include <limits.h>

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint16_t add_part_checksum(uint16_t checksum, const uint8_t *bytes,
                                  size_t byte_count)
{
    size_t byte_index;

    for (byte_index = 0U; byte_index < byte_count; ++byte_index) {
        checksum = (uint16_t)(checksum + bytes[byte_index]);
    }
    return checksum;
}

int redmcsb_f0434_load_dungeon_tail_pc34(
    RedmcsbF1910ReadExactPc34 read, void *context,
    RedmcsbF0434DungeonTailPartPc34
        parts[REDMCSB_F7063_DUNGEON_PART_COUNT],
    RedmcsbF0434DungeonTailReceiptPc34 *receipt)
{
    uint8_t checksum_bytes[2];
    unsigned int part_index;

    if (receipt != NULL) {
        *receipt = (RedmcsbF0434DungeonTailReceiptPc34){0};
        receipt->failed_part = REDMCSB_F7063_DUNGEON_PART_COUNT;
    }
    if (read == NULL || parts == NULL || receipt == NULL) {
        return REDMCSB_F0434_PC34_RESULT_PRECONDITION_FAILED;
    }
    for (part_index = 0U; part_index < REDMCSB_F7063_DUNGEON_PART_COUNT;
         ++part_index) {
        if (parts[part_index].byte_count > (size_t)INT16_MAX ||
            (parts[part_index].byte_count != 0U &&
             parts[part_index].bytes == NULL)) {
            receipt->failed_part = part_index;
            return REDMCSB_F0434_PC34_RESULT_PRECONDITION_FAILED;
        }
        if (!redmcsb_f1910_load_saved_game_part_pc34(
                read, context, parts[part_index].bytes,
                parts[part_index].byte_count)) {
            receipt->failed_part = part_index;
            return REDMCSB_F0434_PC34_RESULT_PART_READ_FAILED;
        }
        receipt->calculated_checksum = add_part_checksum(
            receipt->calculated_checksum, parts[part_index].bytes,
            parts[part_index].byte_count);
        ++receipt->parts_loaded;
    }
    if (!redmcsb_f1910_load_saved_game_part_pc34(
            read, context, checksum_bytes, sizeof(checksum_bytes))) {
        return REDMCSB_F0434_PC34_RESULT_CHECKSUM_READ_FAILED;
    }
    receipt->stored_checksum = read_le16(checksum_bytes);
    if (receipt->stored_checksum != receipt->calculated_checksum) {
        return REDMCSB_F0434_PC34_RESULT_CHECKSUM_MISMATCH;
    }
    return REDMCSB_F0434_PC34_RESULT_OK;
}

const char *redmcsb_f0434_dungeon_tail_pc34_source_evidence(void)
{
    return "ReDMCSB LOADSAVE.C F0434_STARTEND_IsLoadDungeonSuccessful_CPSC; "
           "READWRIT.C F0421_SAVEUTIL_IsReadBytesWithChecksumSuccessful; "
           "CEDTINCA.C F7063_LoadDungeon";
}
