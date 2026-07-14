#include "redmcsb_cedtinc8_dungeon_write_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); failures++; } \
} while (0)

static uint16_t make_parts(RedmcsbF7063DungeonPart *parts, uint8_t *bytes)
{
    uint16_t checksum = 0;
    uint16_t index;

    for (index = 0; index < REDMCSB_F7063_DUNGEON_PART_COUNT; ++index) {
        bytes[index] = (uint8_t)(0x20U + index);
        parts[index].bytes = &bytes[index];
        parts[index].byte_count = 1U;
        checksum = (uint16_t)(checksum + bytes[index]);
    }
    return checksum;
}

static void test_fixed_stream_write(void)
{
    RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t parts_bytes[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t written[REDMCSB_F7063_DUNGEON_PART_COUNT + 2U];
    uint16_t checksum = make_parts(parts, parts_bytes);
    size_t written_size = 0;

    CHECK(redmcsb_cedtinc8_write_dungeon_stream_pc34(
              parts, written, sizeof(written), &written_size) == 1,
          "CEDTINC8 writes all 22 F7060 dungeon parts");
    CHECK(written_size == sizeof(written), "written size includes checksum word");
    CHECK(memcmp(written, parts_bytes, sizeof(parts_bytes)) == 0,
          "parts retain exact source order in the output stream");
    CHECK(written[sizeof(parts_bytes)] == (uint8_t)checksum &&
              written[sizeof(parts_bytes) + 1U] == (uint8_t)(checksum >> 8),
          "output ends with the PC34 little-endian dungeon checksum");
}

static void test_no_partial_output(void)
{
    RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t parts_bytes[REDMCSB_F7063_DUNGEON_PART_COUNT];
    uint8_t written[REDMCSB_F7063_DUNGEON_PART_COUNT + 2U];
    uint8_t before[sizeof(written)];
    size_t written_size = 99U;

    (void)make_parts(parts, parts_bytes);
    memset(written, 0xa5, sizeof(written));
    memcpy(before, written, sizeof(before));
    CHECK(redmcsb_cedtinc8_write_dungeon_stream_pc34(
              parts, written, sizeof(written) - 1U, &written_size) == 0,
          "undersized output refuses the complete dungeon stream");
    CHECK(memcmp(written, before, sizeof(written)) == 0 && written_size == 99U,
          "undersized output produces no partial stream or fallback size");
}

int main(void)
{
    test_fixed_stream_write();
    test_no_partial_output();
    CHECK(strcmp(redmcsb_cedtinc8_dungeon_write_pc34_source_evidence(),
                 "ReDMCSB CEDTINC8.C SAVE_GAME and CEDTINC6.C F7060") == 0,
          "source evidence identifies the write sequence");
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB CEDTINC8 dungeon write stream");
    return 0;
}
