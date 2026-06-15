/*
 * test_dm1_v1_grp02_f0734_statistic_adjusted_attack_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C:1106-1115 (F0307_CHAMPION_
 * GetStatisticAdjustedAttack).
 *
 * GRP-02 (DM1 V1 functional-divergence-report.md):
 *   "F0190/F0191/F0192 damage outcomes are amalgam-only."
 *
 * F0734 is the new-path replacement for F0307.  Pins the
 * F0734 contract:
 *  T1  NULL outAdjusted returns 0
 *  T2  factor = 170 - statisticCurrent
 *  T3  factor < 16: result = attack >> 3
 *  T4  factor >= 16: result = (attack * factor) >> 7
 *  T5  statisticCurrent = 0 -> factor = 170 -> result = (attack*170)>>7
 *  T6  statisticCurrent = 154 -> factor = 16 -> result = (attack*16)>>7
 *  T7  statisticCurrent = 155 -> factor = 15 (< 16) -> result = attack>>3
 *  T8  attack = 0 -> result = 0
 *  T9  Large attack (1000) handled correctly
 *  T10 Returns 1 on success
 *  T11 outAdjusted is set on success
 *  T12 statisticMaximum unused (kept for future clamping)
 *
 * Source-locked to ReDMCSB CHAMPION.C:1106-1115.
 */

#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    int adjusted = -1;
    int rc;

    /* T1: NULL outAdjusted. */
    CHECK(F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 100, 50, NULL) == 0,
          "T1: NULL outAdjusted returns 0");

    /* T2-T4: Boundary tests. */
    /* statisticCurrent=0: factor=170, result = (attack*170)>>7 */
    adjusted = -1;
    CHECK(F0734_COMBAT_GetStatisticAdjustedAttack_Compat(0, 100, 100, &adjusted) == 1,
          "T2a: statCurrent=0 returns 1");
    /* attack=100, factor=170, (100*170)>>7 = 17000>>7 = 132 */
    CHECK(adjusted == 132, "T2b: statCurrent=0, attack=100 -> 132");

    /* T3: factor < 16 path.  statCurrent=155 -> factor=15 (<16). */
    adjusted = -1;
    CHECK(F0734_COMBAT_GetStatisticAdjustedAttack_Compat(155, 100, 100, &adjusted) == 1,
          "T3a: statCurrent=155 returns 1");
    /* attack=100, factor<16 -> result = 100>>3 = 12 */
    CHECK(adjusted == 12, "T3b: statCurrent=155, attack=100 -> 12 (attack>>3)");

    /* T4: factor == 16 (boundary, >= 16). */
    adjusted = -1;
    CHECK(F0734_COMBAT_GetStatisticAdjustedAttack_Compat(154, 100, 100, &adjusted) == 1,
          "T4a: statCurrent=154 returns 1");
    /* attack=100, factor=16, (100*16)>>7 = 1600>>7 = 12 */
    CHECK(adjusted == 12, "T4b: statCurrent=154, attack=100 -> 12 ((100*16)>>7)");

    /* T5: Large statCurrent (factor goes negative). */
    adjusted = -1;
    CHECK(F0734_COMBAT_GetStatisticAdjustedAttack_Compat(200, 100, 100, &adjusted) == 1,
          "T5: statCurrent=200, factor=-30 (< 16) -> attack>>3");
    CHECK(adjusted == 12, "T5: statCurrent=200, attack=100 -> 12 (attack>>3)");

    /* T6: factor boundary at 15. */
    adjusted = -1;
    F0734_COMBAT_GetStatisticAdjustedAttack_Compat(155, 100, 64, &adjusted);
    CHECK(adjusted == 8, "T6: statCurrent=155, attack=64 -> 8 (64>>3)");

    /* T7: factor at 16. */
    adjusted = -1;
    F0734_COMBAT_GetStatisticAdjustedAttack_Compat(154, 100, 64, &adjusted);
    /* 64*16>>7 = 1024>>7 = 8 */
    CHECK(adjusted == 8, "T7: statCurrent=154, attack=64 -> 8 ((64*16)>>7)");

    /* T8: attack = 0. */
    adjusted = -1;
    F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 100, 0, &adjusted);
    CHECK(adjusted == 0, "T8: attack=0 -> adjusted=0");

    /* T9: Large attack. */
    adjusted = -1;
    F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 100, 1000, &adjusted);
    /* 1000*70>>7 = 70000>>7 = 546 */
    CHECK(adjusted == 546, "T9: attack=1000, statCurrent=100 -> 546");

    /* T10: Returns 1 on success. */
    adjusted = -1;
    rc = F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 100, 50, &adjusted);
    CHECK(rc == 1, "T10: returns 1 on success");

    /* T11: outAdjusted is set on success. */
    adjusted = -999;
    F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 100, 50, &adjusted);
    CHECK(adjusted != -999, "T11: outAdjusted is set");

    /* T12: statisticMaximum is unused.  Various values produce same result. */
    {
        int adj1 = -1, adj2 = -1;
        F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 50, 50, &adj1);
        F0734_COMBAT_GetStatisticAdjustedAttack_Compat(100, 200, 50, &adj2);
        CHECK(adj1 == adj2, "T12: statisticMaximum ignored");
    }

    /* Sanity: monotonic decrease as statisticCurrent increases (more
     * powerful champions do less damage per point, F0307 inverse). */
    {
        int prev = 1000;
        int k;
        for (k = 0; k <= 200; k += 20) {
            int v = -1;
            F0734_COMBAT_GetStatisticAdjustedAttack_Compat(k, 200, 100, &v);
            /* factor = 170 - statCurrent.  As statCurrent goes up,
             * factor goes down, so result decreases.  Once factor
             * drops below 16 (statCurrent > 154), result = attack>>3 = 12
             * which is less than any factor-170 result. */
            if (k < 155) {
                CHECK(v < prev, "T13: monotonic decrease in (factor>=16) range");
            } else {
                CHECK(v == 12, "T13: factor<16 plateau at 12");
            }
            prev = v;
        }
    }

    printf("PASS: GRP-02 F0734 statistic-adjusted-attack pin (13 scenarios)\n");
    return 0;
}
