#include "dm1_v1_stairs_exit_direction_f0155_pc34_compat.h"

#include <assert.h>
#include <string.h>

#define MAP_W 4
#define MAP_H 4

static uint8_t sqb(int element, int flags)
{
    return (uint8_t)(((element & 7) << 5) | (flags & 0x1f));
}

static void set_square(uint8_t map[MAP_W * MAP_H],
                       int x,
                       int y,
                       uint8_t raw)
{
    map[(x * MAP_H) + y] = raw;
}

static void fill_corridor(uint8_t map[MAP_W * MAP_H])
{
    int i;

    for (i = 0; i < MAP_W * MAP_H; ++i) {
        map[i] = sqb(1, 0);
    }
}

static void test_source_evidence(void)
{
    const char *evidence = DM1_V1_F0155_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != 0);
    assert(strstr(evidence, "F0155_DUNGEON_GetStairsExitDirection") != 0);
    assert(strstr(evidence, "MASK0x0008_STAIRS_NS_ORIENTATION") != 0);
}

static void test_east_west_oriented_stairs(void)
{
    uint8_t map[MAP_W * MAP_H];
    DM1_V1_StairsExitDirectionF0155Pc34 result;
    (void)result;

    fill_corridor(map);
    set_square(map, 1, 1, sqb(DM1_V1_F0155_ELEMENT_STAIRS_PC34, 0));
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 1, 1, &result) == 1);
    assert(result.valid == 1);
    assert(result.northSouthOrientedStairs == 0);
    assert(result.checkedMapX == 2);
    assert(result.checkedMapY == 1);
    assert(result.checkedSquareBlocked == 0);
    assert(result.exitDirection == DM1_V1_F0155_DIR_EAST_PC34);
    assert(F0155_DUNGEON_GetStairsExitDirection(map, MAP_W, MAP_H, 1, 1) ==
           DM1_V1_F0155_DIR_EAST_PC34);

    set_square(map, 2, 1, sqb(DM1_V1_F0155_ELEMENT_WALL_PC34, 0));
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 1, 1, &result) == 1);
    assert(result.checkedSquareBlocked == 1);
    assert(result.exitDirection == DM1_V1_F0155_DIR_WEST_PC34);

    set_square(map, 2, 1, sqb(DM1_V1_F0155_ELEMENT_STAIRS_PC34, 0));
    assert(F0155_DUNGEON_GetStairsExitDirection(map, MAP_W, MAP_H, 1, 1) ==
           DM1_V1_F0155_DIR_WEST_PC34);
}

static void test_north_south_oriented_stairs(void)
{
    uint8_t map[MAP_W * MAP_H];
    DM1_V1_StairsExitDirectionF0155Pc34 result;
    (void)result;

    fill_corridor(map);
    set_square(map, 1, 1,
               sqb(DM1_V1_F0155_ELEMENT_STAIRS_PC34,
                   DM1_V1_F0155_STAIRS_NS_ORIENTATION_MASK_PC34));
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 1, 1, &result) == 1);
    assert(result.valid == 1);
    assert(result.northSouthOrientedStairs == 1);
    assert(result.checkedMapX == 1);
    assert(result.checkedMapY == 0);
    assert(result.checkedSquareBlocked == 0);
    assert(result.exitDirection == DM1_V1_F0155_DIR_NORTH_PC34);
    assert(F0155_DUNGEON_GetStairsExitDirection(map, MAP_W, MAP_H, 1, 1) ==
           DM1_V1_F0155_DIR_NORTH_PC34);

    set_square(map, 1, 0, sqb(DM1_V1_F0155_ELEMENT_WALL_PC34, 0));
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 1, 1, &result) == 1);
    assert(result.checkedSquareBlocked == 1);
    assert(result.exitDirection == DM1_V1_F0155_DIR_SOUTH_PC34);
}

static void test_boundary_and_invalid_inputs(void)
{
    uint8_t map[MAP_W * MAP_H];
    DM1_V1_StairsExitDirectionF0155Pc34 result;

    fill_corridor(map);
    set_square(map, 0, 0,
               sqb(DM1_V1_F0155_ELEMENT_STAIRS_PC34,
                   DM1_V1_F0155_STAIRS_NS_ORIENTATION_MASK_PC34));
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 0, 0, &result) == 1);
    assert(result.checkedMapX == 0);
    assert(result.checkedMapY == -1);
    assert(result.checkedSquareType == DM1_V1_F0155_ELEMENT_WALL_PC34);
    assert(result.checkedSquareBlocked == 1);
    assert(result.exitDirection == DM1_V1_F0155_DIR_SOUTH_PC34);

    assert(F0155_DUNGEON_GetStairsExitDirection(map, MAP_W, MAP_H, -1, 0) ==
           -1);
    assert(F0155_DUNGEON_GetStairsExitDirection(0, MAP_W, MAP_H, 0, 0) ==
           -1);
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 0, 0, 0) == 0);

    set_square(map, 1, 1, sqb(1, 0));
    memset(&result, 0x7f, sizeof(result));
    assert(DM1_V1_Dungeon_GetStairsExitDirectionF0155Pc34Compat(
        map, MAP_W, MAP_H, 1, 1, &result) == 0);
    assert(result.valid == 0);
    assert(result.exitDirection == -1);
}

int main(void)
{
    test_source_evidence();
    test_east_west_oriented_stairs();
    test_north_south_oriented_stairs();
    test_boundary_and_invalid_inputs();
    return 0;
}
