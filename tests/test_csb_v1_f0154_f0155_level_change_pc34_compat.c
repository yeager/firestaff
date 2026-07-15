#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int s_failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        s_failures++; \
    } \
} while (0)

static void set_square(uint8_t *map, int offset, int height, int x, int y,
                       uint8_t square)
{
    map[offset + x * height + y] = square;
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    uint8_t bytes[32];
    int x;
    int y;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(bytes, 0, sizeof(bytes));
    dungeon.level_count = 2;
    dungeon.square_bytes = 1;
    dungeon.raw_data = bytes;
    dungeon.raw_size = (int)sizeof(bytes);
    dungeon.level_offsets[0] = 0;
    dungeon.level_offsets[1] = 16;
    dungeon.level_widths[0] = 3;
    dungeon.level_widths[1] = 4;
    dungeon.level_heights[0] = dungeon.level_heights[1] = 3;
    dungeon.map_levels[0] = 4;
    dungeon.map_levels[1] = 5;
    dungeon.map_offset_x[0] = 10;
    dungeon.map_offset_y[0] = 20;
    dungeon.map_offset_x[1] = 9;
    dungeon.map_offset_y[1] = 19;

    x = 2;
    y = 1;
    CHECK(csb_v1_dungeon_f0154_get_location_after_level_change_pc34(
        &dungeon, 0, 1, &x, &y) == 1);
    CHECK(x == 3 && y == 2);
    x = 1;
    y = 1;
    CHECK(csb_v1_dungeon_f0154_get_location_after_level_change_pc34(
        &dungeon, 0, 2, &x, &y) == -1);
    CHECK(x == 1 && y == 1);

    set_square(bytes, 0, 3, 1, 1, 0x60); /* bit clear: checks east */
    set_square(bytes, 0, 3, 2, 1, 0x00); /* wall east */
    CHECK(csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
        &dungeon, 0, 1, 1) == 3);
    set_square(bytes, 0, 3, 2, 1, 0x20); /* corridor east */
    CHECK(csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
        &dungeon, 0, 1, 1) == 1);
    set_square(bytes, 0, 3, 1, 1, 0x60 | 0x08); /* bit set: checks north */
    set_square(bytes, 0, 3, 1, 0, 0x00); /* wall north */
    CHECK(csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
        &dungeon, 0, 1, 1) == 2);
    set_square(bytes, 0, 3, 1, 0, 0x20); /* corridor north */
    CHECK(csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
        &dungeon, 0, 1, 1) == 0);
    dungeon.square_bytes = 2;
    CHECK(csb_v1_dungeon_f0155_get_stairs_exit_direction_pc34(
        &dungeon, 0, 1, 1) == -1);

    if (s_failures != 0) return 1;
    puts("test_csb_v1_f0154_f0155_level_change_pc34_compat: PASS");
    return 0;
}
