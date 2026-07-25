#include "dm1_v1_location_after_level_change_f0154_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void test_source_evidence(void)
{
    const char *evidence = DM1_V1_F0154_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != 0);
    assert(strstr(evidence, "F0154_DUNGEON_GetLocationAfterLevelChange") != 0);
    assert(strstr(evidence, "OffsetMapX/Y") != 0);
}

static void test_offset_target_map_selection(void)
{
    const DM1_V1_DungeonMapDescriptorF0154Pc34 maps[] = {
        { 0, 10, 5, 3, 3 },
        { 1, 30, 5, 3, 3 },
        { 1, 11, 4, 5, 5 }
    };
    (void)maps;
    int x = 2;
    (void)x;
    int y = 1;
    (void)y;
    int target = -1;
    (void)target;

    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 3, 0, 1, &x, &y, &target) == 1);
    assert(target == 2);
    assert(x == 1);
    assert(y == 2);
}

static void test_up_and_down_level_delta(void)
{
    const DM1_V1_DungeonMapDescriptorF0154Pc34 maps[] = {
        { 0, 0, 0, 4, 4 },
        { 1, 20, 20, 4, 4 },
        { 1, 0, 0, 4, 4 }
    };
    (void)maps;
    DM1_V1_DungeonLocationAfterLevelChangeF0154Pc34 down;
    DM1_V1_DungeonLocationAfterLevelChangeF0154Pc34 up;

    memset(&down, 0, sizeof(down));
    memset(&up, 0, sizeof(up));

    assert(DM1_V1_Dungeon_GetLocationAfterLevelChangeF0154Pc34Compat(
        maps, 3, 0, 1, 1, 2, &down) == 1);
    assert(down.valid == 1);
    assert(down.sourceLevel == 0);
    assert(down.targetSourceLevel == 1);
    assert(down.targetMapIndex == 2);
    assert(down.mapX == 1);
    assert(down.mapY == 2);
    assert(down.globalX == 1);
    assert(down.globalY == 2);

    assert(DM1_V1_Dungeon_GetLocationAfterLevelChangeF0154Pc34Compat(
        maps, 3, 2, -1, 1, 2, &up) == 1);
    assert(up.valid == 1);
    assert(up.sourceLevel == 1);
    assert(up.targetSourceLevel == 0);
    assert(up.targetMapIndex == 0);
    assert(up.mapX == 1);
    assert(up.mapY == 2);
}

static void test_first_matching_target_wins(void)
{
    const DM1_V1_DungeonMapDescriptorF0154Pc34 maps[] = {
        { 0, 0, 0, 4, 4 },
        { 1, 0, 0, 4, 4 },
        { 1, 0, 0, 4, 4 }
    };
    (void)maps;
    int x = 3;
    (void)x;
    int y = 3;
    (void)y;
    int target = -1;
    (void)target;

    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 3, 0, 1, &x, &y, &target) == 1);
    assert(target == 1);
    assert(x == 3);
    assert(y == 3);
}

static void test_fail_closed_inputs(void)
{
    const DM1_V1_DungeonMapDescriptorF0154Pc34 maps[] = {
        { 0, 0, 0, 4, 4 },
        { 2, 0, 0, 4, 4 },
        { 1, 10, 10, 2, 2 },
        { 1, 0, 0, 0, 4 }
    };
    (void)maps;
    DM1_V1_DungeonLocationAfterLevelChangeF0154Pc34 result;
    int x = 1;
    (void)x;
    int y = 1;
    (void)y;
    int target = 99;
    (void)target;

    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 4, 0, 1, &x, &y, &target) == 0);
    assert(target == -1);
    assert(x == 1);
    assert(y == 1);

    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 4, 0, 1, 0, &y, &target) == 0);
    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 4, 0, 1, &x, 0, &target) == 0);
    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 4, 0, 1, &x, &y, 0) == 0);

    x = -1;
    y = 1;
    assert(F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, 4, 0, 1, &x, &y, &target) == 0);

    memset(&result, 0x7f, sizeof(result));
    assert(DM1_V1_Dungeon_GetLocationAfterLevelChangeF0154Pc34Compat(
        maps, 4, 0, 1, 1, 1, &result) == 0);
    assert(result.valid == 0);
    assert(result.targetMapIndex == -1);
    assert(result.sourceLevel == 0);
    assert(result.targetSourceLevel == 1);
    assert(result.mapX == 1);
    assert(result.mapY == 1);
}

int main(void)
{
    test_source_evidence();
    test_offset_target_map_selection();
    test_up_and_down_level_delta();
    test_first_matching_target_wins();
    test_fail_closed_inputs();
    return 0;
}
