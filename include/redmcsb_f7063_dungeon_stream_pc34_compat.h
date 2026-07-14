#ifndef REDMCSB_F7063_DUNGEON_STREAM_PC34_COMPAT_H
#define REDMCSB_F7063_DUNGEON_STREAM_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB CEDTINCA.C F7063_LoadDungeon, PC34 stream checksum boundary. */

#define REDMCSB_F7063_DUNGEON_PART_COUNT 22U

typedef enum RedmcsbF7063DungeonPartIndex {
    REDMCSB_F7063_PART_HEADER = 0,
    REDMCSB_F7063_PART_MAPS,
    REDMCSB_F7063_PART_COLUMNS_CUMULATIVE_SQUARE_THINGS,
    REDMCSB_F7063_PART_SQUARE_FIRST_THINGS,
    REDMCSB_F7063_PART_TEXT_DATA,
    REDMCSB_F7063_PART_THING_DATA_0,
    REDMCSB_F7063_PART_THING_DATA_15 =
        REDMCSB_F7063_PART_THING_DATA_0 + 15,
    REDMCSB_F7063_PART_RAW_MAP_DATA
} RedmcsbF7063DungeonPartIndex;

typedef struct RedmcsbF7063DungeonPart {
    const uint8_t *bytes;
    uint16_t byte_count;
} RedmcsbF7063DungeonPart;

/*
 * Verifies F7063's fixed read sequence against its trailing 16-bit checksum.
 * The parts remain opaque: this routine does not parse their header, map,
 * ThingData, or runtime semantics.
 */
int redmcsb_f7063_validate_dungeon_stream_checksum_pc34(
    const RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT],
    uint16_t stored_checksum);

const char *redmcsb_f7063_dungeon_stream_pc34_source_evidence(void);

#endif
