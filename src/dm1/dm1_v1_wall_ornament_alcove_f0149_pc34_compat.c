#include "dm1_v1_wall_ornament_alcove_f0149_pc34_compat.h"

static const int kDefaultAlcoveOrnamentIndices[
    DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34] = { 1, 2, 3 };

const char* DM1_V1_F0149_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1332 F0149_DUNGEON_IsWallOrnamentAnAlcove takes "
        "P0252_i_WallOrnamentIndex\n"
        "DUNGEON.C:1336 scans G0267_ai_CurrentMapAlcoveOrnamentIndices with "
        "a counter\n"
        "DUNVIEW.C G0192 initializes the three alcove ornament indices used "
        "by F0107 wall-ornament and F0115 alcove object visibility";
}

int F0149_DUNGEON_IsWallOrnamentAnAlcove(
    const int* currentMapAlcoveOrnamentIndices,
    size_t alcoveOrnamentIndexCount,
    int wallOrnamentIndex)
{
    size_t i;

    if (!currentMapAlcoveOrnamentIndices ||
        alcoveOrnamentIndexCount !=
            DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34 ||
        wallOrnamentIndex < 0) {
        return 0;
    }

    for (i = 0; i < alcoveOrnamentIndexCount; ++i) {
        if (currentMapAlcoveOrnamentIndices[i] == wallOrnamentIndex) {
            return 1;
        }
    }
    return 0;
}

int DM1_V1_Dungeon_IsWallOrnamentAnAlcoveF0149Pc34Compat(
    const int* currentMapAlcoveOrnamentIndices,
    size_t alcoveOrnamentIndexCount,
    int wallOrnamentIndex)
{
    return F0149_DUNGEON_IsWallOrnamentAnAlcove(
        currentMapAlcoveOrnamentIndices,
        alcoveOrnamentIndexCount,
        wallOrnamentIndex);
}

int DM1_V1_Dungeon_IsDefaultWallOrnamentAnAlcoveF0149Pc34Compat(
    int wallOrnamentIndex)
{
    return F0149_DUNGEON_IsWallOrnamentAnAlcove(
        kDefaultAlcoveOrnamentIndices,
        DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
        wallOrnamentIndex);
}
