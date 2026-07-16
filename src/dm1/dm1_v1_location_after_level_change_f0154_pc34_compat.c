#include "dm1_v1_location_after_level_change_f0154_pc34_compat.h"

const char *DM1_V1_F0154_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1510 F0154_DUNGEON_GetLocationAfterLevelChange takes "
        "P0270_i_MapIndex, P0271_i_LevelDelta, P0272_pi_MapX, P0273_pi_MapY; "
        "DUNGEON.C:1517-1526 reads MAP.A.Level plus OffsetMapX/Y into "
        "local new-map state; DUNGEON.C:1528-1554 compares global coordinates "
        "against candidate maps on source level + delta and returns the target "
        "map index with local X/Y rebased by that map offset.";
}

static int dm1_v1_f0154_descriptor_valid(
    const DM1_V1_DungeonMapDescriptorF0154Pc34 *map)
{
    return map && map->width > 0 && map->height > 0;
}

int F0154_DUNGEON_GetLocationAfterLevelChange(
    const DM1_V1_DungeonMapDescriptorF0154Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    int levelDelta,
    int *mapX,
    int *mapY,
    int *outMapIndex)
{
    int sourceLevel;
    int targetSourceLevel;
    int globalX;
    int globalY;
    size_t i;

    if (outMapIndex) {
        *outMapIndex = -1;
    }
    if (!maps || mapCount == 0 || mapIndex < 0 ||
        (size_t)mapIndex >= mapCount || !mapX || !mapY || !outMapIndex ||
        !dm1_v1_f0154_descriptor_valid(&maps[mapIndex])) {
        return 0;
    }
    if (*mapX < 0 || *mapX >= maps[mapIndex].width ||
        *mapY < 0 || *mapY >= maps[mapIndex].height) {
        return 0;
    }

    sourceLevel = maps[mapIndex].sourceLevel;
    targetSourceLevel = sourceLevel + levelDelta;
    globalX = maps[mapIndex].offsetMapX + *mapX;
    globalY = maps[mapIndex].offsetMapY + *mapY;

    for (i = 0; i < mapCount; ++i) {
        int minX;
        int minY;
        int maxX;
        int maxY;

        if (!dm1_v1_f0154_descriptor_valid(&maps[i]) ||
            maps[i].sourceLevel != targetSourceLevel) {
            continue;
        }

        minX = maps[i].offsetMapX;
        minY = maps[i].offsetMapY;
        maxX = minX + maps[i].width - 1;
        maxY = minY + maps[i].height - 1;
        if (globalX < minX || globalX > maxX ||
            globalY < minY || globalY > maxY) {
            continue;
        }

        *mapX = globalX - minX;
        *mapY = globalY - minY;
        *outMapIndex = (int)i;
        return 1;
    }

    return 0;
}

int DM1_V1_Dungeon_GetLocationAfterLevelChangeF0154Pc34Compat(
    const DM1_V1_DungeonMapDescriptorF0154Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    int levelDelta,
    int mapX,
    int mapY,
    DM1_V1_DungeonLocationAfterLevelChangeF0154Pc34 *out)
{
    int targetMapIndex;
    int resolvedX;
    int resolvedY;
    int ok;

    if (!out) {
        return 0;
    }
    out->valid = 0;
    out->sourceLevel = 0;
    out->targetSourceLevel = 0;
    out->targetMapIndex = -1;
    out->mapX = mapX;
    out->mapY = mapY;
    out->globalX = 0;
    out->globalY = 0;

    if (!maps || mapCount == 0 || mapIndex < 0 ||
        (size_t)mapIndex >= mapCount ||
        !dm1_v1_f0154_descriptor_valid(&maps[mapIndex])) {
        return 0;
    }

    out->sourceLevel = maps[mapIndex].sourceLevel;
    out->targetSourceLevel = maps[mapIndex].sourceLevel + levelDelta;
    out->globalX = maps[mapIndex].offsetMapX + mapX;
    out->globalY = maps[mapIndex].offsetMapY + mapY;

    resolvedX = mapX;
    resolvedY = mapY;
    targetMapIndex = -1;
    ok = F0154_DUNGEON_GetLocationAfterLevelChange(
        maps, mapCount, mapIndex, levelDelta,
        &resolvedX, &resolvedY, &targetMapIndex);
    if (!ok) {
        return 0;
    }

    out->valid = 1;
    out->targetMapIndex = targetMapIndex;
    out->mapX = resolvedX;
    out->mapY = resolvedY;
    return 1;
}
