#ifndef FIRESTAFF_DM1_V1_WALL_ORNAMENT_ALCOVE_F0149_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_WALL_ORNAMENT_ALCOVE_F0149_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34 = 3
};

const char* DM1_V1_F0149_SourceEvidencePc34(void);

int F0149_DUNGEON_IsWallOrnamentAnAlcove(
    const int* currentMapAlcoveOrnamentIndices,
    size_t alcoveOrnamentIndexCount,
    int wallOrnamentIndex);

int DM1_V1_Dungeon_IsWallOrnamentAnAlcoveF0149Pc34Compat(
    const int* currentMapAlcoveOrnamentIndices,
    size_t alcoveOrnamentIndexCount,
    int wallOrnamentIndex);

int DM1_V1_Dungeon_IsDefaultWallOrnamentAnAlcoveF0149Pc34Compat(
    int wallOrnamentIndex);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_WALL_ORNAMENT_ALCOVE_F0149_PC34_COMPAT_H */
