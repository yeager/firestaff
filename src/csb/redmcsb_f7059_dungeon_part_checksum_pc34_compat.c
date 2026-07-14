#include "redmcsb_f7059_dungeon_part_checksum_pc34_compat.h"

static void accumulate_dungeon_part(const uint8_t *buffer, uint16_t byte_count,
                                    uint16_t *checksum)
{
    uint16_t part_checksum = 0;

    while (byte_count-- != 0) {
        part_checksum = (uint16_t)(part_checksum + *buffer++);
    }
    *checksum = (uint16_t)(*checksum + part_checksum);
}

void redmcsb_f7059_read_dungeon_part_with_checksum_pc34(
    const uint8_t *buffer, uint16_t byte_count, uint16_t *checksum)
{
    accumulate_dungeon_part(buffer, byte_count, checksum);
}

void redmcsb_f7060_write_dungeon_part_with_checksum_pc34(
    const uint8_t *buffer, uint16_t byte_count, uint16_t *checksum)
{
    accumulate_dungeon_part(buffer, byte_count, checksum);
}

const char *redmcsb_f7059_dungeon_part_checksum_pc34_source_evidence(void)
{
    return "ReDMCSB CEDTINC6.C F7059/F7060";
}
