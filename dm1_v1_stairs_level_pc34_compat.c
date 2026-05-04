#include "dm1_v1_stairs_level_pc34_compat.h"
#include <string.h>

void m11_stairs_init(M11_StairLevelState* s) {
    if (!s) return;
    memset(s, 0, sizeof(M11_StairLevelState));
    s->currentLevel = 0;
}

int m11_stairs_add(M11_StairLevelState* s, int x, int y, int dir, int destLevel, int destX, int destY, int destFacing) {
    if (!s) return 0;
    if (s->stairCount >= M11_MAX_STAIRS) return 0;

    M11_StairDef* stair = &s->stairs[s->stairCount];
    stair->x = x;
    stair->y = y;
    stair->direction = dir;
    stair->destLevel = destLevel;
    stair->destX = destX;
    stair->destY = destY;
    stair->destFacing = destFacing;

    s->stairCount++;
    return 1;
}

int m11_stairs_check(const M11_StairLevelState* s, int x, int y, M11_StairDef* out) {
    if (!s || !out) return 0;

    for (int i = 0; i < s->stairCount; i++) {
        if (s->stairs[i].x == x && s->stairs[i].y == y) {
            *out = s->stairs[i];
            return 1;
        }
    }
    return 0;
}

int m11_stairs_use(M11_StairLevelState* s, int x, int y, int* newX, int* newY, int* newFacing) {
    if (!s) return 0;

    M11_StairDef foundStair;
    if (!m11_stairs_check(s, x, y, &foundStair)) {
        return 0;
    }

    s->transitionFromLevel = s->currentLevel;
    s->currentLevel = foundStair.destLevel;
    s->transitionToLevel = foundStair.destLevel;
    s->transitionActive = 1;
    s->transitionTicksLeft = 500;

    if (newX) *newX = foundStair.destX;
    if (newY) *newY = foundStair.destY;
    if (newFacing) *newFacing = foundStair.destFacing;

    return 1;
}

void m11_stairs_add_level(M11_StairLevelState* s, int width, int height) {
    if (!s) return;
    if (s->levelCount >= M11_MAX_LEVELS) return;

    M11_LevelInfo* level = &s->levels[s->levelCount];
    level->width = width;
    level->height = height;
    level->levelIndex = s->levelCount;

    s->levelCount++;
}

void m11_stairs_tick(M11_StairLevelState* s, int tickMs) {
    if (!s) return;

    if (s->transitionActive) {
        s->transitionTicksLeft -= tickMs;
        if (s->transitionTicksLeft <= 0) {
            s->transitionActive = 0;
        }
    }
}

int m11_stairs_is_transitioning(const M11_StairLevelState* s) {
    if (!s) return 0;
    return s->transitionActive;
}
