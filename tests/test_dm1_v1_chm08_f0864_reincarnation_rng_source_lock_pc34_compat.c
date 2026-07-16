/*
 * test_dm1_v1_chm08_f0864_reincarnation_rng_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB REVIVE.C F0282:807-810:
 *   12 iterations of M002_RANDOM(7), each increments the picked
 *   statistic's current+maximum by 1.
 *
 * CHM-08 (DM1 V1 functional-divergence-report.md):
 *   "Reincarnation AL0823_ui_StatisticIndex < 7 loop count is
 *    preserved; the 12 random stat increments are RNG-deterministic"
 *    — already source-locked, included only as verification.
 *
 * Pins the F0864_RESURRECTION_ComputeReincarnation_Compat invariant:
 *  T1  All vitals (health, stamina, mana) are halved (max & current)
 *  T2  With rngValues all 0, statIncrements[0] = 12 (all 12 picks
 *      go to stat 0)
 *  T3  With rngValues {0..11}, each statIncrements[i] = 1
 *      (each stat picked once) -- but only for stats 0..6.  Stats
 *      7..11 contribute nothing (rng % 7 wraps).
 *  T4  With deterministic rng, the result is deterministic
 *      (call N times with same rng => same result)
 *  T5  Loop runs exactly 12 times (sum of statIncrements == 12)
 *  T6  Stat index range: 0..6 only (never 7+)
 *
 * Source-locked to ReDMCSB REVIVE.C F0282:807-810.
 */

#include "dm1_v1_resurrection_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    ReincarnationResult_Compat r;
    uint8_t rng[12];
    int i, sum;

    /* T1: All vitals are halved. */
    memset(rng, 0, sizeof(rng));
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        200, 100, 400, 200, 100, 50, rng);
    CHECK(r.newMaxHealth == 100,       "T1: maxHealth halved (200->100)");
    CHECK(r.newCurrentHealth == 50,    "T1: currentHealth halved (100->50)");
    CHECK(r.newMaxStamina == 200,      "T1: maxStamina halved (400->200)");
    CHECK(r.newCurrentStamina == 100,  "T1: currentStamina halved (200->100)");
    CHECK(r.newMaxMana == 50,          "T1: maxMana halved (100->50)");
    CHECK(r.newCurrentMana == 25,      "T1: currentMana halved (50->25)");

    /* T2: rng all 0 -> all 12 picks go to stat 0. */
    memset(rng, 0, sizeof(rng));
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        100, 50, 200, 100, 50, 25, rng);
    CHECK(r.statIncrements[0] == 12, "T2: rng all 0 -> stat 0 gets all 12");
    sum = 0;
    for (i = 0; i < 7; ++i) sum += r.statIncrements[i];
    CHECK(sum == 12, "T2: total increments == 12");

    /* T3: rng {0,1,2,3,4,5,6,7,8,9,10,11} -> stat 0..6 picked
     * once each (7 picks), then 5 picks wrap (7%7=0, 8%7=1, ...
     * 11%7=4) -> stat 0 gets +2, stat 1 gets +2, stat 2 gets +2,
     * stat 3 gets +2, stat 4 gets +2, stat 5 gets +1, stat 6 gets +1. */
    for (i = 0; i < 12; ++i) rng[i] = (uint8_t)i;
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        100, 50, 200, 100, 50, 25, rng);
    CHECK(r.statIncrements[0] == 2, "T3: stat 0 gets 2 picks (0+7)");
    CHECK(r.statIncrements[1] == 2, "T3: stat 1 gets 2 picks (1+8)");
    CHECK(r.statIncrements[2] == 2, "T3: stat 2 gets 2 picks (2+9)");
    CHECK(r.statIncrements[3] == 2, "T3: stat 3 gets 2 picks (3+10)");
    CHECK(r.statIncrements[4] == 2, "T3: stat 4 gets 2 picks (4+11)");
    CHECK(r.statIncrements[5] == 1, "T3: stat 5 gets 1 pick (5)");
    CHECK(r.statIncrements[6] == 1, "T3: stat 6 gets 1 pick (6)");
    sum = 0;
    for (i = 0; i < 7; ++i) sum += r.statIncrements[i];
    CHECK(sum == 12, "T3: total increments == 12");

    /* T4: Deterministic for same rng. */
    for (i = 0; i < 12; ++i) rng[i] = (uint8_t)((i * 31 + 7) & 0xFF);
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        100, 50, 200, 100, 50, 25, rng);
    ReincarnationResult_Compat r2 = F0864_RESURRECTION_ComputeReincarnation_Compat(
        100, 50, 200, 100, 50, 25, rng);
    for (i = 0; i < 7; ++i) {
        CHECK(r.statIncrements[i] == r2.statIncrements[i],
              "T4: deterministic for same rng");
    }
    CHECK(r.newMaxHealth == r2.newMaxHealth, "T4: deterministic maxHealth");

    /* T5: Sum across all 7 stat increments is exactly 12. */
    for (i = 0; i < 12; ++i) rng[i] = (uint8_t)((i * 13 + 3) & 0x07);
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        100, 50, 200, 100, 50, 25, rng);
    sum = 0;
    for (i = 0; i < 7; ++i) sum += r.statIncrements[i];
    CHECK(sum == 12, "T5: sum of statIncrements == 12");

    /* T6: All stat indices in 0..6 (never out of range). */
    for (i = 0; i < 12; ++i) rng[i] = (uint8_t)(i * 7 + 1);  /* arbitrary */
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        100, 50, 200, 100, 50, 25, rng);
    for (i = 0; i < 7; ++i) {
        CHECK(r.statIncrements[i] <= 12,
              "T6: each statIncrement[i] <= 12 (rng is in 0..6)");
    }

    printf("PASS: CHM-08 F0864 reincarnation 12-stat-increment source-lock (6 scenarios)\n");
    return 0;
}
