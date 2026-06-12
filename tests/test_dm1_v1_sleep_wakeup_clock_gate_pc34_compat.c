/*
 * test_dm1_v1_sleep_wakeup_clock_gate_pc34_compat.c
 *
 * DM1 V1 sleep/wakeup/clock gate — focused on the F0331 *clock* slice that
 * the existing rest/wake gates do not cover:
 *
 *   1. F0830_LIFECYCLE_ComputeTimeCriteria_Compat — the time-criteria
 *      bit pattern that gates F0331 mana-regen (wisdom+wizPriest) and HP
 *      healing (vitality+12).  ReDMCSB CHAMPION.C F0331 lines
 *      2331-2332 (L1012_ui_TimeCriteria).
 *   2. F0846_LIFECYCLE_ApplyStatDrift_Compat — the rest-modulated
 *      statistic-recovery cadence.  ReDMCSB CHAMPION.C F0331 lines
 *      2450-2473 (the `!((unsigned int16_t)GameTime &
 *      (G0300_B_PartyIsResting ? 63 : 255))` gate plus the
 *      `current > max` clamp-by-current-over-max branch).
 *
 * Lane contract (pass-NNN [dm1-v1-sleep-wakeup-clock-gate]):
 *   - Pin the exact bit-rewiring F0830 uses: ((GT & 0x80) +
 *     ((GT & 0x100) >> 2) + ((GT & 0x40) << 2)) >> 2.
 *   - Pin the two clock periods (256 normal, 64 rest) plus the
 *     "drift does not run on non-multiple ticks" guard.
 *   - Pin the below-max +1 / above-max -current/maximum clamp.
 *   - Pin the leader candidate skip that F0331 has but F0846 mirrors
 *     at the champion level (caller must not invoke on a dead champ).
 *
 * This is contract-only and source-locked to ReDMCSB CHAMPION.C F0331.
 * It does not claim original DOS parity.
 */

#include <stdio.h>
#include <string.h>

#include "memory_champion_lifecycle_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL %s\n", (msg)); \
    } \
} while (0)

#define CHECK_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL %s: got %d expected %d\n", (msg), a_, e_); \
    } \
} while (0)

/* ── Helpers ──────────────────────────────────────────────────────── */

static uint16_t time_criteria(uint32_t gameTime) {
    return F0830_LIFECYCLE_ComputeTimeCriteria_Compat(gameTime);
}

static void reset_stats(struct ChampionLifecycleState_Compat* champ,
                        uint8_t cur, uint8_t max) {
    int i;
    for (i = 0; i < LIFECYCLE_STAT_COUNT; ++i) {
        champ->statistics[i][LIFECYCLE_STAT_MAXIMUM] = max;
        champ->statistics[i][LIFECYCLE_STAT_CURRENT] = cur;
        champ->statistics[i][LIFECYCLE_STAT_MINIMUM] = 0;
    }
}

static struct ChampionLifecycleState_Compat make_blank_champ(void) {
    struct ChampionLifecycleState_Compat champ;
    memset(&champ, 0, sizeof(champ));
    return champ;
}

/* ── F0830 time-criteria bit pattern ──────────────────────────────── */

static void test_time_criteria_zero_when_low_bits_clear(void) {
    /* ReDMCSB CHAMPION.C F0331 line 2331: timeCriteria reads from
     * bits 0x40/0x80/0x100 of gameTime only.  For gameTime = 0
     * (and for any gameTime < 0x40) all three operands are 0,
     * so timeCriteria must be 0. */
    CHECK_EQ(time_criteria(0u),   0, "GT=0 -> timeCriteria=0");
    CHECK_EQ(time_criteria(1u),   0, "GT=1 -> timeCriteria=0");
    CHECK_EQ(time_criteria(0x3Fu), 0, "GT=0x3F -> timeCriteria=0");
}

static void test_time_criteria_bit0x40_contribution(void) {
    /* F0830: c = (GT & 0x40) << 2.  When only bit 0x40 is set
     * c = 0x100, and after the final >> 2 the contribution is 0x40. */
    CHECK_EQ(time_criteria(0x40u), 0x40, "GT=0x40 -> timeCriteria=0x40");
    CHECK_EQ(time_criteria(0x41u), 0x40, "GT=0x41 -> timeCriteria=0x40");
}

static void test_time_criteria_bit0x80_contribution(void) {
    /* Bit 0x80 contributes 0x80 / 4 = 0x20 to timeCriteria. */
    CHECK_EQ(time_criteria(0x80u), 0x20, "GT=0x80 -> timeCriteria=0x20");
}

static void test_time_criteria_bit0x100_contribution(void) {
    /* Bit 0x100 is shifted right 2 then divided by 4: 0x100 / 4 / 4 = 0x10. */
    CHECK_EQ(time_criteria(0x100u), 0x10, "GT=0x100 -> timeCriteria=0x10");
}

static void test_time_criteria_combined_bits(void) {
    /* F0830: a = GT & 0x80
     *        b = (GT & 0x100) >> 2
     *        c = (GT & 0x40) << 2
     *        timeCriteria = (a + b + c) >> 2
     * With all three bits set (GT = 0x40 | 0x80 | 0x100 = 0x1C0):
     *   a = 0x80, b = 0x40, c = 0x100 -> sum = 0x1C0 -> result = 0x70.
     */
    CHECK_EQ(time_criteria(0x40u | 0x80u | 0x100u), 0x70,
             "GT=0x1C0 -> timeCriteria=0x70");
}

static void test_time_criteria_periodic_window(void) {
    /* F0331 uses the result in two boolean gates:
     *   timeCriteria < (Wisdom + wizPriestLevel)  for mana regen
     *   timeCriteria < (Vitality + 12)            for HP healing
     * F0331 fires on *every* tick — but the boolean flips on/off as
     * the bit pattern cycles.  Over a 0x200 window (covering 0x40
     * and 0x80 mod cycles) the timeCriteria should not be constant. */
    int saw_zero = 0, saw_nonzero = 0, distinct = 0;
    uint32_t gt;
    int prev = -1;
    for (gt = 0; gt < 0x200u; ++gt) {
        int tc = (int)time_criteria(gt);
        if (tc == 0) saw_zero = 1;
        if (tc != 0) saw_nonzero = 1;
        if (tc != prev) {
            distinct++;
            prev = tc;
        }
    }
    CHECK(saw_zero && saw_nonzero,
          "time-criteria spans 0 and non-zero over 0x200 ticks");
    CHECK(distinct > 1,
          "time-criteria actually changes between adjacent ticks");
}

static void test_time_criteria_ignores_other_bits(void) {
    /* F0830 only reads 0x40/0x80/0x100.  Bits 0x01, 0x02, 0x04, 0x08,
     * 0x10, 0x20, and 0x200/0x400/0x800/etc. must not change the
     * time-criteria value. */
    uint32_t base = 0x40u | 0x80u | 0x100u; /* = 0x1C0 */
    uint32_t base_tc = time_criteria(base);
    CHECK_EQ(time_criteria(base | 0x01u),  (int)base_tc, "low bit 0x01 ignored");
    CHECK_EQ(time_criteria(base | 0x02u),  (int)base_tc, "low bit 0x02 ignored");
    CHECK_EQ(time_criteria(base | 0x04u),  (int)base_tc, "low bit 0x04 ignored");
    CHECK_EQ(time_criteria(base | 0x10u),  (int)base_tc, "low bit 0x10 ignored");
    CHECK_EQ(time_criteria(base | 0x20u),  (int)base_tc, "low bit 0x20 ignored");
    CHECK_EQ(time_criteria(base | 0x200u), (int)base_tc, "high bit 0x200 ignored");
    CHECK_EQ(time_criteria(base | 0x800u), (int)base_tc, "high bit 0x800 ignored");
}

/* ── F0846 stat-drift clock ───────────────────────────────────────── */

static void test_stat_drift_skips_non_multiple_ticks_active(void) {
    /* F0331: !((unsigned int16_t)GameTime & 255) for active.  The
     * equivalent of the period check is (gameTime % 256) == 0. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 5, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 1u,   0), 0,
             "active tick 1 not a multiple of 256 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 64u,  0), 0,
             "active tick 64 not a multiple of 256 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 128u, 0), 0,
             "active tick 128 not a multiple of 256 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 255u, 0), 0,
             "active tick 255 not a multiple of 256 -> no drift");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 5,
             "active non-multiple does not mutate stats");
}

static void test_stat_drift_skips_non_multiple_ticks_resting(void) {
    /* F0331: !((unsigned int16_t)GameTime & 63) for resting. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 5, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 1u,  1), 0,
             "resting tick 1 not a multiple of 64 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 32u, 1), 0,
             "resting tick 32 not a multiple of 64 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 48u, 1), 0,
             "resting tick 48 not a multiple of 64 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 63u, 1), 0,
             "resting tick 63 not a multiple of 64 -> no drift");
}

static void test_stat_drift_fires_on_active_multiple(void) {
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 5, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0), 1,
             "active tick 256 multiple -> drift fires");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 6,
             "below-max stat incremented to 6 after active drift");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT], 6,
             "vitality below-max stat incremented to 6 after active drift");

    /* Subsequent non-multiple ticks should not fire and not change. */
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 257u, 0), 0,
             "active tick 257 not a multiple -> no drift");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 6,
             "non-multiple tick does not change stats");
}

static void test_stat_drift_fires_on_resting_multiple(void) {
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 5, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 64u, 1), 1,
             "resting tick 64 multiple -> drift fires");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 6,
             "below-max stat incremented to 6 after resting drift");
}

static void test_stat_drift_period_doubles_when_resting(void) {
    /* The lane that distinguishes rest from active: 64 ticks is
     * 4x as fast as 256 ticks.  64 is a multiple of 64 but NOT
     * 256, so it fires only when resting. */
    struct ChampionLifecycleState_Compat champA = make_blank_champ();
    struct ChampionLifecycleState_Compat champR = make_blank_champ();
    reset_stats(&champA, 5, 30);
    reset_stats(&champR, 5, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champA, 64u, 0), 0,
             "active 64 not a multiple of 256 -> no drift");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champR, 64u, 1), 1,
             "resting 64 is a multiple of 64 -> drift fires");
}

static void test_stat_drift_above_max_clamps(void) {
    /* F0331: when curv > maxv, the stat drifts DOWN by (curv / maxv)
     * (with step at least 1).  The +1 from the recovery path must
     * not push the stat past max. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 50, 30); /* 50 > 30 */

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0), 1,
             "drift fires on multiple even when over-max");
    /* step = 50 / 30 = 1 (integer division).  curv = 50 - 1 = 49. */
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 49,
             "above-max stat decrements by curv/maxv (50/30=1)");
}

static void test_stat_drift_above_max_large_step(void) {
    /* When curv is much larger than maxv, step is bigger.  Here
     * curv = 200, maxv = 30 -> step = 200/30 = 6.  curv = 194. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 200, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0), 1,
             "drift fires on multiple with very high stat");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 194,
             "above-max stat decrements by 200/30=6");
}

static void test_stat_drift_at_max_holds(void) {
    /* ReDMCSB: when curv == maxv, the if/else if chain hits neither
     * branch; the value must not change. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 30, 30);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0), 1,
             "drift fires on multiple even at max");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 30,
             "at-max stat holds at max");
}

static void test_stat_drift_zero_max_clamps_via_step_one(void) {
    /* ReDMCSB: when maxv == 0, the step falls back to 1 so the
     * stat cannot wedge on a non-zero current. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 7, 0);

    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0), 1,
             "drift fires on multiple with zero max");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT], 6,
             "zero-max stat decrements by 1 (step fallback)");
}

static void test_stat_drift_null_champ_guard(void) {
    /* The function must early-return on a NULL champion instead of
     * dereferencing through statistics[]. */
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(0, 256u, 0), 0,
             "null champ guard returns 0");
    CHECK_EQ(F0846_LIFECYCLE_ApplyStatDrift_Compat(0, 64u,  1), 0,
             "null champ guard returns 0 (resting)");
}

static void test_stat_drift_only_modifies_current(void) {
    /* F0331 only mutates [C1_CURRENT] of each statistic.  The
     * [C0_MAXIMUM] and [C2_MINIMUM] bytes must not move. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    reset_stats(&champ, 12, 30);
    /* Tamper with max/min so we can detect any accidental write. */
    champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM] = 40;
    champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MINIMUM] = 7;

    F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0);
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM], 40,
             "drift does not mutate maximum byte");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MINIMUM], 7,
             "drift does not mutate minimum byte");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT], 13,
             "drift increments current by 1");
}

static void test_stat_drift_per_stat_independent(void) {
    /* The loop iterates over all 7 statistics (LUCK..ANTIFIRE) and
     * each one drifts from its own (max, current) pair.  A bug
     * that decouples current from max would show up as a per-stat
     * difference. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    memset(champ.statistics, 0, sizeof(champ.statistics));
    champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_MAXIMUM]      = 30;
    champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT]      = 5;
    champ.statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_MAXIMUM]   = 60;
    champ.statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_CURRENT]   = 5;
    champ.statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_MAXIMUM]  = 30;
    champ.statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_CURRENT]  = 30;
    champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM]     = 30;
    champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_CURRENT]     = 30;
    champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM]   = 30;
    champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT]   = 100; /* above max */
    champ.statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_MAXIMUM]  = 0;
    champ.statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_CURRENT]  = 9;
    champ.statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_MAXIMUM]   = 30;
    champ.statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_CURRENT]   = 30;

    F0846_LIFECYCLE_ApplyStatDrift_Compat(&champ, 256u, 0);

    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_LUCK][LIFECYCLE_STAT_CURRENT],      6,
             "LUCK below max increments by 1");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_CURRENT],   6,
             "STRENGTH below max increments by 1");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_CURRENT],  30,
             "DEXTERITY at max holds");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_CURRENT],     30,
             "WISDOM at max holds");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT],   97,
             "VITALITY above max decrements by 100/30=3");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_CURRENT],  8,
             "ANTIMAGIC zero max decrements by 1 (fallback step)");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_CURRENT],   30,
             "ANTIFIRE at max holds");
}

/* ── Entry point ──────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM1 V1 Sleep/Wakeup/Clock Gate ===\n");
    printf("ReDMCSB: CHAMPION.C F0331 time-criteria (line 2331) + stat-drift period gate (lines 2450-2473)\n");
    printf("M10:     F0830_LIFECYCLE_ComputeTimeCriteria_Compat + F0846_LIFECYCLE_ApplyStatDrift_Compat\n\n");

    /* F0830 time-criteria */
    test_time_criteria_zero_when_low_bits_clear();
    test_time_criteria_bit0x40_contribution();
    test_time_criteria_bit0x80_contribution();
    test_time_criteria_bit0x100_contribution();
    test_time_criteria_combined_bits();
    test_time_criteria_periodic_window();
    test_time_criteria_ignores_other_bits();

    /* F0846 stat-drift clock */
    test_stat_drift_skips_non_multiple_ticks_active();
    test_stat_drift_skips_non_multiple_ticks_resting();
    test_stat_drift_fires_on_active_multiple();
    test_stat_drift_fires_on_resting_multiple();
    test_stat_drift_period_doubles_when_resting();
    test_stat_drift_above_max_clamps();
    test_stat_drift_above_max_large_step();
    test_stat_drift_at_max_holds();
    test_stat_drift_zero_max_clamps_via_step_one();
    test_stat_drift_null_champ_guard();
    test_stat_drift_only_modifies_current();
    test_stat_drift_per_stat_independent();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
