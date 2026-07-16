#include "dm1_v1_current_map_f0173_f0174_pc34_compat.h"

#include <string.h>

const char *DM1_V1_F0173_F0174_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:2724 F0173_DUNGEON_SetCurrentMap selects "
        "G0271_i_CurrentMapIndex and current map metadata\n"
        "DUNGEON.C:2728-2740 refreshes current map width, height, raw map "
        "pointer, column cumulative SquareFirstThing counts, door info, and "
        "ornament tables\n"
        "DUNGEON.C:2742 F0174_DUNGEON_SetCurrentMapAndPartyMap calls F0173 "
        "then updates G0309_i_PartyMapIndex and party-map cached map data\n"
        "MOVESENS.C F0267 uses F0173 for level-change target inspection and "
        "GAMELOOP.C processes deferred party-map changes through F0174";
}

static void clear_result(DM1_V1_CurrentMapResultF0173F0174Pc34 *out)
{
    if (out) {
        memset(out, 0, sizeof(*out));
        out->previousCurrentMapIndex = -1;
        out->currentMapIndex = -1;
    }
}

void DM1_V1_CurrentMap_InitF0173F0174Pc34Compat(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime)
{
    if (!runtime) {
        return;
    }
    memset(runtime, 0, sizeof(*runtime));
    runtime->currentMapIndex = -1;
    runtime->partyMapIndex = -1;
}

static const DM1_V1_MapDescriptorF0173F0174Pc34 *find_map(
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex)
{
    size_t i;

    if (!maps || mapIndex < 0) {
        return 0;
    }
    for (i = 0; i < mapCount; ++i) {
        if (maps[i].mapIndex == mapIndex) {
            return &maps[i];
        }
    }
    return 0;
}

static int descriptor_valid(const DM1_V1_MapDescriptorF0173F0174Pc34 *map)
{
    if (!map || map->mapIndex < 0 || map->width <= 0 || map->height <= 0 ||
        !map->mapBytes || !map->columnCumulativeSquareFirstThingCounts) {
        return 0;
    }
    if (map->mapByteCount < (size_t)(map->width * map->height) ||
        map->columnCount < (size_t)map->width) {
        return 0;
    }
    return 1;
}

static void publish_current_map(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *map)
{
    runtime->valid = 1;
    runtime->currentMapIndex = map->mapIndex;
    runtime->currentMapWidth = map->width;
    runtime->currentMapHeight = map->height;
    runtime->currentMapBytes = map->mapBytes;
    runtime->currentMapByteCount = map->mapByteCount;
    runtime->currentMapColumnCumulativeSquareFirstThingCounts =
        map->columnCumulativeSquareFirstThingCounts;
    runtime->currentMapColumnCount = map->columnCount;
    runtime->currentMapDoorInfo = map->doorInfo;
    runtime->currentMapDoorInfoCount = map->doorInfoCount;
    runtime->currentMapWallOrnamentIndices = map->wallOrnamentIndices;
    runtime->currentMapWallOrnamentCount = map->wallOrnamentCount;
    runtime->currentMapFloorOrnamentIndices = map->floorOrnamentIndices;
    runtime->currentMapFloorOrnamentCount = map->floorOrnamentCount;
    runtime->currentMapAlcoveOrnamentIndices = map->alcoveOrnamentIndices;
    runtime->currentMapAlcoveOrnamentCount = map->alcoveOrnamentCount;
}

static void publish_party_map(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *map)
{
    runtime->partyMapIndex = map->mapIndex;
    runtime->partyMapWidth = map->width;
    runtime->partyMapHeight = map->height;
    runtime->partyMapBytes = map->mapBytes;
    runtime->partyMapByteCount = map->mapByteCount;
    runtime->partyMapColumnCumulativeSquareFirstThingCounts =
        map->columnCumulativeSquareFirstThingCounts;
    runtime->partyMapColumnCount = map->columnCount;
}

int F0173_DUNGEON_SetCurrentMap(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out)
{
    const DM1_V1_MapDescriptorF0173F0174Pc34 *map;
    int previousMapIndex;

    clear_result(out);
    if (!runtime) {
        return 0;
    }
    previousMapIndex = runtime->currentMapIndex;
    map = find_map(maps, mapCount, mapIndex);
    if (!descriptor_valid(map)) {
        return 0;
    }

    publish_current_map(runtime, map);
    runtime->f0173CallCount += 1;

    if (out) {
        out->valid = 1;
        out->previousCurrentMapIndex = previousMapIndex;
        out->currentMapIndex = runtime->currentMapIndex;
        out->currentMapWidth = runtime->currentMapWidth;
        out->currentMapHeight = runtime->currentMapHeight;
        out->updatedCurrentMap = 1;
        out->updatedPartyMap = 0;
    }
    return 1;
}

int F0174_DUNGEON_SetCurrentMapAndPartyMap(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out)
{
    const DM1_V1_MapDescriptorF0173F0174Pc34 *map;

    if (!F0173_DUNGEON_SetCurrentMap(runtime, maps, mapCount, mapIndex, out)) {
        return 0;
    }

    map = find_map(maps, mapCount, mapIndex);
    publish_party_map(runtime, map);
    runtime->f0174CallCount += 1;
    if (out) {
        out->updatedPartyMap = 1;
    }
    return 1;
}

int DM1_V1_Dungeon_SetCurrentMapF0173Pc34Compat(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out)
{
    return F0173_DUNGEON_SetCurrentMap(
        runtime,
        maps,
        mapCount,
        mapIndex,
        out);
}

int DM1_V1_Dungeon_SetCurrentMapAndPartyMapF0174Pc34Compat(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out)
{
    return F0174_DUNGEON_SetCurrentMapAndPartyMap(
        runtime,
        maps,
        mapCount,
        mapIndex,
        out);
}
