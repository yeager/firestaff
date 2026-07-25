#include "dm1_v1_random_ornament_f0169_f0170_f0171_pc34_compat.h"

#include "dm1_v1_dungeon_square_structs_pc34_compat.h"

#include <assert.h>
#include <string.h>

static uint8_t sqb(int element, int flags)
{
    return (uint8_t)(((element & 7) << 5) | (flags & 0x1f));
}

static void test_f0169_index(void)
{
    static const uint8_t enabled[] = { 0, 1, 0, 1, 1, 0 };
    (void)enabled;
    DM1_V1_RandomOrnamentIndexF0169Pc34 out;
    (void)out;

    assert(F0169_DUNGEON_GetRandomOrnamentIndex(enabled, 6, 0, &out) == 1);
    assert(out.valid == 1);
    assert(out.ornamentIndex == 1);
    assert(out.candidateOrdinal == 0);
    assert(out.admittedCandidateCount == 3);

    assert(DM1_V1_Dungeon_GetRandomOrnamentIndexF0169Pc34Compat(
               enabled, 6, 1, &out) == 1);
    assert(out.ornamentIndex == 3);
    assert(out.candidateOrdinal == 1);

    assert(F0169_DUNGEON_GetRandomOrnamentIndex(enabled, 6, 5, &out) == 1);
    assert(out.ornamentIndex == 4);
    assert(out.candidateOrdinal == 2);

    assert(F0169_DUNGEON_GetRandomOrnamentIndex(0, 6, 0, &out) == 0);
    assert(out.valid == 0);
    assert(out.ornamentIndex == -1);
}

static void test_f0170_ordinal(void)
{
    static const uint8_t enabled[] = { 0, 1, 0, 1, 1, 0 };
    (void)enabled;
    static const uint8_t none[] = { 0, 0, 0 };
    (void)none;
    DM1_V1_RandomOrnamentOrdinalF0170Pc34 out;
    (void)out;

    assert(F0170_DUNGEON_GetRandomOrnamentOrdinal(enabled, 6, 0, &out) == 1);
    assert(out.valid == 1);
    assert(out.ornamentIndex == 1);
    assert(out.ornamentOrdinal == 2);
    assert(out.admittedCandidateCount == 3);

    assert(DM1_V1_Dungeon_GetRandomOrnamentOrdinalF0170Pc34Compat(
               enabled, 6, 2, &out) == 1);
    assert(out.ornamentIndex == 4);
    assert(out.ornamentOrdinal == 5);

    assert(F0170_DUNGEON_GetRandomOrnamentOrdinal(none, 3, 0, &out) == 0);
    assert(out.valid == 0);
    assert(out.ornamentIndex == -1);
}

static void test_f0171_wall_relative_ordinals(void)
{
    static const uint8_t wallEnabled[] = { 0, 1, 0, 1, 1, 0 };
    static const uint8_t floorEnabled[] = { 1, 0, 1 };
    static const uint16_t randomValues[] = { 0, 1, 2, 3 };
    const DM1_V1_RandomOrnamentCatalogF0169F0170Pc34 catalog = {
        wallEnabled, 6, floorEnabled, 3
    };
    DM1_V1_SetSquareAspectRandomOrnamentsF0171InputPc34 input;
    DM1_V1_SetSquareAspectRandomOrnamentsF0171ResultPc34 out;
    (void)out;

    memset(&input, 0, sizeof(input));
    input.catalog = &catalog;
    input.direction = DM1_DIR_NORTH;
    input.rawSquare = sqb(DM1_ELEMENT_WALL,
                          DM1_WALL_EAST_RANDOM_ORN |
                          DM1_WALL_NORTH_RANDOM_ORN |
                          DM1_WALL_WEST_RANDOM_ORN);
    input.randomValues = randomValues;
    input.randomValueCount = 4;

    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, &out) == 1);
    assert(out.valid == 1);
    assert(out.aspect[DM1_SQA_ELEMENT] == DM1_ELEMENT_WALL);
    assert(out.randomCalls == 3);
    assert(out.wroteRightWallOrnament == 1);
    assert(out.wroteFrontWallOrnament == 1);
    assert(out.wroteLeftWallOrnament == 1);
    assert(out.aspect[DM1_SQA_RIGHT_WALL_ORN_ORD] == 2);
    assert(out.aspect[DM1_SQA_FRONT_WALL_ORN_ORD] == 4);
    assert(out.aspect[DM1_SQA_LEFT_WALL_ORN_ORD] == 5);
    assert(out.aspect[DM1_SQA_FLOOR_ORN_ORDINAL] == 5);

    input.direction = DM1_DIR_EAST;
    input.rawSquare = sqb(DM1_ELEMENT_WALL,
                          DM1_WALL_SOUTH_RANDOM_ORN |
                          DM1_WALL_EAST_RANDOM_ORN);
    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, &out) == 1);
    assert(out.randomCalls == 2);
    assert(out.wroteRightWallOrnament == 1);
    assert(out.wroteFrontWallOrnament == 1);
    assert(out.wroteLeftWallOrnament == 0);
    assert(out.aspect[DM1_SQA_RIGHT_WALL_ORN_ORD] == 2);
    assert(out.aspect[DM1_SQA_FRONT_WALL_ORN_ORD] == 4);
    assert(out.aspect[DM1_SQA_LEFT_WALL_ORN_ORD] == 0);
}

static void test_f0171_floor_and_fail_closed(void)
{
    static const uint8_t wallEnabled[] = { 0, 1, 0, 1, 1, 0 };
    static const uint8_t floorEnabled[] = { 1, 0, 1 };
    static const uint8_t floorDisabled[] = { 0, 0, 0 };
    static const uint16_t randomValues[] = { 1, 0, 0, 0 };
    DM1_V1_RandomOrnamentCatalogF0169F0170Pc34 catalog = {
        wallEnabled, 6, floorEnabled, 3
    };
    DM1_V1_SetSquareAspectRandomOrnamentsF0171InputPc34 input;
    DM1_V1_SetSquareAspectRandomOrnamentsF0171ResultPc34 out;
    (void)out;

    memset(&input, 0, sizeof(input));
    input.catalog = &catalog;
    input.direction = DM1_DIR_SOUTH;
    input.rawSquare = sqb(DM1_ELEMENT_CORRIDOR, DM1_CORRIDOR_RANDOM_ORN);
    input.randomValues = randomValues;
    input.randomValueCount = 4;

    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, &out) == 1);
    assert(out.aspect[DM1_SQA_ELEMENT] == DM1_ELEMENT_CORRIDOR);
    assert(out.randomCalls == 1);
    assert(out.wroteFloorOrnament == 1);
    assert(out.aspect[DM1_SQA_FLOOR_ORN_ORDINAL] == 3);

    input.rawSquare = sqb(DM1_ELEMENT_CORRIDOR, 0);
    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, &out) == 1);
    assert(out.randomCalls == 0);
    assert(out.wroteFloorOrnament == 0);
    assert(out.aspect[DM1_SQA_FLOOR_ORN_ORDINAL] == 0);

    catalog.floorOrnamentEnabledByIndex = floorDisabled;
    input.rawSquare = sqb(DM1_ELEMENT_CORRIDOR, DM1_CORRIDOR_RANDOM_ORN);
    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, &out) == 1);
    assert(out.randomCalls == 1);
    assert(out.wroteFloorOrnament == 0);
    assert(out.aspect[DM1_SQA_FLOOR_ORN_ORDINAL] == 0);

    input.randomValueCount = 3;
    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, &out) == 0);
    assert(out.valid == 0);

    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               0, &out) == 0);
    assert(F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals(
               &input, 0) == 0);
}

int main(void)
{
    const char *evidence = DM1_V1_F0169_F0170_F0171_SourceEvidencePc34();
    (void)evidence;
    assert(strstr(evidence, "F0169_DUNGEON_GetRandomOrnamentIndex") != 0);
    assert(strstr(evidence, "F0170_DUNGEON_GetRandomOrnamentOrdinal") != 0);
    assert(strstr(evidence,
                  "F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals") !=
           0);

    test_f0169_index();
    test_f0170_ordinal();
    test_f0171_wall_relative_ordinals();
    test_f0171_floor_and_fail_closed();

    return 0;
}
