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

static void set_square(uint8_t *map, int height, int x, int y, uint8_t square)
{
    map[x * height + y] = square;
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    uint8_t map[9];
    int relative_x = -1;
    int relative_y = -1;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(map, 0, sizeof(map));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 3;
    dungeon.level_heights[0] = 3;
    dungeon.level_offsets[0] = 0;
    dungeon.square_bytes = 1;
    dungeon.raw_data = map;
    dungeon.raw_size = (int)sizeof(map);

    set_square(map, 3, 0, 0, 0x20); /* corridor */
    set_square(map, 3, 2, 1, 0x20); /* corridor */
    set_square(map, 3, 1, 0, 0x20); /* corridor */
    set_square(map, 3, 1, 2, 0x20); /* corridor */
    set_square(map, 3, 2, 2, 0x60); /* pit */

    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, 2, 2) == 0x60);
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, -1, 0) == 0x04);
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, 3, 1) == 0x01);
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, 1, -1) == 0x02);
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, 1, 3) == 0x08);
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, -2, 1) == 0x00);
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, 2, 3) == 0x00);

    CHECK(csb_v1_dungeon_f0152_get_relative_square_pc34(
        &dungeon, 0, 1, 2, 1, 0, 0) == 0x20);
    CHECK(csb_v1_dungeon_f0153_get_relative_square_type_pc34(
        &dungeon, 0, 1, 2, 1, 0, 0) == 1);
    CHECK(csb_v1_dungeon_f0150_get_relative_location_pc34(
        1, 2, 1, 0, 0, &relative_x, &relative_y) == 0 &&
          relative_x == 2 && relative_y == 1);
    CHECK(csb_v1_dungeon_f0150_get_relative_location_pc34(
        4, 1, 0, 1, 1, &relative_x, &relative_y) == -1);
    CHECK(csb_v1_dungeon_f0152_get_relative_square_pc34(
        &dungeon, 0, 0, 1, 0, 1, 0) == 0x02);
    CHECK(csb_v1_dungeon_f0152_get_relative_square_pc34(
        &dungeon, 0, 4, 1, 0, 1, 1) == -1);
    dungeon.square_bytes = 2;
    CHECK(csb_v1_dungeon_f0151_get_square_pc34(&dungeon, 0, 0, 0) == -1);

    if (s_failures != 0) return 1;
    puts("test_csb_v1_f0151_f0153_dungeon_relative_square_pc34_compat: PASS");
    return 0;
}
