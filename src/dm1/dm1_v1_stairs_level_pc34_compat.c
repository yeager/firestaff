#include "dm1_v1_stairs_level_pc34_compat.h"
#include <string.h>

/*
 * Invariants governing the M11_StairLevelState state machine:
 *
 * I1 — No concurrent transitions.  m11_stairs_use must not mutate state while
 *   transitionActive is already 1.  A second stair step during a pending
 *   transition would corrupt currentLevel / transitionFromLevel / transitionToLevel.
 *
 * I2 — IRED (Immutability-before-mutation rule).  transitionFromLevel and
 *   transitionToLevel describe the edge in flight.  Both must be written BEFORE
 *   currentLevel changes, and transitionActive must be set as the very last
 *   write of the m11_stairs_use sequence so that any concurrent caller that
 *   tests transitionActive atomically sees either "no transition" or "complete
 *   new transition" — never a partial or mid-flight state.
 *
 * I3 — transitionTicksLeft >= 0 always.  tickMs can overshoot (e.g. OS
 *   scheduling delay), so the tick function clamps to 0 rather than allowing
 *   a negative underflow that would also corrupt transitionActive logic.
 *
 * I4 — Tick is idempotent when inactive.  When transitionActive is 0 the
 *   function is a no-op regardless of how many ticks fire; callers such as
 *   m11_game_view.c invoke it every frame without branching on transition state.
 */

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

    /* I1 — reject any stair step while a transition is already in flight. */
    if (s->transitionActive) return 0;

    M11_StairDef foundStair;
    if (!m11_stairs_check(s, x, y, &foundStair)) {
        return 0;
    }

    /*
     * I2 — write all transition metadata BEFORE currentLevel changes.
     *
     * order: transitionFromLevel -> transitionToLevel -> currentLevel ->
     *         transitionTicksLeft -> transitionActive (last)
     *
     * transitionTicksLeft is set to a nominal value (500 ms); no authoritative
     * ReDMCSB constant is known for this.  Adjust only if callers report
     * transitions ending prematurely or hanging.
     */
    s->transitionFromLevel  = s->currentLevel;
    s->transitionToLevel    = foundStair.destLevel;
    s->currentLevel         = foundStair.destLevel;
    s->transitionTicksLeft  = 500;
    s->transitionActive     = 1;  /* must be last write of the sequence */

    if (newX)      *newX      = foundStair.destX;
    if (newY)      *newY      = foundStair.destY;
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

    /*
     * I4 — idempotent no-op when inactive.
     * I3 — transitionTicksLeft must never go negative; clamp on overshoot.
     */
    if (s->transitionActive && s->transitionTicksLeft > 0) {
        if (s->transitionTicksLeft <= tickMs) {
            s->transitionTicksLeft = 0;
        } else {
            s->transitionTicksLeft -= tickMs;
        }
        if (s->transitionTicksLeft == 0) {
            s->transitionActive = 0;
        }
    }
}

int m11_stairs_is_transitioning(const M11_StairLevelState* s) {
    if (!s) return 0;
    return s->transitionActive;
}
