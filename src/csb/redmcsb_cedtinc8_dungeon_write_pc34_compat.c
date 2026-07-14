#include "redmcsb_cedtinc8_dungeon_write_pc34_compat.h"

#include <stdint.h>
#include <string.h>

int redmcsb_cedtinc8_write_dungeon_stream_pc34(
    const RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT],
    uint8_t *written_bytes, size_t written_bytes_size, size_t *written_size)
{
    size_t required_size = 2U;
    uint16_t checksum = 0;
    uint16_t part_index;
    uint8_t *cursor;

    if (parts == NULL || written_bytes == NULL || written_size == NULL) {
        return 0;
    }
    for (part_index = 0; part_index < REDMCSB_F7063_DUNGEON_PART_COUNT;
         ++part_index) {
        uint16_t byte_count = parts[part_index].byte_count;

        if (byte_count != 0U && parts[part_index].bytes == NULL) {
            return 0;
        }
        if ((size_t)byte_count > SIZE_MAX - required_size) {
            return 0;
        }
        required_size += byte_count;
    }
    if (written_bytes_size < required_size) {
        return 0;
    }

    cursor = written_bytes;
    for (part_index = 0; part_index < REDMCSB_F7063_DUNGEON_PART_COUNT;
         ++part_index) {
        const uint8_t *bytes = parts[part_index].bytes;
        uint16_t byte_count = parts[part_index].byte_count;
        uint16_t byte_index;

        if (byte_count != 0U) {
            memcpy(cursor, bytes, byte_count);
            cursor += byte_count;
        }
        for (byte_index = 0; byte_index < byte_count; ++byte_index) {
            checksum = (uint16_t)(checksum + bytes[byte_index]);
        }
    }
    cursor[0] = (uint8_t)checksum;
    cursor[1] = (uint8_t)(checksum >> 8);
    *written_size = required_size;
    return 1;
}

const char *redmcsb_cedtinc8_dungeon_write_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINC8.C SAVE_GAME and CEDTINC6.C F7060";
}
