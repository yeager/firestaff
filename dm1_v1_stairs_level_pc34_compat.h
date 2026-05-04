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
} M11_StairDef;

#define M11_MAX_STAIRS 32
#define M11_MAX_LEVELS 16

typedef struct {
    int width;
    int height;
    int levelIndex;
} M11_LevelInfo;

typedef struct {
    M11_StairDef stairs[M11_MAX_STAIRS];
    int stairCount;
    M11_LevelInfo levels[M11_MAX_LEVELS];
    int levelCount;
    int currentLevel;
    int transitionActive;
    int transitionTicksLeft;
    int transitionFromLevel;
    int transitionToLevel;
} M11_StairLevelState;

void m11_stairs_init(M11_StairLevelState* s);
int m11_stairs_add(M11_StairLevelState* s, int x, int y, int dir, int destLevel, int destX, int destY, int destFacing);
int m11_stairs_check(const M11_StairLevelState* s, int x, int y, M11_StairDef* out);
int m11_stairs_use(M11_StairLevelState* s, int x, int y, int* newX, int* newY, int* newFacing);
void m11_stairs_add_level(M11_StairLevelState* s, int width, int height);
void m11_stairs_tick(M11_StairLevelState* s, int tickMs);
int m11_stairs_is_transitioning(const M11_StairLevelState* s);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_STAIRS_LEVEL_PC34_COMPAT_H */
