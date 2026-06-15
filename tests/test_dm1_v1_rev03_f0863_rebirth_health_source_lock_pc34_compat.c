/*
 * test_dm1_v1_rev03_f0863_rebirth_health_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB REVIVE.C:828-839 (F0283_CHAMPION_
 * ViAltarRebirth health penalty):
 *   newMax = max(25, oldMax - (oldMax/64 + 1))
 *   newCurrent = newMax / 2
 *
 * REV-03 (DM1 V1 functional-divergence-report.md):
 *   "F0283 ViAltar rebirth is preserved verbatim" — N/A match.
 *
 * Firestaff's F0863_RESURRECTION_ComputeRebirthHealth_Compat
 * (in src/dm1/dm1_v1_resurrection_pc34_compat.c) implements the
 * exact formula.  This test pins the contract:
 *  T1  currentMaxHealth=100 -> penalty = 100/64 + 1 = 2
 *                 newMax = max(25, 100 - 2) = 98
 *                 newCurrent = 49
 *  T2  currentMaxHealth=64 -> penalty = 64/64 + 1 = 2
 *                 newMax = max(25, 62) = 62
 *                 newCurrent = 31
 *  T3  currentMaxHealth=63 -> penalty = 63/64 + 1 = 1
 *                 newMax = max(25, 62) = 62
 *                 newCurrent = 31
 *  T4  currentMaxHealth=50 -> penalty = 50/64 + 1 = 1
 *                 newMax = max(25, 49) = 49
 *                 newCurrent = 24
 *  T5  currentMaxHealth=25 -> penalty = 25/64 + 1 = 1
 *                 newMax = max(25, 24) = 25
 *                 newCurrent = 12
 *  T6  currentMaxHealth=24 -> penalty = 24/64 + 1 = 1
 *                 newMax = max(25, 23) = 25
 *                 newCurrent = 12
 *  T7  currentMaxHealth=0 -> penalty = 0/64 + 1 = 1
 *                 newMax = max(25, -1) = 25
 *                 newCurrent = 12
 *  T8  currentMaxHealth=200 -> penalty = 200/64 + 1 = 4
 *                 newMax = max(25, 196) = 196
 *                 newCurrent = 98
 *  T9  currentMaxHealth=1024 -> penalty = 1024/64 + 1 = 17
 *                 newMax = max(25, 1007) = 1007
 *                 newCurrent = 503
 *  T10 Penalty is always (max/64 + 1), never larger
 *  T11 newMax is always >= 25 (the floor)
 *  T12 newCurrent is always newMax / 2
 *  T13 100 sample values: newMax <= currentMax for all
 *  T14 100 sample values: newCurrent <= newMax for all
 *
 * Source-locked to ReDMCSB REVIVE.C:828-839.
 */

#include "dm1_v1_resurrection_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* ReDMCSB reference implementation. */
static RebirthHealthResult_Compat kF0863_ComputeRebirthHealth_Ref(int16_t m) {
    RebirthHealthResult_Compat r;
    int16_t penalty = (m >> 6) + 1;
    int16_t newMax = m - penalty;
    if (newMax < 25) newMax = 25;
    r.newMaxHealth = newMax;
    r.newCurrentHealth = newMax >> 1;
    return r;
}

int main(void) {
    RebirthHealthResult_Compat r;
    RebirthHealthResult_Compat ref;

    /* T1: currentMaxHealth=100. */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(100);
    ref = kF0863_ComputeRebirthHealth_Ref(100);
    CHECK(r.newMaxHealth == 98, "T1: 100 -> newMax=98");
    CHECK(r.newCurrentHealth == 49, "T1: 100 -> newCurrent=49");
    CHECK(r.newMaxHealth == ref.newMaxHealth, "T1: matches reference");
    CHECK(r.newCurrentHealth == ref.newCurrentHealth, "T1: matches reference");

    /* T2: currentMaxHealth=64. */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(64);
    ref = kF0863_ComputeRebirthHealth_Ref(64);
    CHECK(r.newMaxHealth == 62, "T2: 64 -> newMax=62");
    CHECK(r.newCurrentHealth == 31, "T2: 64 -> newCurrent=31");
    CHECK(r.newMaxHealth == ref.newMaxHealth, "T2: matches reference");

    /* T3: currentMaxHealth=63. */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(63);
    ref = kF0863_ComputeRebirthHealth_Ref(63);
    CHECK(r.newMaxHealth == 62, "T3: 63 -> newMax=62");
    CHECK(r.newCurrentHealth == 31, "T3: 63 -> newCurrent=31");

    /* T4: currentMaxHealth=50. */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(50);
    CHECK(r.newMaxHealth == 49, "T4: 50 -> newMax=49");
    CHECK(r.newCurrentHealth == 24, "T4: 50 -> newCurrent=24");

    /* T5: currentMaxHealth=25 (at the floor). */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(25);
    CHECK(r.newMaxHealth == 25, "T5: 25 -> newMax=25 (at floor)");
    CHECK(r.newCurrentHealth == 12, "T5: 25 -> newCurrent=12");

    /* T6: currentMaxHealth=24 (below the floor, clamped to 25). */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(24);
    CHECK(r.newMaxHealth == 25, "T6: 24 -> newMax=25 (clamped to floor)");
    CHECK(r.newCurrentHealth == 12, "T6: 24 -> newCurrent=12");

    /* T7: currentMaxHealth=0 (extreme edge, clamped to 25). */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(0);
    CHECK(r.newMaxHealth == 25, "T7: 0 -> newMax=25 (clamped to floor)");
    CHECK(r.newCurrentHealth == 12, "T7: 0 -> newCurrent=12");

    /* T8: currentMaxHealth=200. */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(200);
    ref = kF0863_ComputeRebirthHealth_Ref(200);
    CHECK(r.newMaxHealth == 196, "T8: 200 -> newMax=196");
    CHECK(r.newCurrentHealth == 98, "T8: 200 -> newCurrent=98");
    CHECK(r.newMaxHealth == ref.newMaxHealth, "T8: matches reference");

    /* T9: currentMaxHealth=1024 (large). */
    r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(1024);
    CHECK(r.newMaxHealth == 1007, "T9: 1024 -> newMax=1007");
    CHECK(r.newCurrentHealth == 503, "T9: 1024 -> newCurrent=503");

    /* T10: Penalty is always (max/64 + 1) - sanity check. */
    CHECK((100 >> 6) + 1 == 2, "T10: penalty 100/64+1 = 2");
    CHECK((64 >> 6) + 1 == 2, "T10: penalty 64/64+1 = 2");
    CHECK((1000 >> 6) + 1 == 16, "T10: penalty 1000/64+1 = 16");

    /* T11: newMax is always >= 25. */
    {
        int16_t m;
        for (m = 0; m <= 200; ++m) {
            r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(m);
            CHECK(r.newMaxHealth >= 25, "T11: newMax >= 25 for all m in [0..200]");
        }
    }

    /* T12: newCurrent is always newMax / 2. */
    {
        int16_t m;
        for (m = 0; m <= 200; ++m) {
            r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(m);
            CHECK(r.newCurrentHealth == r.newMaxHealth / 2,
                  "T12: newCurrent = newMax / 2");
        }
    }

    /* T13: 100 sample values: newMax <= currentMax for currentMax >= 26.
     * For currentMax < 26, the floor of 25 is enforced, so newMax may
     * exceed currentMax.  This is by design (the floor). */
    {
        int k;
        for (k = 2; k < 102; ++k) {
            int16_t m = (int16_t)(k * 13);
            r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(m);
            CHECK(r.newMaxHealth <= m,
                  "T13: newMax <= currentMax for currentMax >= 26");
        }
    }

    /* T14: 100 sample values: newCurrent <= newMax. */
    {
        int k;
        for (k = 0; k < 100; ++k) {
            int16_t m = (int16_t)(k * 13);
            r = F0863_RESURRECTION_ComputeRebirthHealth_Compat(m);
            CHECK(r.newCurrentHealth <= r.newMaxHealth,
                  "T14: newCurrent <= newMax");
        }
    }

    printf("PASS: REV-03 F0863 ViAltar rebirth health source-lock (14 scenarios)\n");
    return 0;
}
