#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_dungeon_world_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    TEST_THING = 0x0800,
    TEST_MAP_OFFSET = 80,
    TEST_FIRST_THING_OFFSET = 90,
    TEST_THING_OFFSET = 100
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
    CSB_V1_DungeonData *dungeon;
    uint8_t before[128];

    dungeon = (CSB_V1_DungeonData *)calloc(1, sizeof(*dungeon));
    CHECK(dungeon != NULL);
    if (!dungeon) return 1;
    dungeon->raw_data = (uint8_t *)calloc(1, sizeof(before));
    CHECK(dungeon->raw_data != NULL);
    if (!dungeon->raw_data) {
        free(dungeon);
        return 1;
    }
    dungeon->raw_size = (int)sizeof(before);
    dungeon->level_count = 1;
    dungeon->square_bytes = 1;
    dungeon->level_offsets[0] = TEST_MAP_OFFSET;
    dungeon->level_widths[0] = 2;
    dungeon->level_heights[0] = 1;
    dungeon->square_first_thing_base = TEST_FIRST_THING_OFFSET;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[2] = TEST_THING_OFFSET;
    dungeon->thing_type_counts[2] = 1;

    write_u16(dungeon->raw_data, 60, 0);
    write_u16(dungeon->raw_data, 62, 1);
    dungeon->raw_data[TEST_MAP_OFFSET] = 0x10;
    write_u16(dungeon->raw_data, TEST_FIRST_THING_OFFSET, TEST_THING);
    write_u16(dungeon->raw_data, TEST_THING_OFFSET, CSB_THING_ENDOFLIST);
    memcpy(before, dungeon->raw_data, sizeof(before));

    csb_v1_dungeon_set_current(dungeon);
    csb_v1_dungeon_set_current_level(0);
    CHECK(csb_dungeon_move_thing_default(TEST_THING, 0, 0, 2, 0) == -1);
    CHECK(memcmp(csb_v1_dungeon_get_current()->raw_data,
                 before, sizeof(before)) == 0);

    csb_v1_dungeon_unload();
    if (s_failures != 0) return 1;
    puts("test_csb_v1_f0163_f0164_atomic_link_pc34_compat: PASS");
    return 0;
}
