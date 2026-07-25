#include "dm1_v1_wall_ornament_alcove_f0149_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    const int defaultAlcoves[DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34] = {
        1, 2, 3
    };
    (void)defaultAlcoves;
    const int customAlcoves[DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34] = {
        4, 7, 11
    };
    (void)customAlcoves;
    const int duplicateAlcoves[DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34] = {
        5, 5, 8
    };
    (void)duplicateAlcoves;

    assert(strstr(DM1_V1_F0149_SourceEvidencePc34(),
                  "F0149_DUNGEON_IsWallOrnamentAnAlcove") != 0);

    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               defaultAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               1) == 1);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               defaultAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               2) == 1);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               defaultAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               3) == 1);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               defaultAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               0) == 0);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               defaultAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               4) == 0);

    assert(DM1_V1_Dungeon_IsWallOrnamentAnAlcoveF0149Pc34Compat(
               customAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               7) == 1);
    assert(DM1_V1_Dungeon_IsWallOrnamentAnAlcoveF0149Pc34Compat(
               customAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               2) == 0);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               duplicateAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               5) == 1);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               duplicateAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               8) == 1);

    assert(DM1_V1_Dungeon_IsDefaultWallOrnamentAnAlcoveF0149Pc34Compat(1) ==
           1);
    assert(DM1_V1_Dungeon_IsDefaultWallOrnamentAnAlcoveF0149Pc34Compat(3) ==
           1);
    assert(DM1_V1_Dungeon_IsDefaultWallOrnamentAnAlcoveF0149Pc34Compat(6) ==
           0);

    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               0, DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34, 1) == 0);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(defaultAlcoves, 2, 1) == 0);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(defaultAlcoves, 4, 1) == 0);
    assert(F0149_DUNGEON_IsWallOrnamentAnAlcove(
               defaultAlcoves,
               DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
               -1) == 0);

    return 0;
}
