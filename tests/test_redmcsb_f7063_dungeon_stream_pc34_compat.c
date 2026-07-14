#include "redmcsb_f7063_dungeon_stream_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        failures++; \
    } \
} while (0)

static uint16_t make_full_stream(RedmcsbF7063DungeonPart *parts,
                                 uint8_t *bytes)
{
    uint16_t checksum = 0;
    uint16_t index;

    for (index = 0; index < REDMCSB_F7063_DUNGEON_PART_COUNT; ++index) {
        bytes[index] = (uint8_t)(index + 1U);
        parts[index].bytes = &bytes[index];
        parts[index].byte_count = 1U;
        checksum = (uint16_t)(checksum + bytes[index]);
    }
    return checksum;
}

static void test_fixed_source_order_checksum(void)
{
    RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t bytes[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint16_t checksum = make_full_stream(parts, bytes);

    CHECK(redmcsb_f7063_validate_dungeon_stream_checksum_pc34(parts,
                                                               checksum) == 1,
          "F7063 accepts its complete ordered dungeon-part stream");
    CHECK(redmcsb_f7063_validate_dungeon_stream_checksum_pc34(parts,
                                                               (uint16_t)(checksum + 1U)) == 0,
          "F7063 rejects a mismatching trailing checksum word");
}

static void test_zero_sized_thing_pools(void)
{
    RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t bytes[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint16_t checksum = make_full_stream(parts, bytes);
    uint16_t index;

    for (index = REDMCSB_F7063_PART_THING_DATA_0;
         index <= REDMCSB_F7063_PART_THING_DATA_15; ++index) {
        checksum = (uint16_t)(checksum - bytes[index]);
        parts[index].bytes = NULL;
        parts[index].byte_count = 0U;
    }
    CHECK(redmcsb_f7063_validate_dungeon_stream_checksum_pc34(parts,
                                                               checksum) == 1,
          "F7063 preserves zero-byte ThingData pool checksum behavior");
}

static void test_missing_bytes_refused(void)
{
    RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t bytes[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint16_t checksum = make_full_stream(parts, bytes);

    parts[REDMCSB_F7063_PART_RAW_MAP_DATA].bytes = NULL;
    CHECK(redmcsb_f7063_validate_dungeon_stream_checksum_pc34(parts,
                                                               checksum) == 0,
          "F7063 refuses a missing nonempty dungeon part");
}

int main(void)
{
    test_fixed_source_order_checksum();
    test_zero_sized_thing_pools();
    test_missing_bytes_refused();
    CHECK(strcmp(redmcsb_f7063_dungeon_stream_pc34_source_evidence(),
                 "ReDMCSB CEDTINCA.C F7063_LoadDungeon") == 0,
          "source evidence identifies F7063");

    if (failures != 0) {
        return 1;
    }
    puts("PASSED: ReDMCSB F7063 dungeon stream checksum");
    return 0;
}
