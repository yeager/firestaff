#ifndef DM1_V1_LOCATION_AFTER_LEVEL_CHANGE_F0154_PC34_COMPAT_H
#define DM1_V1_LOCATION_AFTER_LEVEL_CHANGE_F0154_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sourceLevel;
    int offsetMapX;
    int offsetMapY;
    int width;
    int height;
} DM1_V1_DungeonMapDescriptorF0154Pc34;

typedef struct {
    int valid;
    int sourceLevel;
    int targetSourceLevel;
    int targetMapIndex;
    int mapX;
    int mapY;
    int globalX;
    int globalY;
} DM1_V1_DungeonLocationAfterLevelChangeF0154Pc34;

const char *DM1_V1_F0154_SourceEvidencePc34(void);

int F0154_DUNGEON_GetLocationAfterLevelChange(
    const DM1_V1_DungeonMapDescriptorF0154Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    int levelDelta,
    int *mapX,
    int *mapY,
    int *outMapIndex);

int DM1_V1_Dungeon_GetLocationAfterLevelChangeF0154Pc34Compat(
    const DM1_V1_DungeonMapDescriptorF0154Pc34 *maps,
    size_t mapCount,
    int mapIndex,
    int levelDelta,
    int mapX,
    int mapY,
    DM1_V1_DungeonLocationAfterLevelChangeF0154Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif
