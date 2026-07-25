#include "dm1_v1_current_map_f0173_f0174_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void make_maps(DM1_V1_MapDescriptorF0173F0174Pc34 maps[3])
{
    static const uint8_t map0[] = { 0x20, 0x21, 0x22, 0x23 };
    static const uint8_t map2[] = { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45 };
    static const uint8_t badMap[] = { 0x60 };
    static const uint16_t columns0[] = { 0, 2 };
    static const uint16_t columns2[] = { 0, 1, 4 };
    static const uint16_t doors2[] = { 17, 23 };
    static const uint8_t wall2[] = { 3, 4, 7 };
    static const uint8_t floor2[] = { 1, 8 };
    static const uint8_t alcove2[] = { 2, 5, 9 };

    memset(maps, 0, sizeof(maps[0]) * 3);

    maps[0].mapIndex = 0;
    maps[0].width = 2;
    maps[0].height = 2;
    maps[0].mapBytes = map0;
    maps[0].mapByteCount = sizeof(map0);
    maps[0].columnCumulativeSquareFirstThingCounts = columns0;
    maps[0].columnCount = 2;

    maps[1].mapIndex = 2;
    maps[1].width = 3;
    maps[1].height = 2;
    maps[1].mapBytes = map2;
    maps[1].mapByteCount = sizeof(map2);
    maps[1].columnCumulativeSquareFirstThingCounts = columns2;
    maps[1].columnCount = 3;
    maps[1].doorInfo = doors2;
    maps[1].doorInfoCount = 2;
    maps[1].wallOrnamentIndices = wall2;
    maps[1].wallOrnamentCount = 3;
    maps[1].floorOrnamentIndices = floor2;
    maps[1].floorOrnamentCount = 2;
    maps[1].alcoveOrnamentIndices = alcove2;
    maps[1].alcoveOrnamentCount = 3;

    maps[2].mapIndex = 4;
    maps[2].width = 2;
    maps[2].height = 2;
    maps[2].mapBytes = badMap;
    maps[2].mapByteCount = sizeof(badMap);
    maps[2].columnCumulativeSquareFirstThingCounts = columns0;
    maps[2].columnCount = 2;
}

static void test_f0173_sets_current_map_only(void)
{
    DM1_V1_MapDescriptorF0173F0174Pc34 maps[3];
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 runtime;
    DM1_V1_CurrentMapResultF0173F0174Pc34 result;
    (void)result;

    make_maps(maps);
    DM1_V1_CurrentMap_InitF0173F0174Pc34Compat(&runtime);
    runtime.partyMapIndex = 9;

    assert(F0173_DUNGEON_SetCurrentMap(&runtime, maps, 3, 2, &result) == 1);
    assert(result.valid == 1);
    assert(result.previousCurrentMapIndex == -1);
    assert(result.currentMapIndex == 2);
    assert(result.currentMapWidth == 3);
    assert(result.currentMapHeight == 2);
    assert(result.updatedCurrentMap == 1);
    assert(result.updatedPartyMap == 0);

    assert(runtime.valid == 1);
    assert(runtime.currentMapIndex == 2);
    assert(runtime.currentMapWidth == 3);
    assert(runtime.currentMapHeight == 2);
    assert(runtime.currentMapBytes == maps[1].mapBytes);
    assert(runtime.currentMapByteCount == maps[1].mapByteCount);
    assert(runtime.currentMapColumnCumulativeSquareFirstThingCounts ==
           maps[1].columnCumulativeSquareFirstThingCounts);
    assert(runtime.currentMapColumnCount == 3);
    assert(runtime.currentMapDoorInfo == maps[1].doorInfo);
    assert(runtime.currentMapWallOrnamentIndices == maps[1].wallOrnamentIndices);
    assert(runtime.currentMapFloorOrnamentIndices ==
           maps[1].floorOrnamentIndices);
    assert(runtime.currentMapAlcoveOrnamentIndices ==
           maps[1].alcoveOrnamentIndices);
    assert(runtime.partyMapIndex == 9);
    assert(runtime.f0173CallCount == 1);
    assert(runtime.f0174CallCount == 0);
}

static void test_f0174_sets_current_and_party_map(void)
{
    DM1_V1_MapDescriptorF0173F0174Pc34 maps[3];
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 runtime;
    DM1_V1_CurrentMapResultF0173F0174Pc34 result;
    (void)result;

    make_maps(maps);
    DM1_V1_CurrentMap_InitF0173F0174Pc34Compat(&runtime);
    assert(DM1_V1_Dungeon_SetCurrentMapF0173Pc34Compat(
               &runtime, maps, 3, 0, &result) == 1);
    assert(runtime.currentMapIndex == 0);

    assert(F0174_DUNGEON_SetCurrentMapAndPartyMap(
               &runtime, maps, 3, 2, &result) == 1);
    assert(result.valid == 1);
    assert(result.previousCurrentMapIndex == 0);
    assert(result.currentMapIndex == 2);
    assert(result.updatedCurrentMap == 1);
    assert(result.updatedPartyMap == 1);

    assert(runtime.currentMapIndex == 2);
    assert(runtime.partyMapIndex == 2);
    assert(runtime.partyMapWidth == 3);
    assert(runtime.partyMapHeight == 2);
    assert(runtime.partyMapBytes == maps[1].mapBytes);
    assert(runtime.partyMapColumnCumulativeSquareFirstThingCounts ==
           maps[1].columnCumulativeSquareFirstThingCounts);
    assert(runtime.f0173CallCount == 2);
    assert(runtime.f0174CallCount == 1);

    assert(DM1_V1_Dungeon_SetCurrentMapAndPartyMapF0174Pc34Compat(
               &runtime, maps, 3, 0, &result) == 1);
    assert(runtime.currentMapIndex == 0);
    assert(runtime.partyMapIndex == 0);
    assert(runtime.f0173CallCount == 3);
    assert(runtime.f0174CallCount == 2);
}

static void test_fail_closed_without_state_mutation(void)
{
    DM1_V1_MapDescriptorF0173F0174Pc34 maps[3];
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 runtime;
    DM1_V1_CurrentMapResultF0173F0174Pc34 result;
    (void)result;

    make_maps(maps);
    DM1_V1_CurrentMap_InitF0173F0174Pc34Compat(&runtime);
    assert(F0174_DUNGEON_SetCurrentMapAndPartyMap(
               &runtime, maps, 3, 2, &result) == 1);

    assert(F0173_DUNGEON_SetCurrentMap(&runtime, maps, 3, 7, &result) == 0);
    assert(result.valid == 0);
    assert(runtime.currentMapIndex == 2);
    assert(runtime.partyMapIndex == 2);
    assert(runtime.f0173CallCount == 1);
    assert(runtime.f0174CallCount == 1);

    assert(F0174_DUNGEON_SetCurrentMapAndPartyMap(
               &runtime, maps, 3, 4, &result) == 0);
    assert(runtime.currentMapIndex == 2);
    assert(runtime.partyMapIndex == 2);
    assert(runtime.f0173CallCount == 1);
    assert(runtime.f0174CallCount == 1);

    assert(F0173_DUNGEON_SetCurrentMap(0, maps, 3, 0, &result) == 0);
    assert(F0174_DUNGEON_SetCurrentMapAndPartyMap(0, maps, 3, 0, 0) == 0);
}

int main(void)
{
    const char *evidence = DM1_V1_F0173_F0174_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "F0173_DUNGEON_SetCurrentMap") != 0);
    assert(strstr(evidence, "F0174_DUNGEON_SetCurrentMapAndPartyMap") != 0);
    assert(strstr(evidence, "G0309_i_PartyMapIndex") != 0);

    test_f0173_sets_current_map_only();
    test_f0174_sets_current_and_party_map();
    test_fail_closed_without_state_mutation();

    return 0;
}
