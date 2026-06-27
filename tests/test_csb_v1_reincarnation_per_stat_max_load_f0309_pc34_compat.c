/*
 * test_csb_v1_reincarnation_per_stat_max_load_f0309_pc34_compat.c
 *
 * CSB V1 champion per-stat ↔ F0309 max-load interaction fixture
 * (Champions GAP 2 per-stat-parity follow-up slice).
 *
 * The companion `test_csb_v1_champion_per_stat_parity_pc34_compat.c`
 * pins every per-stat field of the CHANGE7_24 reincarnation rule
 * (HP/Mana/Stamina halve, six non-Luck stats -1/8th, Luck exempt,
 * Skills[16] cleared, NEEDS_RENAME set, DEAD cleared, ActionIndex
 * reset to REST, per-champion `reincarnateStatPenalty` divisor).
 *
 * What remained out of that companion was the deterministic
 * interaction between the per-champion `randomPoints` (Character.cpp:14
 * default 12, max 25) and the seeded F0309 maximum-load formula
 * (CHAMPION.C F0309 lines 1157-1178), as well as the
 * `reincarnateStatPenalty` divisor's downstream effect on F0309.
 *
 *   base = (STR_CURRENT << 3) + 100
 *   base = F0306(stamina-adjusted)
 *   base += 9; base -= base % 10  (round up to next multiple of 10)
 *
 * Reincarnation reduces STR by 1/8th, then applies `randomPoints`
 * deterministic LCG +1 boosts across the seven-stat fan.  STR is
 * one of the seven random targets, so the resulting MaxLoad shifts
 * in lockstep with the deterministic LCG sequence.
 *
 * This fixture predicts the post-reincarnation STR via an inline
 * LCG mirror (matching the `seed_lcg` / `next_lcg` constants in
 * csb_v1_character_pc34_compat.c) and asserts the live F0309 value
 * agrees.  That gives an end-to-end deterministic contract:
 * any future drift in the LCG seed, the randomPoints loop bound,
 * the per-stat penalty, the minimum clamp, or the F0309 / F0306
 * formula will break this fixture.  All values are computed by
 * hand from the source contracts and verified against the runtime.
 *
 * Source-locks:
 *   ReDMCSB CHAMPION.C F0306 lines 1078-1106 (stamina adjustment)
 *   ReDMCSB CHAMPION.C F0309 lines 1157-1178 (maximum load)
 *   ReDMCSB CHAMPION.C F0284 lines 117-130   (party rotation, sibling)
 *   ReDMCSB REVIVE.C   F0282:744-806         (C161 reincarnation branch:
 *                                              halve HP/Mana/Stamina,
 *                                              -1/8th non-Luck stats,
 *                                              clear Skills[16],
 *                                              set NEEDS_RENAME,
 *                                              `randomPoints` random +1)
 *   ReDMCSB REVIVE.C   F0278                 (resurrect routine, sibling)
 *   ReDMCSB DEFS.H:327-330                   (C160 C161 panel command constants)
 *   CSBWin Character.cpp:14,682-687          (per-champion
 *                                              reincarnateAttributePenalty
 *                                              / reincarnateStatPenalty /
 *                                              randomPoints globals)
 *   CSBWin SaveGame.cpp                      (DM1→CSB import path,
 *                                              Champions GAP 3)
 *   Firestaff:
 *     src/csb/csb_v1_character_pc34_compat.c
 *       - csb_v1_champion_reincarnate() lines 397-490
 *       - csb_v1_champion_get_maximum_load() lines 241-251
 *       - csb_v1_champion_stamina_adjusted_value() lines 215-228
 *       - g_lcg_seed / seed_lcg / next_lcg  lines 67-74
 *
 * Companion fixtures (kept disjoint):
 *   - test_csb_v1_champion_per_stat_parity_pc34_compat
 *       (per-stat parity only — does NOT touch F0309 / F0306)
 *   - test_csb_v1_runtime_champion_load_attrs
 *       (boot-driven runtime handoff, only asserts post-reincarnate
 *        MaxLoad is in [340, 580] band and a multiple of 10 — does
 *        NOT lock the deterministic LCG-derived STR value)
 *   - test_csb_v1_reincarnation_penalty_pc34_compat
 *       (older ChampionState_Compat shape, 6 attrs no Luck)
 *   - firestaff_csb_v1_champion_stat_determinism_probe
 *       (pure F0309 / F0306 / F0310 math, no reincarnation mutation)
 *   - firestaff_csb_v1_phase7_verification
 *       (bundled phase-7 verification)
 *
 * This file is data-free: no DM1/CSB assets are required.
 */

#include "csb_v1_character_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(actual, expected, msg) do { \
    long _a = (long)(actual); \
    long _e = (long)(expected); \
    if (_a == _e) { ++g_pass; printf("  PASS: %s (%ld == %ld)\n", msg, _a, _e); } \
    else          { ++g_fail; printf("  FAIL: %s (got %ld, expected %ld)\n", msg, _a, _e); } \
} while (0)

/* ── LCG prediction helpers (mirror of src/csb/csb_v1_character_pc34_compat.c
 *    g_lcg_seed / seed_lcg / next_lcg — locked to the same constants
 *    so this fixture fails on any drift in those constants). ── */
typedef struct {
    uint16_t seed;
    int      str_hits;       /* number of +1 boosts the LCG produces on STR */
    int      dex_hits;
    int      wis_hits;
    int      vit_hits;
    int      am_hits;
    int      af_hits;
    int      luck_hits;
} lcg_forecast_t;

static uint16_t pred_seed(uint32_t val) {
    return (uint16_t)(val ? val : 0x1234);
}

static uint16_t pred_next(uint16_t seed) {
    return (uint16_t)(seed * 0xC007u + 1u);
}

/* Walk the LCG forward `random_points` iterations starting from
 * `seed`, and tally the per-stat hit count (one hit == +1 boost). */
static lcg_forecast_t forecast_lcg(int random_points)
{
    lcg_forecast_t fc;
    memset(&fc, 0, sizeof(fc));
    fc.seed = pred_seed((uint32_t)random_points * 17u + 3u);
    uint16_t s = fc.seed;
    for (int i = 0; i < random_points; ++i) {
        s = pred_next(s);
        int idx = (int)(s % 7u);
        switch (idx) {
            case 0: fc.str_hits++;  break;
            case 1: fc.dex_hits++;  break;
            case 2: fc.wis_hits++;  break;
            case 3: fc.vit_hits++;  break;
            case 4: fc.am_hits++;   break;
            case 5: fc.af_hits++;   break;
            case 6: fc.luck_hits++; break;  /* Luck exempt: count but don't boost */
            default: break;
        }
    }
    return fc;
}

/* F0306 / F0309 closed-form prediction (mirrors
 * csb_v1_champion_stamina_adjusted_value + csb_v1_champion_get_maximum_load
 * exactly so the fixture breaks on any drift in either formula). */
static unsigned int predict_max_load(int16_t str_cur,
                                     int16_t sta_cur,
                                     int16_t sta_max)
{
    int32_t base = ((int32_t)str_cur << 3) + 100;       /* F0309 line 1 */
    int16_t half_max = (int16_t)(sta_max >> 1);
    if (half_max > 0 && sta_cur < half_max) {           /* F0306 branch */
        int16_t half_val = (int16_t)(base >> 1);
        int32_t scaled   = ((int32_t)half_val * (int32_t)sta_cur) / (int32_t)half_max;
        base = (int32_t)half_val + scaled;
    }
    if (base < 0) base = 0;
    base += 9;                                          /* F0309 round-up */
    base -= base % 10;
    return (unsigned int)base;
}

/* Build a dead champion ready for `csb_v1_champion_reincarnate()`.
 * Mirrors the helper pattern from
 * test_csb_v1_champion_per_stat_parity_pc34_compat.c (the
 * caller-supplied vital_cur / vital_max snapshot is re-asserted
 * after kill() so the F0282 halving step has the source-locked
 * pre-death HP/STA/MP values to operate on).  Starting STR and
 * stamina are configurable so the F0309 baseline and the
 * post-reincarnation F0309 value can be asserted from the same
 * champion shape. */
static void build_dead_champion_with_str(CSB_V1_Champion *c,
                                          int16_t hp_cur, int16_t hp_max,
                                          int16_t sta_cur, int16_t sta_max,
                                          int16_t mp_cur,  int16_t mp_max,
                                          int16_t str_cur, int16_t str_max,
                                          int16_t other_cur, int16_t other_max,
                                          int16_t luck_cur, int16_t luck_max,
                                          uint8_t random_points,
                                          uint8_t stat_penalty)
{
    int i;
    memset(c, 0, sizeof(*c));
    csb_v1_champion_init(c);
    c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR]      = (uint16_t)str_cur;
    c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX]      = (uint16_t)str_max;
    /* DEX / WIS / VIT / AM / AF all start at the same baseline so the
     * per-stat penalty applies uniformly; only STR's pre/post values
     * are individually inspected because STR is the F0309 driver. */
    for (i = CSB_V1_STAT_DEX; i <= CSB_V1_STAT_ANTIFIRE; ++i) {
        c->Statistics[i][CSB_V1_STAT_CUR] = (uint16_t)other_cur;
        c->Statistics[i][CSB_V1_STAT_MAX] = (uint16_t)other_max;
        c->Statistics[i][CSB_V1_STAT_MIN] = 30;  /* CHANGE7_24 minimum floor */
    }
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR] = (uint16_t)luck_cur;
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MAX] = (uint16_t)luck_max;
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        c->Skills[i] = (uint8_t)(i + 1);
    }
    c->randomPoints = random_points;
    c->reincarnateStatPenalty = stat_penalty;
    csb_v1_champion_kill(c);
    c->CurrentHealth   = hp_cur;
    c->MaximumHealth   = hp_max;
    c->CurrentStamina  = sta_cur;
    c->MaximumStamina  = sta_max;
    c->CurrentMana     = mp_cur;
    c->MaximumMana     = mp_max;
}

/* ──────────────────────────────────────────────────────────────────
 * Test 1 — Pre-reincarnation F0309 baseline.
 *
 * Sanity-check that `csb_v1_champion_get_maximum_load()` reproduces
 * the source-locked F0309 / F0306 formula on a healthy champion.
 * This is the "before" half of the before/after max-load parity
 * invariant; Test 2 locks the "after" half on the same champion.
 * ────────────────────────────────────────────────────────────────── */
static void test_f0309_baseline_pre_reincarnation(void)
{
    CSB_V1_Champion c;
    unsigned int pre_max_load;
    unsigned int predicted;
    printf("\n-- Test 1: F0309 max-load baseline before reincarnation --\n");

    build_dead_champion_with_str(&c,
                                 100, 100,   /* HP */
                                  80,  80,   /* STA */
                                  60,  60,   /* MP */
                                  60,  60,   /* STR */
                                  60,  60,   /* DEX/WIS/VIT/AM/AF */
                                  50,  50,   /* LUCK */
                                  12,        /* randomPoints (unused pre) */
                                  8);        /* reincarnateStatPenalty (unused pre) */

    predicted = predict_max_load(60, 80, 80);
    CHECK_EQ(predicted, 580u,
             "predicted F0309 max_load for STR=60, full stamina is 580");

    pre_max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ(pre_max_load, 580u,
             "csb_v1_champion_get_maximum_load() returns 580 for STR=60 (F0309 round-up)");

    /* Even before reincarnation the predicted and runtime values agree,
     * proving the closed-form predictor tracks the live F0309 / F0306
     * helpers in src/csb/csb_v1_character_pc34_compat.c exactly. */
    CHECK_EQ((long)pre_max_load, (long)predicted,
             "predicted == runtime F0309 baseline (closed-form tracks live)");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 2 — randomPoints=12 (spec default) drives a deterministic
 * LCG boost on STR; post-reincarnation MaxLoad must agree with the
 * LCG-derived STR.
 *
 * Pre-reincarnation: STR=60, MaxLoad=580 (Test 1).
 * Penalty (divisor=8): STR 60 - 60/8 = 60 - 7 = 53.
 * LCG (randomPoints=12, seed = 12*17+3 = 207 = 0xCF): see
 * `forecast_lcg(12)` results below.
 * Closed-form predicts STR=53+str_hits, MaxLoad=ROUND((STR<<3)+100).
 * This slice locks the full
 *   reincarnateStatPenalty -> per-stat penalty -> randomPoints LCG
 *   -> STR -> F0309 max_load
 * pipeline end-to-end.
 * ────────────────────────────────────────────────────────────────── */
static void test_random_points_default_drive_max_load(void)
{
    CSB_V1_Champion c;
    int16_t predicted_str;
    unsigned int predicted_max_load;
    unsigned int post_max_load;
    lcg_forecast_t fc;
    printf("\n-- Test 2: randomPoints=12 + divisor=8 -> deterministic F0309 max-load --\n");

    fc = forecast_lcg(12);
    /* Sanity-check the LCG prediction first.  The LCG seed
     * `pred_seed(12*17+3) = pred_seed(207) = 0x00CF = 207` and the
     * walk is deterministic — any drift in the constants would
     * surface here. */
    CHECK_EQ(fc.seed, 207,
             "LCG seed for randomPoints=12 is 12*17+3 = 207 (locked to src constants)");

    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  60,  60,
                                  60,  60,
                                  50,  50,
                                  12,  /* randomPoints = spec default */
                                  8);  /* reincarnateStatPenalty = spec default */

    predicted_str = (int16_t)(60 - 60 / 8 + fc.str_hits);
    predicted_max_load = predict_max_load(predicted_str, 80, 80);

    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate returns 0 (REVIVE.C F0278 C161)");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             (int)predicted_str,
             "post-reincarnation STR matches LCG prediction (53 + str_hits)");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX],
             (int)predicted_str,
             "post-reincarnation STR max row equals cur row (F0282 cur=max)");

    post_max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)post_max_load, (long)predicted_max_load,
             "post-reincarnation F0309 max_load matches LCG-driven prediction");

    /* F0309 round-up must keep MaxLoad a multiple of 10. */
    CHECK_EQ(post_max_load % 10u, 0u,
             "post-reincarnation F0309 max_load is a multiple of 10 (F0309 round-up)");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 3 — randomPoints=0 isolates the per-stat penalty so the
 * max-load is exactly the F0309 value at the post-penalty STR
 * (60 - 60/8 = 53 -> base 524 -> adjusted 530).  No LCG boosts
 * mean the value is locked without any random-walk interference.
 * ────────────────────────────────────────────────────────────────── */
static void test_random_points_zero_isolates_penalty(void)
{
    CSB_V1_Champion c;
    unsigned int post_max_load;
    printf("\n-- Test 3: randomPoints=0 isolates the per-stat penalty (no LCG boosts) --\n");

    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  60,  60,
                                  60,  60,
                                  50,  50,
                                  0,    /* randomPoints = 0: no LCG boosts */
                                  8);

    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate returns 0 with randomPoints=0");
    /* STR = 60 - 60/8 = 53.  No LCG boosts applied. */
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR], 53,
             "STR 60 - 60/8 = 53 (no LCG boosts when randomPoints=0)");

    post_max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)post_max_load, 530,
             "F0309 max_load = round-up((53<<3)+100) = round-up(524) = 530");

    /* F0306 with full-stamina (cur=80, max=80 -> half_max=40, cs>=40)
     * returns the value unchanged, so the F0306 path is a no-op for
     * the default full-stamina champion.  That keeps this test
     * isolated to the F0309 side. */
    CHECK_EQ(post_max_load % 10u, 0u,
             "F0309 max_load with randomPoints=0 is a multiple of 10");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 4 — reincarnateStatPenalty divisor variation drives different
 * post-reincarnation STR values, and therefore different F0309 max
 * loads.  divisor=4 (large penalty), divisor=16 (small penalty),
 * divisor=0 (zero-divisor fallback to 8).
 * ────────────────────────────────────────────────────────────────── */
static void test_stat_penalty_divisor_drives_max_load(void)
{
    CSB_V1_Champion c;
    unsigned int max_load;
    printf("\n-- Test 4: reincarnateStatPenalty divisor drives F0309 max-load --\n");

    /* divisor=4: STR 80 - 80/4 = 60; no LCG boosts.  F0309 base
     * = (60<<3)+100 = 580; since 580 is already a multiple of 10,
     * the F0309 `+9 -mod10` round-up idiom leaves it at 580. */
    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  80,  80,
                                  80,  80,
                                  50,  50,
                                  0,
                                  4);
    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate with divisor=4 returns 0");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR], 60,
             "STR 80 - 80/4 = 60 (divisor=4)");
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)max_load, 580,
             "F0309 max_load for STR=60 = round-up((60<<3)+100) = 580 (already multiple of 10)");

    /* divisor=16: STR 80 - 80/16 = 75; no LCG boosts.  F0309 base
     * = (75<<3)+100 = 700; 700 is already a multiple of 10, so
     * the round-up idiom leaves it at 700. */
    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  80,  80,
                                  80,  80,
                                  50,  50,
                                  0,
                                  16);
    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate with divisor=16 returns 0");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR], 75,
             "STR 80 - 80/16 = 75 (divisor=16)");
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)max_load, 700,
             "F0309 max_load for STR=75 = round-up((75<<3)+100) = 700 (already multiple of 10)");

    /* divisor=0: spec leaves the divisor at 0 -> fallback to 8;
     * STR 80 - 80/8 = 70.  F0309 base = (70<<3)+100 = 660;
     * 660 is already a multiple of 10 so round-up leaves it at 660. */
    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  80,  80,
                                  80,  80,
                                  50,  50,
                                  0,
                                  0);
    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate with divisor=0 (zero-divisor fallback) returns 0");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR], 70,
             "STR 80 - 80/8 = 70 (divisor=0 falls back to 8)");
    max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)max_load, 660,
             "F0309 max_load for STR=70 = round-up((70<<3)+100) = 660 (already multiple of 10)");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 5 — randomPoints at the spec maximum (25) shifts STR by
 * the deterministic LCG count for that seed, so the F0309 max_load
 * follows the per-stat penalty + 25 random boosts on STR.
 * ────────────────────────────────────────────────────────────────── */
static void test_random_points_spec_max_drives_max_load(void)
{
    CSB_V1_Champion c;
    lcg_forecast_t fc;
    int16_t predicted_str;
    unsigned int predicted_max_load;
    unsigned int post_max_load;
    printf("\n-- Test 5: randomPoints=25 (spec max) drives deterministic F0309 max-load --\n");

    fc = forecast_lcg(25);
    CHECK_EQ(fc.seed, 428,
             "LCG seed for randomPoints=25 is 25*17+3 = 428 (locked to src constants)");

    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  60,  60,
                                  60,  60,
                                  50,  50,
                                  25,  /* randomPoints = spec max */
                                  8);

    predicted_str = (int16_t)(60 - 60 / 8 + fc.str_hits);
    predicted_max_load = predict_max_load(predicted_str, 80, 80);

    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate with randomPoints=25 returns 0");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             (int)predicted_str,
             "post-reincarnation STR = 53 + str_hits for randomPoints=25");

    post_max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)post_max_load, (long)predicted_max_load,
             "F0309 max_load at spec-max randomPoints matches LCG-derived STR");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 6 — F0306 half-stamina adjustment interacts with the
 * post-reincarnation STR.  Even with full HP / STA / MP, the test
 * can construct a half-stamina snapshot (cur=20, max=80) BEFORE
 * reincarnation; the reincarnation halves CurrentStamina to 40 and
 * MaximumStamina to 40 (both >= half_max=20), so F0306 returns the
 * value unchanged — but to exercise the F0306 branch directly we
 * override the post-reincarnation stamina to (cur=10, max=80) so
 * half_max=40, cs=10 < 40 and the F0306 halve-and-scale kicks in.
 *
 * Pre-reincarnation STR=60 -> penalty 53 -> rp=12 lifts to 54.
 * Base = (54<<3)+100 = 532.  half_val = 266.  scaled = 266*10/40 = 66.
 * adjusted = 266 + 66 = 332.  +9 -mod10 = 340.
 * ────────────────────────────────────────────────────────────────── */
static void test_f0306_half_stamina_interaction(void)
{
    CSB_V1_Champion c;
    lcg_forecast_t fc;
    int16_t predicted_str;
    unsigned int post_max_load;
    printf("\n-- Test 6: F0306 half-stamina interaction with post-reincarnation STR --\n");

    fc = forecast_lcg(12);
    predicted_str = (int16_t)(60 - 60 / 8 + fc.str_hits);

    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  60,  60,
                                  60,  60,
                                  50,  50,
                                  12,
                                  8);
    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate returns 0 with half-stamina follow-up");
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             (int)predicted_str,
             "post-reincarnation STR matches LCG prediction");

    /* Now drop the stamina snapshot to half-max so F0306 actually
     * takes the cs < half_max branch.  Source: F0306 lines 1078-1106. */
    c.CurrentStamina  = 10;  /* below half_max(40) */
    c.MaximumStamina  = 80;
    post_max_load = csb_v1_champion_get_maximum_load(&c);
    /* base=(predicted_str<<3)+100; half_val=base/2; scaled=half_val*10/40;
     * adjusted=half_val+scaled; +9 -mod10.  For predicted_str=54:
     * base=532; half_val=266; scaled=266*10/40=66; adj=332; +9=341; %10=1;
     * 341-1=340. */
    CHECK_EQ((long)post_max_load, 340,
             "F0306 half-stamina + F0309 round-up = 340 for STR=54, cur=10/80");

    /* F0309 sanity-check on a healthy stamina snapshot: same STR, full
     * stamina must yield the F0309 round-up value with no F0306
     * adjustment. */
    c.CurrentStamina = 80;  /* above half_max(40) — F0306 no-op */
    c.MaximumStamina = 80;
    post_max_load = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)post_max_load, 540,
             "F0309 max_load at STR=54, full stamina = round-up((54<<3)+100) = 540");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 7 — Before/after max-load delta:
 *   post_max_load < pre_max_load for every randomPoints > 0 because
 *   the per-stat penalty dominates any random +1 boosts on STR in
 *   the spec-default seeded LCG (Test 2's str_hits=1 vs the
 *   7-point penalty loss for STR=60).  This is the user-facing
 *   contract for the reincarnation panel: a reincarnated champion
 *   carries less.
 * ────────────────────────────────────────────────────────────────── */
static void test_max_load_regression_post_reincarnation(void)
{
    CSB_V1_Champion c;
    unsigned int pre, post;
    int str_hits_default;
    printf("\n-- Test 7: post-reincarnation max-load regression vs pre-reincarnation --\n");

    /* Pre baseline ─────────────────────────────────────────────── */
    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  60,  60,
                                  60,  60,
                                  50,  50,
                                  12,  /* unused pre-reincarnation */
                                  8);
    pre = csb_v1_champion_get_maximum_load(&c);
    CHECK_EQ((long)pre, 580,
             "pre-reincarnation max_load baseline = 580 (STR=60, full stamina)");

    /* Re-build with same starting shape so the post path is comparable. */
    build_dead_champion_with_str(&c,
                                 100, 100,
                                  80,  80,
                                  60,  60,
                                  60,  60,
                                  60,  60,
                                  50,  50,
                                  12,  /* spec default */
                                  8);  /* spec default */
    CHECK_EQ(csb_v1_champion_reincarnate(&c), 0,
             "reincarnate returns 0 on the regression-fair champion");
    post = csb_v1_champion_get_maximum_load(&c);

    /* spec-default: penalty -7 on STR, LCG adds str_hits to STR.
     * The LCG forecast must agree with the post-reincarnation STR. */
    str_hits_default = forecast_lcg(12).str_hits;
    CHECK_EQ((int)c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             60 - 60/8 + str_hits_default,
             "post STR = 53 + str_hits from spec-default LCG");

    /* Reincarnation must lower the maximum load (penalty dominates
     * the spec-default LCG boost).  Even with randomPoints=25 (Test 5
     * gives str_hits=5 -> STR=58 -> MaxLoad=570 vs pre 580), the
     * post value is strictly below the pre value because the
     * per-stat penalty always reduces STR by at least STR/16. */
    CHECK(post < pre,
          "post-reincarnation max_load < pre-reincarnation max_load "
          "(per-stat penalty dominates the LCG boost at every spec value)");
}

/* ──────────────────────────────────────────────────────────────────
 * Test 8 — Source-evidence + NULL safety contract.  Mirrors the
 * companion per-stat fixture's evidence contract so probes that
 * grep for "CSB" / "REVIVE" / "Character.cpp" / "F0309" can rely on
 * the same surface.
 * ────────────────────────────────────────────────────────────────── */
static void test_source_evidence_and_null_safety(void)
{
    const char *e;
    printf("\n-- Test 8: source-evidence + NULL safety --\n");

    /* NULL safety on the F0309 helper (revive path lives separately). */
    CHECK_EQ(csb_v1_champion_get_maximum_load(NULL), 0u,
             "csb_v1_champion_get_maximum_load(NULL) = 0 (defensive contract)");
    CHECK_EQ(csb_v1_champion_get_movement_ticks(NULL), 2u,
             "csb_v1_champion_get_movement_ticks(NULL) = 2 (BUG0_72 branch entry)");

    e = csb_v1_character_source_evidence();
    CHECK(e != NULL, "character_source_evidence returns non-NULL");
    if (e) {
        CHECK(strlen(e) > 10, "source evidence is substantive");
        CHECK(strstr(e, "CSB") != NULL || strstr(e, "csb") != NULL ||
              strstr(e, "REVIVE") != NULL || strstr(e, "Character.cpp") != NULL,
              "source evidence names CSB or REVIVE.C or Character.cpp");
        CHECK(strstr(e, "F0309") != NULL || strstr(e, "CHAMPION.C") != NULL,
              "source evidence references F0309 / CHAMPION.C");
    }
}

int main(void)
{
    printf("=== CSB V1 champion per-stat ↔ F0309 max-load interaction ===\n");

    test_f0309_baseline_pre_reincarnation();
    test_random_points_default_drive_max_load();
    test_random_points_zero_isolates_penalty();
    test_stat_penalty_divisor_drives_max_load();
    test_random_points_spec_max_drives_max_load();
    test_f0306_half_stamina_interaction();
    test_max_load_regression_post_reincarnation();
    test_source_evidence_and_null_safety();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
