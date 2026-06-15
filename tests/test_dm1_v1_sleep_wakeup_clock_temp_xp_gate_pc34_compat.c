/*
 * test_dm1_v1_sleep_wakeup_clock_temp_xp_gate_pc34_compat.c
 *
 * DM1 V1 sleep/wakeup/clock gate — focused on the F0331 *per-tick
 * skill temporary-experience decay* slice that the existing F0830/F0846
 * clock gate (pass777) and the F0831/F0832 stamina + hunger/thirst gate
 * do not cover.  The companion pass777 gate pins the period-gated
 * (TimeCriteria bit pattern + 256/64 tick stat-drift cadence) and the
 * companion F0331 stamina gate pins the (maxStamina>>8)-1 + idle-bonus
 * stamina-amount contract plus the F0331 inner do-while food/water
 * cycle.  This gate pins the *per-tick temporary-experience decay* slice
 * inside the F0331 body:
 *
 *   F0847_LIFECYCLE_ApplyTemporaryXPDecay_Compat — the per-tick loop
 *      `for (i = 0; i < LIFECYCLE_SKILL_COUNT; i++)` that decrements
 *      every positive `skills20[i].temporaryExperience` by exactly 1.
 *      ReDMCSB CHAMPION.C F0331 lines 2362-2368 (the
 *      `C19_SKILL_WATER .. C00_SKILL_FIGHTER` loop that pre-dates the
 *      CSB-style ascending iteration).
 *
 * Lane contract (pass-NNN [dm1-v1-sleep-wakeup-clock-temp-xp-gate]):
 *   - Pin F0847's per-tick "every positive temp-experience field is
 *     decremented by exactly 1" contract.
 *   - Pin F0847's per-tick "zero temp-experience fields are left
 *     unchanged at 0" guard (the `> 0` test, NOT `>= 0`).
 *   - Pin F0847's per-tick "negative temp-experience fields are left
 *     unchanged" guard (the `> 0` test, NOT `!= 0`).
 *   - Pin F0847's null-champ guard (returns 0, no field touched).
 *   - Pin F0847's per-skill independence: only the positive fields
 *     move, every other field is byte-stable, and the
 *     LIFECYCLE_SKILL_COUNT = 20 lane is exercised for every skill
 *     index 0..19.
 *   - Pin the iterative decay contract over a chain of 4 consecutive
 *     ticks: a positive field reaches 0 in 4 ticks and stays 0
 *     thereafter, with no negative overshoot.
 *   - Pin the symmetry between the F0847 (ascending) iteration and
 *     the ReDMCSB F0331 (descending) iteration: the *net* field
 *     outcome must be identical, even though the order of writes
 *     differs.
 *   - Pin the LIFECYCLE_TEMP_XP_CAP = 32000 ceiling clamp is *not*
 *     applied by F0847 itself (F0847 is a pure decay; the cap is
 *     enforced by F0849 when adding experience).
 *   - Pin the int16_t saturation safety: even with a tempExperience
 *     value at the int16_t max (INT16_MAX = 32767), the per-tick
 *     decrement is exact and does not wrap.
 *
 * This is contract-only and source-locked to ReDMCSB CHAMPION.C F0331
 * (PC 3.4 MEDIA240 path).  It does not claim original DOS parity.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

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
    long long a_ = (long long)(actual); \
    long long e_ = (long long)(expected); \
    if (a_ == e_) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL %s: got %lld expected %lld\n", (msg), a_, e_); \
    } \
} while (0)

/* ── Helpers ──────────────────────────────────────────────────────── */

static struct ChampionLifecycleState_Compat make_blank_champ(void) {
    struct ChampionLifecycleState_Compat champ;
    memset(&champ, 0, sizeof(champ));
    return champ;
}

static int apply_decay(struct ChampionLifecycleState_Compat* champ) {
    return F0847_LIFECYCLE_ApplyTemporaryXPDecay_Compat(champ);
}

static void set_temp_xp(struct ChampionLifecycleState_Compat* champ,
                        int skill, int16_t value) {
    if (skill < 0 || skill >= LIFECYCLE_SKILL_COUNT) return;
    champ->skills20[skill].temporaryExperience = value;
}

static int16_t get_temp_xp(const struct ChampionLifecycleState_Compat* champ,
                           int skill) {
    if (skill < 0 || skill >= LIFECYCLE_SKILL_COUNT) return 0;
    return champ->skills20[skill].temporaryExperience;
}

/* ── Null-champ guard ─────────────────────────────────────────────── */

static void test_decay_null_champ_returns_zero(void) {
    /* ReDMCSB F0331 + F0847: a null champion pointer is a no-op
     * and reports failure (0).  The F0331 caller does not check the
     * return value, but the contract is still "do not crash on null". */
    int rc = F0847_LIFECYCLE_ApplyTemporaryXPDecay_Compat(0);
    CHECK_EQ(rc, 0, "null champ returns 0");
}

/* ── Per-skill field independence ────────────────────────────────── */

static void test_decay_zero_field_untouched(void) {
    /* F0847 guard: `if (temp > 0) temp--`.  Zero is not positive, so
     * zero fields stay zero after one decay tick. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    int i;
    /* All fields start at 0. */
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ(get_temp_xp(&champ, i), 0, "all skills start at 0");
    }
    int rc = apply_decay(&champ);
    CHECK_EQ(rc, 1, "decay with all-zero temps returns 1");
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ(get_temp_xp(&champ, i), 0, "zero temp stays zero after decay");
    }
}

static void test_decay_negative_field_untouched(void) {
    /* F0847 guard: `if (temp > 0) temp--`.  Negative values are not
     * positive, so negative fields stay at their negative value
     * (they are not clamped to 0 either). */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    int i;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        set_temp_xp(&champ, i, (int16_t)-100);
    }
    int rc = apply_decay(&champ);
    CHECK_EQ(rc, 1, "decay with all-negative temps returns 1");
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ(get_temp_xp(&champ, i), -100,
                 "negative temp stays negative after decay");
    }
}

static void test_decay_positive_field_decremented_by_one(void) {
    /* F0847 contract: every positive field is decremented by
     * exactly 1, not by 2, not by 0, not by the entire value. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    int i;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        set_temp_xp(&champ, i, (int16_t)50);
    }
    int rc = apply_decay(&champ);
    CHECK_EQ(rc, 1, "decay with all-50 temps returns 1");
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ(get_temp_xp(&champ, i), 49,
                 "positive 50 temp decremented to 49 after decay");
    }
}

static void test_decay_field_of_one_reaches_zero(void) {
    /* F0847 boundary: a field at 1 is positive, so it becomes 0
     * after one decay tick. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER, (int16_t)1);
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER), 0,
             "temp=1 -> temp=0 after one decay tick");
}

static void test_decay_field_of_two_reaches_one(void) {
    /* F0847 boundary: a field at 2 becomes 1, not 0. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_WATER, (int16_t)2);
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WATER), 1,
             "temp=2 -> temp=1 after one decay tick");
}

static void test_decay_does_not_overshoot_zero(void) {
    /* F0847 guard: a positive field becomes 0, never -1. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST, (int16_t)1);
    apply_decay(&champ);
    apply_decay(&champ);
    apply_decay(&champ);
    /* Three decay ticks on a field of 1: 1->0, 0->0, 0->0. */
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST), 0,
             "temp=1 -> 0 -> 0 -> 0, no negative overshoot");
}

/* ── Per-skill independence across the full 0..19 skill range ────── */

static void test_decay_only_positive_fields_change(void) {
    /* F0847 contract: per-skill independence.  Mixed positive/zero/
     * negative fields: only the positive ones move. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER,  (int16_t)100);
    set_temp_xp(&champ, LIFECYCLE_SKILL_NINJA,    (int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST,   (int16_t)200);
    set_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD,   (int16_t)-50);
    set_temp_xp(&champ, LIFECYCLE_SKILL_SWING,    (int16_t)1);
    set_temp_xp(&champ, LIFECYCLE_SKILL_THRUST,   (int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_CLUB,     (int16_t)7);
    set_temp_xp(&champ, LIFECYCLE_SKILL_PARRY,    (int16_t)-1);
    set_temp_xp(&champ, LIFECYCLE_SKILL_STEAL,    (int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHT,    (int16_t)42);
    set_temp_xp(&champ, LIFECYCLE_SKILL_THROW,    (int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_SHOOT,    (int16_t)3);
    set_temp_xp(&champ, LIFECYCLE_SKILL_IDENTIFY, (int16_t)-7);
    set_temp_xp(&champ, LIFECYCLE_SKILL_HEAL,     (int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_INFLUENCE,(int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_DEFEND,   (int16_t)11);
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIRE,     (int16_t)0);
    set_temp_xp(&champ, LIFECYCLE_SKILL_AIR,      (int16_t)5);
    set_temp_xp(&champ, LIFECYCLE_SKILL_EARTH,    (int16_t)-99);
    set_temp_xp(&champ, LIFECYCLE_SKILL_WATER,    (int16_t)0);

    int rc = apply_decay(&champ);
    CHECK_EQ(rc, 1, "decay returns 1 on a mixed field");

    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER),  99,
             "F=100->99");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_NINJA),    0,
             "NINJA=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST),   199,
             "P=200->199");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD),   -50,
             "WIZ=-50->-50");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_SWING),    0,
             "SW=1->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_THRUST),   0,
             "TH=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_CLUB),     6,
             "C=7->6");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_PARRY),    -1,
             "PA=-1->-1");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_STEAL),    0,
             "S=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHT),    41,
             "F=42->41");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_THROW),    0,
             "T=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_SHOOT),    2,
             "SH=3->2");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_IDENTIFY), -7,
             "I=-7->-7");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_HEAL),     0,
             "H=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_INFLUENCE),0,
             "INF=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_DEFEND),   10,
             "D=11->10");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIRE),     0,
             "FI=0->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_AIR),      4,
             "A=5->4");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_EARTH),    -99,
             "E=-99->-99");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WATER),    0,
             "W=0->0");
}

/* ── Iterative decay over multiple ticks ──────────────────────────── */

static void test_decay_iterative_chain_reaches_zero(void) {
    /* F0847 chain: a field at 5 should reach 0 after exactly 5
     * decay ticks and stay 0 on tick 6+. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD, (int16_t)5);

    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 5,
             "tick 0: temp=5");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 4,
             "tick 1: temp=4");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 3,
             "tick 2: temp=3");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 2,
             "tick 3: temp=2");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 1,
             "tick 4: temp=1");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 0,
             "tick 5: temp=0");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 0,
             "tick 6: temp=0 stays 0");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WIZARD), 0,
             "tick 7: temp=0 stays 0");
}

static void test_decay_iterative_chain_independent_per_skill(void) {
    /* F0847 chain: each positive skill decays independently, the
     * slower ones stay positive while the faster ones reach 0. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER, (int16_t)1);
    set_temp_xp(&champ, LIFECYCLE_SKILL_NINJA,   (int16_t)3);
    set_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST,  (int16_t)5);

    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER), 0,
             "tick 1: F=1->0");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_NINJA),   2,
             "tick 1: N=3->2");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST),  4,
             "tick 1: P=5->4");

    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER), 0,
             "tick 2: F=0->0 (no negative overshoot)");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_NINJA),   1,
             "tick 2: N=2->1");
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_PRIEST),  3,
             "tick 2: P=4->3");
}

static void test_decay_iterative_chain_15_ticks(void) {
    /* F0847 chain: a field at 15 should reach 0 after exactly 15
     * decay ticks. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIRE, (int16_t)15);
    int i;
    for (i = 0; i < 15; ++i) {
        apply_decay(&champ);
    }
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIRE), 0,
             "field=15 reaches 0 after 15 decay ticks");
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIRE), 0,
             "field=0 stays 0 on tick 16");
}

/* ── ReDMCSB F0331 symmetry: ascending iteration vs descending ───── */

static void test_decay_all_positive_full_range(void) {
    /* F0847 ascending iteration: every skill 0..19 is visited.
     * F0331 (ReDMCSB) uses a *descending* iteration (C19..C00) but
     * the *net* field outcome must be identical.  This gate pins
     * the ascending iteration end-state, not the iteration order. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    int i;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        set_temp_xp(&champ, i, (int16_t)(i + 1));
    }
    /* Verify pre-state. */
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ(get_temp_xp(&champ, i), i + 1, "pre-state i+1");
    }
    apply_decay(&champ);
    /* Every positive field decremented by 1. */
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ(get_temp_xp(&champ, i), i,
                 "all-positive full range: i+1 -> i after one tick");
    }
}

/* ── int16_t saturation safety ───────────────────────────────────── */

static void test_decay_int16_max_safety(void) {
    /* F0847 safety: a tempExperience of INT16_MAX = 32767 must
     * decrement to INT16_MAX - 1 = 32766, not wrap to -32768. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER, (int16_t)INT16_MAX);
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER),
             (long long)INT16_MAX - 1L,
             "INT16_MAX -> INT16_MAX-1, no wrap");
}

static void test_decay_int16_min_safety(void) {
    /* F0847 guard: INT16_MIN = -32768 is not positive, so it stays
     * at INT16_MIN (no clamp, no wrap, no sign flip). */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_WATER, (int16_t)INT16_MIN);
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_WATER),
             (long long)INT16_MIN,
             "INT16_MIN stays INT16_MIN (not positive, not clamped)");
}

static void test_decay_lifecycle_temp_xp_cap_not_applied(void) {
    /* F0847 contract: F0847 is pure decay.  The LIFECYCLE_TEMP_XP_CAP
     * (32000) is enforced by F0849 (AddSkillExperience) when
     * *adding* experience, NOT by F0847.  This gate pins that F0847
     * does not silently clamp the upper bound. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_AIR, (int16_t)(LIFECYCLE_TEMP_XP_CAP - 1));
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_AIR),
             (long long)(LIFECYCLE_TEMP_XP_CAP - 2),
             "F0847 does not clamp to LIFECYCLE_TEMP_XP_CAP");
}

/* ── Reuse after full zero (decrement-only path) ──────────────────── */

static void test_decay_reuse_after_full_zero(void) {
    /* F0847 contract: once a field reaches 0, subsequent decay
     * ticks must keep it at 0 (no negative overshoot) so the
     * caller can re-use the same field later by F0849 (AddSkill
     * Experience). */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER, (int16_t)3);
    apply_decay(&champ);
    apply_decay(&champ);
    apply_decay(&champ);
    /* F=0 now.  Caller adds 50 via F0849. */
    set_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER, (int16_t)50);
    apply_decay(&champ);
    CHECK_EQ(get_temp_xp(&champ, LIFECYCLE_SKILL_FIGHTER), 49,
             "field reaches 0, caller re-uses it, F0847 still decrements by 1");
}

/* ── skill-experience (not temporary) is not touched ──────────────── */

static void test_decay_does_not_touch_experience_field(void) {
    /* F0847 contract: only temporaryExperience is decremented.
     * The (separately-stored) `experience` field stays at its
     * pre-tick value. */
    struct ChampionLifecycleState_Compat champ = make_blank_champ();
    int i;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        champ.skills20[i].experience = (int32_t)(1000 + i);
        set_temp_xp(&champ, i, (int16_t)5);
    }
    apply_decay(&champ);
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; ++i) {
        CHECK_EQ((long long)champ.skills20[i].experience,
                 (long long)(1000 + i),
                 "experience field is byte-stable across F0847");
        CHECK_EQ(get_temp_xp(&champ, i), 4,
                 "temporaryExperience is decremented by 1");
    }
}

/* ── Run all ──────────────────────────────────────────────────────── */

int main(void) {
    /* Null-champ guard. */
    test_decay_null_champ_returns_zero();

    /* Per-skill field independence. */
    test_decay_zero_field_untouched();
    test_decay_negative_field_untouched();
    test_decay_positive_field_decremented_by_one();
    test_decay_field_of_one_reaches_zero();
    test_decay_field_of_two_reaches_one();
    test_decay_does_not_overshoot_zero();

    /* Per-skill independence across the full 0..19 skill range. */
    test_decay_only_positive_fields_change();

    /* Iterative decay over multiple ticks. */
    test_decay_iterative_chain_reaches_zero();
    test_decay_iterative_chain_independent_per_skill();
    test_decay_iterative_chain_15_ticks();

    /* ReDMCSB F0331 symmetry: ascending vs descending iteration. */
    test_decay_all_positive_full_range();

    /* int16_t saturation safety. */
    test_decay_int16_max_safety();
    test_decay_int16_min_safety();
    test_decay_lifecycle_temp_xp_cap_not_applied();

    /* Reuse after full zero. */
    test_decay_reuse_after_full_zero();

    /* skill-experience (not temporary) is not touched. */
    test_decay_does_not_touch_experience_field();

    printf("test_dm1_v1_sleep_wakeup_clock_temp_xp_gate_pc34_compat: %d passed, %d failed\n",
           g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
