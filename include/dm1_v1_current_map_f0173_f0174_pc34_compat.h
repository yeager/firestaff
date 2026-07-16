#ifndef FIRESTAFF_DM1_V1_CURRENT_MAP_F0173_F0174_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CURRENT_MAP_F0173_F0174_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int mapIndex;
    int width;
    int height;
    const uint8_t *mapBytes;
    size_t mapByteCount;
    const uint16_t *columnCumulativeSquareFirstThingCounts;
    size_t columnCount;
    const uint16_t *doorInfo;
    size_t doorInfoCount;
    const uint8_t *wallOrnamentIndices;
    size_t wallOrnamentCount;
    const uint8_t *floorOrnamentIndices;
    size_t floorOrnamentCount;
    const uint8_t *alcoveOrnamentIndices;
    size_t alcoveOrnamentCount;
} DM1_V1_MapDescriptorF0173F0174Pc34;

typedef struct {
    int valid;
    int currentMapIndex;
    int currentMapWidth;
    int currentMapHeight;
    const uint8_t *currentMapBytes;
    size_t currentMapByteCount;
    const uint16_t *currentMapColumnCumulativeSquareFirstThingCounts;
    size_t currentMapColumnCount;
    const uint16_t *currentMapDoorInfo;
    size_t currentMapDoorInfoCount;
    const uint8_t *currentMapWallOrnamentIndices;
    size_t currentMapWallOrnamentCount;
    const uint8_t *currentMapFloorOrnamentIndices;
    size_t currentMapFloorOrnamentCount;
    const uint8_t *currentMapAlcoveOrnamentIndices;
    size_t currentMapAlcoveOrnamentCount;

    int partyMapIndex;
    int partyMapWidth;
    int partyMapHeight;
    const uint8_t *partyMapBytes;
    size_t partyMapByteCount;
    const uint16_t *partyMapColumnCumulativeSquareFirstThingCounts;
    size_t partyMapColumnCount;

    int f0173CallCount;
    int f0174CallCount;
} DM1_V1_CurrentMapRuntimeF0173F0174Pc34;

typedef struct {
    int valid;
    int previousCurrentMapIndex;
    int currentMapIndex;
    int currentMapWidth;
    int currentMapHeight;
    int updatedCurrentMap;
    int updatedPartyMap;
} DM1_V1_CurrentMapResultF0173F0174Pc34;

const char *DM1_V1_F0173_F0174_SourceEvidencePc34(void);

void DM1_V1_CurrentMap_InitF0173F0174Pc34Compat(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime);

int F0173_DUNGEON_SetCurrentMap(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out);

int F0174_DUNGEON_SetCurrentMapAndPartyMap(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out);

int DM1_V1_Dungeon_SetCurrentMapF0173Pc34Compat(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out);

int DM1_V1_Dungeon_SetCurrentMapAndPartyMapF0174Pc34Compat(
    DM1_V1_CurrentMapRuntimeF0173F0174Pc34 *runtime,
    const DM1_V1_MapDescriptorF0173F0174Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    DM1_V1_CurrentMapResultF0173F0174Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
