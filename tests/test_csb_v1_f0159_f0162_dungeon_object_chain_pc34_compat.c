#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    TEST_END = 0xfffe,
    TEST_SENSOR = 0x0c00,
    TEST_GROUP = 0x1000,
    TEST_MAP_OFFSET = 80,
    TEST_FIRST_THING_OFFSET = 90,
    TEST_SENSOR_OFFSET = 100,
    TEST_GROUP_OFFSET = 108
};

static int s_failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        s_failures++; \
    } \
} while (0)

static void write_u16(uint8_t *bytes, int offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1] = (uint8_t)(value >> 8);
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    uint8_t bytes[128];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(bytes, 0, sizeof(bytes));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = TEST_MAP_OFFSET;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = TEST_FIRST_THING_OFFSET;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[3] = TEST_SENSOR_OFFSET;
    dungeon.thing_type_counts[3] = 1;
    dungeon.thing_data_bases[4] = TEST_GROUP_OFFSET;
    dungeon.thing_type_counts[4] = 1;
    dungeon.raw_data = bytes;
    dungeon.raw_size = (int)sizeof(bytes);

    /* Real PC34 column-count table begins at 44 + one MAP descriptor. */
    write_u16(bytes, 60, 0);
    bytes[TEST_MAP_OFFSET] = 0x10; /* square has a Thing list */
    write_u16(bytes, TEST_FIRST_THING_OFFSET, TEST_SENSOR);
    write_u16(bytes, TEST_SENSOR_OFFSET, TEST_GROUP);
    write_u16(bytes, TEST_GROUP_OFFSET, TEST_END);

    CHECK(csb_v1_dungeon_f0159_get_next_thing_pc34(
        &dungeon, TEST_SENSOR) == TEST_GROUP);
    CHECK(csb_v1_dungeon_f0162_get_square_first_object_pc34(
        &dungeon, 0, 0, 0) == TEST_GROUP);

    write_u16(bytes, TEST_SENSOR_OFFSET, TEST_END);
    CHECK(csb_v1_dungeon_f0162_get_square_first_object_pc34(
        &dungeon, 0, 0, 0) == TEST_END);
    write_u16(bytes, TEST_SENSOR_OFFSET, TEST_SENSOR);
    CHECK(csb_v1_dungeon_f0162_get_square_first_object_pc34(
        &dungeon, 0, 0, 0) == TEST_END);
    dungeon.square_bytes = 2;
    CHECK(csb_v1_dungeon_f0159_get_next_thing_pc34(
        &dungeon, TEST_SENSOR) == TEST_END);
    CHECK(csb_v1_dungeon_f0162_get_square_first_object_pc34(
        &dungeon, 0, 0, 0) == TEST_END);

    if (s_failures != 0) return 1;
    puts("test_csb_v1_f0159_f0162_dungeon_object_chain_pc34_compat: PASS");
    return 0;
}
