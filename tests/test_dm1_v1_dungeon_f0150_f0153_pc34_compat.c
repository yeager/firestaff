#include "dm1_v1_dungeon_square_structs_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAP_W 4
#define MAP_H 3

static int g_failures = 0;

static uint8_t sqb(int element, int flags)
{
    return (uint8_t)(((element & 7) << 5) | (flags & 0x1f));
}

static void check_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        ++g_failures;
    }
}

static void set_square(uint8_t map[MAP_W * MAP_H],
                       int x,
                       int y,
                       uint8_t value)
{
    map[(x * MAP_H) + y] = value;
}

static void test_f0150_coordinates(void)
{
    static const struct {
        int dir;
        int forward;
        int right;
        int want_x;
        int want_y;
    } cases[] = {
        { DM1_DIR_NORTH, 2,  1, 6, 3 },
        { DM1_DIR_EAST,  2,  1, 7, 6 },
        { DM1_DIR_SOUTH, 2,  1, 4, 7 },
        { DM1_DIR_WEST,  2,  1, 3, 4 },
        { DM1_DIR_NORTH, 3, -1, 4, 2 },
        { DM1_DIR_WEST, -1, 2, 6, 3 }
    };
    size_t i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_DungeonF0150CoordinatesPc34 coords;
        char label[64];
        memset(&coords, 0, sizeof(coords));
        check_int("f0150 returns valid",
                  dm1_v1_dungeon_f0150_update_map_coordinates_after_relative_movement_pc34(
                      5, 5, cases[i].dir, cases[i].forward, cases[i].right,
                      &coords),
                  1);
        snprintf(label, sizeof(label), "f0150[%lu].x", (unsigned long)i);
        check_int(label, coords.x, cases[i].want_x);
        snprintf(label, sizeof(label), "f0150[%lu].y", (unsigned long)i);
        check_int(label, coords.y, cases[i].want_y);
    }
}

static void test_f0151_boundary_square(void)
{
    uint8_t map[MAP_W * MAP_H];
    memset(map, sqb(DM1_ELEMENT_WALL, 0), sizeof(map));
    set_square(map, 1, 1, sqb(DM1_ELEMENT_TELEPORTER, DM1_TELEPORTER_OPEN));
    set_square(map, 0, 1, sqb(DM1_ELEMENT_CORRIDOR, 0));
    set_square(map, MAP_W - 1, 1, sqb(DM1_ELEMENT_PIT, DM1_PIT_OPEN));
    set_square(map, 2, 0, sqb(DM1_ELEMENT_CORRIDOR, 0));
    set_square(map, 2, MAP_H - 1, sqb(DM1_ELEMENT_PIT, DM1_PIT_OPEN));

    check_int("f0151 in-bounds column-major",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H, 1, 1),
              sqb(DM1_ELEMENT_TELEPORTER, DM1_TELEPORTER_OPEN));
    check_int("f0151 west edge",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H, -1, 1),
              sqb(DM1_ELEMENT_WALL, DM1_WALL_EAST_RANDOM_ORN));
    check_int("f0151 east edge",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H,
                                                    MAP_W, 1),
              sqb(DM1_ELEMENT_WALL, DM1_WALL_WEST_RANDOM_ORN));
    check_int("f0151 north edge",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H, 2, -1),
              sqb(DM1_ELEMENT_WALL, DM1_WALL_SOUTH_RANDOM_ORN));
    check_int("f0151 south edge",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H,
                                                    2, MAP_H),
              sqb(DM1_ELEMENT_WALL, DM1_WALL_NORTH_RANDOM_ORN));
    check_int("f0151 diagonal is plain wall",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H, -1, -1),
              sqb(DM1_ELEMENT_WALL, 0));

    set_square(map, 0, 1, sqb(DM1_ELEMENT_WALL, 0));
    check_int("f0151 wall edge suppresses random ornament",
              dm1_v1_dungeon_f0151_get_square_pc34(map, MAP_W, MAP_H, -1, 1),
              sqb(DM1_ELEMENT_WALL, 0));
}

static void test_f0152_f0153_relative_square(void)
{
    uint8_t map[MAP_W * MAP_H];
    memset(map, sqb(DM1_ELEMENT_CORRIDOR, 0), sizeof(map));
    set_square(map, 2, 0, sqb(DM1_ELEMENT_DOOR, DM1_DOOR_NS_ORIENTATION));
    set_square(map, 3, 1, sqb(DM1_ELEMENT_PIT, DM1_PIT_OPEN));
    set_square(map, 0, 1, sqb(DM1_ELEMENT_STAIRS, DM1_STAIRS_UP));

    check_int("f0152 north relative door",
              dm1_v1_dungeon_f0152_get_relative_square_pc34(
                  map, MAP_W, MAP_H, 1, 1, DM1_DIR_NORTH, 1, 1),
              sqb(DM1_ELEMENT_DOOR, DM1_DOOR_NS_ORIENTATION));
    check_int("f0153 east relative pit type",
              dm1_v1_dungeon_f0153_get_relative_square_type_pc34(
                  map, MAP_W, MAP_H, 1, 1, DM1_DIR_EAST, 2, 0),
              DM1_ELEMENT_PIT);
    check_int("f0153 west relative stairs type",
              dm1_v1_dungeon_f0153_get_relative_square_type_pc34(
                  map, MAP_W, MAP_H, 1, 1, DM1_DIR_WEST, 1, 0),
              DM1_ELEMENT_STAIRS);
    check_int("f0152 south boundary wall",
              dm1_v1_dungeon_f0152_get_relative_square_pc34(
                  map, MAP_W, MAP_H, 2, 1, DM1_DIR_SOUTH, 2, 0),
              sqb(DM1_ELEMENT_WALL, DM1_WALL_NORTH_RANDOM_ORN));
}

int main(void)
{
    test_f0150_coordinates();
    test_f0151_boundary_square();
    test_f0152_f0153_relative_square();

    if (g_failures != 0) {
        fprintf(stderr, "FAILURES=%d\n", g_failures);
        return 1;
    }
    puts("DM1 F0150-F0153 dungeon square contract passed");
    return 0;
}
