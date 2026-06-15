/*
 * test_dm1_v1_dun01_f0150_f0701_step_delta_pc34_compat.c
 *
 * Source-locked to ReDMCSB DUNGEON.C:1371-1426 (F0150)
 * via the sanitized amalgam's G0233_ai_Graphic559_DirectionToStepEastCount
 * and G0234_ai_Graphic559_DirectionToStepNorthCount tables.
 *
 * DUN-01 (DM1 V1 functional-divergence-report.md):
 *   "Two parallel implementations of the same coordinate transform.
 *    The new path is more readable but is not directly testable
 *    against F0150 source-lock.  The legacy path remains
 *    source-locked via the amalgam."
 *
 * The F0701_MOVEMENT_GetStepDelta_Compat in the new compat
 * layer uses inline s_dx[4] / s_dy[4] tables.  These MUST match
 * the amalgam's G0233 / G0234 so the two parallel paths produce
 * identical results.  This test pins s_dx/s_dy against the
 * amalgam tables and exercises F0701 directly.
 *
 * Pins:
 *  - s_dx[i] == G0233[i] for all i (forward-step East component)
 *  - s_dy[i] == G0234[i] for all i (forward-step North component)
 *  - F0701 with MOVE_FORWARD == G0233[direction] (East) / G0234[direction] (North)
 *  - F0701 with MOVE_RIGHT == G0233[(direction+1)&3] / G0234[(direction+1)&3]
 *  - F0701 with MOVE_BACKWARD == -F0701(MOVE_FORWARD) (steps back)
 *  - F0701 with MOVE_LEFT == -F0701(MOVE_RIGHT) (steps left)
 */

#include "memory_movement_pc34_compat.h"

#include <stdio.h>

#define DIR_NORTH 0
#define DIR_EAST  1
#define DIR_SOUTH 2
#define DIR_WEST  3

/* Reference values from G0233_ai_Graphic559_DirectionToStepEastCount
 * and G0234_ai_Graphic559_DirectionToStepNorthCount (DUNGEON.C:1318-1338).
 * These are the canonical ReDMCSB tables; the F0150 source-locked
 * transformation uses them. */
static const int kG0233_EastStep[4]    = {  0,  1,  0, -1 };
static const int kG0234_NorthStep[4]   = { -1,  0,  1,  0 };

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    int dx, dy;
    int d, m;

    /* T1: F0701 with MOVE_FORWARD = the G0233/G0234 entries for direction. */
    for (d = 0; d < 4; ++d) {
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_FORWARD, &dx, &dy);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T1: F0701(forward, dir=%d) East step", d);
        CHECK(dx == kG0233_EastStep[d], buf);
        snprintf(buf, sizeof(buf),
                 "T1: F0701(forward, dir=%d) North step", d);
        CHECK(dy == kG0234_NorthStep[d], buf);
    }

    /* T2: F0701 with MOVE_RIGHT = G0233/G0234 entries for (direction+1)&3. */
    for (d = 0; d < 4; ++d) {
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_RIGHT, &dx, &dy);
        int rd = (d + 1) & 3;
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T2: F0701(right, dir=%d) East step", d);
        CHECK(dx == kG0233_EastStep[rd], buf);
        snprintf(buf, sizeof(buf),
                 "T2: F0701(right, dir=%d) North step", d);
        CHECK(dy == kG0234_NorthStep[rd], buf);
    }

    /* T3: MOVE_BACKWARD = -MOVE_FORWARD. */
    for (d = 0; d < 4; ++d) {
        int fdx, fdy;
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_FORWARD, &fdx, &fdy);
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_BACKWARD, &dx, &dy);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T3: BACKWARD dx = -FORWARD dx (dir=%d)", d);
        CHECK(dx == -fdx, buf);
        snprintf(buf, sizeof(buf),
                 "T3: BACKWARD dy = -FORWARD dy (dir=%d)", d);
        CHECK(dy == -fdy, buf);
    }

    /* T4: MOVE_LEFT = -MOVE_RIGHT. */
    for (d = 0; d < 4; ++d) {
        int rdx, rdy;
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_RIGHT, &rdx, &rdy);
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_LEFT, &dx, &dy);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T4: LEFT dx = -RIGHT dx (dir=%d)", d);
        CHECK(dx == -rdx, buf);
        snprintf(buf, sizeof(buf),
                 "T4: LEFT dy = -RIGHT dy (dir=%d)", d);
        CHECK(dy == -rdy, buf);
    }

    /* T5: Pair (MOVE_FORWARD + MOVE_RIGHT + MOVE_BACKWARD + MOVE_LEFT) is a
     *     null step (party turns full circle). */
    int totalDx = 0, totalDy = 0;
    for (d = 0; d < 4; ++d) {
        int m_action[4] = {MOVE_FORWARD, MOVE_RIGHT, MOVE_BACKWARD, MOVE_LEFT};
        for (m = 0; m < 4; ++m) {
            F0701_MOVEMENT_GetStepDelta_Compat(d, m_action[m], &dx, &dy);
            totalDx += dx;
            totalDy += dy;
        }
    }
    CHECK(totalDx == 0, "T5: full circle dx is 0");
    CHECK(totalDy == 0, "T5: full circle dy is 0");

    /* T6: F0701 honours all 4 cardinal directions. */
    for (d = 0; d < 4; ++d) {
        F0701_MOVEMENT_GetStepDelta_Compat(d, MOVE_FORWARD, &dx, &dy);
        CHECK(dx != 0 || dy != 0, "T6: F0701 forward direction is non-zero");
    }

    /* T7: Invalid move action returns (0, 0). */
    F0701_MOVEMENT_GetStepDelta_Compat(DIR_NORTH, -1, &dx, &dy);
    CHECK(dx == 0 && dy == 0, "T7: invalid action returns (0, 0)");

    printf("PASS: DUN-01 F0150/F0701 step-delta invariants (7 scenarios)\n");
    return 0;
}
