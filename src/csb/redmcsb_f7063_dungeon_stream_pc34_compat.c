#include "redmcsb_f7063_dungeon_stream_pc34_compat.h"

#include <stddef.h>

int redmcsb_f7063_validate_dungeon_stream_checksum_pc34(
    const RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT],
    uint16_t stored_checksum)
{
    uint16_t checksum = 0;
    uint16_t part_index;

    if (parts == NULL) {
        return 0;
    }

    for (part_index = 0; part_index < REDMCSB_F7063_DUNGEON_PART_COUNT;
         ++part_index) {
        const uint8_t *bytes = parts[part_index].bytes;
        uint16_t byte_count = parts[part_index].byte_count;

        if (byte_count != 0U && bytes == NULL) {
            return 0;
        }
        while (byte_count-- != 0U) {
            checksum = (uint16_t)(checksum + *bytes++);
        }
    }

    return checksum == stored_checksum;
}

const char *redmcsb_f7063_dungeon_stream_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINCA.C F7063_LoadDungeon";
}
