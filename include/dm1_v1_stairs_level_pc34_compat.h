#ifndef FIRESTAFF_DM1_V1_STAIRS_LEVEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_STAIRS_LEVEL_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Based on ReDMCSB MOVESENS.C F0267 stair handling, DUNGEON.C level transitions
 */

enum {
    DM1_STAIR_UP = 0,
    DM1_STAIR_DOWN = 1
};

typedef struct {
    int x;
    int y;
    int direction;
    int destLevel;
    int destX;
    int destY;
    int destFacing;
} DM1_V1_StairDefPc34;

#define DM1_V1_MAX_STAIRS_PC34 32
#define DM1_V1_MAX_LEVELS_PC34 16

typedef struct {
    int width;
    int height;
    int levelIndex;
} DM1_V1_LevelInfoPc34;

typedef struct {
    DM1_V1_StairDefPc34 stairs[DM1_V1_MAX_STAIRS_PC34];
    int stairCount;
    DM1_V1_LevelInfoPc34 levels[DM1_V1_MAX_LEVELS_PC34];
    int levelCount;
    int currentLevel;
    int transitionActive;
    int transitionTicksLeft;
    int transitionFromLevel;
    int transitionToLevel;
} DM1_V1_StairLevelStatePc34;

void DM1_V1_Stairs_InitPc34Compat(DM1_V1_StairLevelStatePc34* s);
int DM1_V1_Stairs_AddPc34Compat(DM1_V1_StairLevelStatePc34* s, int x, int y, int dir, int destLevel, int destX, int destY, int destFacing);
int DM1_V1_Stairs_CheckPc34Compat(const DM1_V1_StairLevelStatePc34* s, int x, int y, DM1_V1_StairDefPc34* out);
int DM1_V1_Stairs_UsePc34Compat(DM1_V1_StairLevelStatePc34* s, int x, int y, int* newX, int* newY, int* newFacing);
void DM1_V1_Stairs_AddLevelPc34Compat(DM1_V1_StairLevelStatePc34* s, int width, int height);
void DM1_V1_Stairs_TickPc34Compat(DM1_V1_StairLevelStatePc34* s, int tickMs);
int DM1_V1_Stairs_IsTransitioningPc34Compat(const DM1_V1_StairLevelStatePc34* s);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_STAIRS_LEVEL_PC34_COMPAT_H */
