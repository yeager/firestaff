/*
 * test_csb_v1_champion_per_stat_parity_pc34_compat.c
 *
 * CSB V1 champion per-stat parity fixture (Champions GAP 2 + GAP 3).
 *
 * Source-locked against ReDMCSB REVIVE.C CHANGE7_24 (CSB reincarnation
 * rules) + Character.cpp:14,682-687 (per-champion globals
 * `reincarnateAttributePenalty` / `reincarnateStatPenalty` /
 * `randomPoints`) + CSBWin Character.cpp:14 (the three globals live
 * in the champion record, not as globals on a per-game basis, but
 * the CSB V1 champion struct mirrors the defaults 2/8/3 the spec
 * documents).
 *
 * The earlier `csb_v1_reincarnation_penalty_apply` compat shim
 * (src/csb/csb_v1_reincarnation_penalty_pc34_compat.c) operates on
 * the older `ChampionState_Compat` shape with a 6-attribute array
 * that does not include Luck.  This probe targets the newer
 * `csb_v1_champion_reincarnate()` entry point (REVIVE.C F0278 +
 * C161_COMMAND_CLICK_IN_PANEL_REINCARNATE) which has the full 7-stat
 * Statistics array (STR/DEX/WIS/VIT/ANTIMAGIC/ANTIFIRE/LUCK per
 * `CSB_V1_STAT_*`) and the per-champion `reincarnateStatPenalty`
 * divisor that the spec mandates (`-1/8th of current value`).
 *
 * The slice pins per-stat parity for every field the CHANGE7_24
 * spec mentions, so the gate catches any future drift in
 * `csb_v1_champion_reincarnate()` that would break DM1→CSB
 * champion transfer (Champions GAP 3 / CEDTINC8.C:101-118) or the
 * Hall-of-Champions reincarnation panel (DEFS.H:327-330 C161 +
 * REVIVE.C F0282:744-806).
 *
 * Source-locks:
 *   ReDMCSB REVIVE.C F0278 F0281 (CSB reincarnation routine)
 *   ReDMCSB DEFS.H:757-768 (C00..C19 skill slots, class indices)
 *   ReDMCSB DEFS.H:821-826 (M027/M028 portrait atlas math — N/A here)
 *   ReDMCSB DEFS.H:327-330 (C160 C161 panel command constants)
 *   ReDMCSB REVIVE.C F0282:744-806 (C161 reincarnation branch:
 *                                    halve HP/Mana/Stamina,
 *                                    reduce max by 1/8th,
 *                                    Luck exempt,
 *                                    clear Skills[16],
 *                                    set NEEDS_RENAME,
 *                                    12 random +1 stat boosts)
 *   CSBWin Character.cpp:14,682-687 (per-champion
 *                                    reincarnateAttributePenalty /
 *                                    reincarnateStatPenalty /
 *                                    randomPoints)
 *   CSBWin SaveGame.cpp DM1→CSB import (Champions GAP 3)
 *   Firestaff: src/csb/csb_v1_character_pc34_compat.c
 *              csb_v1_champion_reincarnate() lines 397-490
 *
 * Companion fixtures:
 *   - test_csb_v1_reincarnation_penalty_pc34_compat (older
 *     ChampionState_Compat shape, 6 attrs no Luck, attrPenalty
 *     semantics)
 *   - test_csb_v1_runtime_champion_load_attrs (covers HP halved,
 *     post-reincarnate max load band — broader runtime handoff)
 *   - firestaff_csb_v1_champion_stat_determinism_probe (F0309/F0310
 *     max-load + movement-tick determinism, not reincarnation)
 *
 * This file is data-free: no DM1/CSB assets are required.
 */

#include "csb_v1_character_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

/* Build a deterministic dead champion with the given stat table.
 * `vital_cur` / `vital_max` are the pre-reincarnate Current/Maximum
 * Health/Stamina/Mana values.  stat_cur / stat_max are the per-stat
 * values (Statistics[idx][CUR] / Statistics[idx][MAX]).
 *
 * The minimum for every stat is fixed at 30 (matches ReDMCSB
 * champion creation defaults).  The champion is killed via
 * csb_v1_champion_kill() so the reincarnation guard fires; the
 * caller-supplied vital_cur / vital_max values are then re-asserted
 * so the subsequent `csb_v1_champion_reincarnate()` has the
 * source-locked "pre-death HP" snapshot to halve (csb_v1_champion_kill
 * intentionally zeroes CurrentHealth per the ReDMCSB V1 panel-display
 * contract; in CSB the pre-death values are normally restored by the
 * combat handler before the reincarnation panel can open).  This
 * matches the established pattern in test_csb_v1_runtime_champion_load_attrs.c.
 *
 * `random_points` controls how many +1 boosts the random walk adds
 * after the deterministic penalty; pass 0 to isolate the penalty
 * semantics, 12 for the spec default. */
static void build_dead_champion(CSB_V1_Champion *c,
                                int16_t hp_cur, int16_t hp_max,
                                int16_t sta_cur, int16_t sta_max,
                                int16_t mp_cur,  int16_t mp_max,
                                int16_t str_cur, int16_t str_max,
                                int16_t dex_cur, int16_t dex_max,
                                int16_t wis_cur, int16_t wis_max,
                                int16_t vit_cur, int16_t vit_max,
                                int16_t am_cur,  int16_t am_max,
                                int16_t af_cur,  int16_t af_max,
                                int16_t luck_cur, int16_t luck_max,
                                uint8_t random_points)
{
    int i;
    memset(c, 0, sizeof(*c));
    csb_v1_champion_init(c);
    c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR]      = (uint16_t)str_cur;
    c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX]      = (uint16_t)str_max;
    c->Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR]      = (uint16_t)dex_cur;
    c->Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_MAX]      = (uint16_t)dex_max;
    c->Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR]      = (uint16_t)wis_cur;
    c->Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_MAX]      = (uint16_t)wis_max;
    c->Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_CUR]      = (uint16_t)vit_cur;
    c->Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_MAX]      = (uint16_t)vit_max;
    c->Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_CUR] = (uint16_t)am_cur;
    c->Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_MAX] = (uint16_t)am_max;
    c->Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_CUR]  = (uint16_t)af_cur;
    c->Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_MAX]  = (uint16_t)af_max;
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR]     = (uint16_t)luck_cur;
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MAX]     = (uint16_t)luck_max;
    /* Pre-populate skills so we can verify they get cleared. */
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        c->Skills[i] = (uint8_t)(i + 1);
    }
    c->randomPoints = random_points;
    csb_v1_champion_kill(c);
    /* Re-assert the caller-supplied vital snapshot so the F0282
     * halving step has the source-locked pre-death HP / STA / MP
     * values to operate on.  csb_v1_champion_kill() intentionally
     * zeroes CurrentHealth for the panel display, but combat
     * snapshots the pre-death values (LastHealth/LastStamina/LastMana)
     * and the reincarnation panel restores them before F0282 runs. */
    c->CurrentHealth   = hp_cur;
    c->MaximumHealth   = hp_max;
    c->CurrentStamina  = sta_cur;
    c->MaximumStamina  = sta_max;
    c->CurrentMana     = mp_cur;
    c->MaximumMana     = mp_max;
}

/* The fixture exercise focuses on a known champion with:
 *   HP   100/100  → halved 50/50
 *   STA   80/80   → halved 40/40
 *   MP    60/60   → halved 30/30
 *   STR   60/60   → 60 - 60/8 = 60 - 7 = 53   (stat_penalty=8)
 *   DEX   60/60   → 53
 *   WIS   60/60   → 53
 *   VIT   60/60   → 53
 *   AM    60/60   → 53
 *   AF    60/60   → 53
 *   LUCK  50/50   → preserved at 50 (CHANGE7_24 exempts Luck)
 *   randomPoints = 0 so the per-stat penalty is deterministic.
 */
static void test_per_stat_parity_at_default_scaling(void)
{
    CSB_V1_Champion c;
    printf("\n-- Test 1: per-stat parity at default reincarnateStatPenalty=8 --\n");

    build_dead_champion(&c,
                        100, 100,    /* HP */
                         80,  80,    /* STA */
                         60,  60,    /* MP */
                         60,  60,    /* STR */
                         60,  60,    /* DEX */
                         60,  60,    /* WIS */
                         60,  60,    /* VIT */
                         60,  60,    /* ANTIMAGIC */
                         60,  60,    /* ANTIFIRE */
                         50,  50,    /* LUCK */
                         0);         /* randomPoints */

    CHECK(csb_v1_champion_is_dead(&c) == 1,
          "fixture: champion is dead after csb_v1_champion_kill()");
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate returns 0 on success (REVIVE.C F0278 C161)");

    /* ── Vitals: HP / STA / MP all halved (REVIVE.C F0282:744-806) ── */
    CHECK_EQ(c.CurrentHealth,    50, "HP current  100 -> 50 (F0282 halve)");
    CHECK_EQ(c.MaximumHealth,    50, "HP max      100 -> 50 (F0282 halve)");
    CHECK_EQ(c.CurrentStamina,   40, "STA current  80 -> 40 (F0282 halve)");
    CHECK_EQ(c.MaximumStamina,   40, "STA max      80 -> 40 (F0282 halve)");
    CHECK_EQ(c.CurrentMana,      30, "MP  current  60 -> 30 (F0282 halve)");
    CHECK_EQ(c.MaximumMana,      30, "MP  max      60 -> 30 (F0282 halve)");

    /* ── Per-stat 1/8th reduction on the six non-Luck stats ── */
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             53, "STR 60 - 60/8 = 53 (CHANGE7_24 -1/8th current)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX],
             53, "STR max row also rewritten to 53 (F0282 cur=max)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR],
             53, "DEX 60 - 60/8 = 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_MAX],
             53, "DEX max row also rewritten to 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR],
             53, "WIS 60 - 60/8 = 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_MAX],
             53, "WIS max row also rewritten to 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_CUR],
             53, "VIT 60 - 60/8 = 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_MAX],
             53, "VIT max row also rewritten to 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_CUR],
             53, "ANTIMAGIC 60 - 60/8 = 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_MAX],
             53, "ANTIMAGIC max row also rewritten to 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_CUR],
             53, "ANTIFIRE 60 - 60/8 = 53");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_MAX],
             53, "ANTIFIRE max row also rewritten to 53");

    /* ── Luck: CHANGE7_24 explicitly exempts Luck from the penalty ── */
    CHECK_EQ(c.Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR],
             50, "LUCK preserved at 50 (CHANGE7_24 exempts Luck)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MAX],
             50, "LUCK max preserved at 50");

    /* ── Skill array: every slot zeroed (F0282 F0008_MAIN_ClearBytes) ── */
    {
        int i;
        int cleared = 1;
        for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
            if (c.Skills[i] != 0) { cleared = 0; break; }
        }
        CHECK(cleared,
              "all 16 CSB_V1_SKILL_COUNT skill slots cleared (F0282 F0008)");
    }

    /* ── Status attributes: DEAD cleared, NEEDS_RENAME set ── */
    CHECK_EQ(csb_v1_champion_is_dead(&c), 0,
             "DEAD attribute cleared after reincarnate");
    CHECK(c.Attributes & CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME,
          "NEEDS_RENAME attribute set (F0281_CHAMPION_Rename prompt)");

    /* ── Action resets to REST so the reincarnated champion is ready ── */
    CHECK_EQ(c.ActionIndex, CSB_V1_ACTION_REST,
             "ActionIndex reset to REST after reincarnate");
}

/* The minimum-floor invariant: if the 1/8th subtraction would push
 * a stat below `min`, the value is clamped to `min`.  This is the
 * source-locked behaviour from REVIVE.C F0282 (`cur = max =
 * GetMaximum(stat_min, cur - cur/8)`).
 *
 * Fixture: STR=32, stat_min=30, divisor=8 → 32 - 32/8 = 32 - 4 = 28.
 *          28 < min(30), so the clamp pushes STR back to 30.
 */
static void test_per_stat_parity_respects_minimum(void)
{
    CSB_V1_Champion c;
    printf("\n-- Test 2: minimum-floor invariant (cur - cur/8 clamped to min) --\n");

    build_dead_champion(&c,
                        40, 40,    /* HP */
                        40, 40,    /* STA */
                        40, 40,    /* MP */
                        32, 32,    /* STR (under-test) */
                        60, 60,    /* DEX */
                        60, 60,    /* WIS */
                        60, 60,    /* VIT */
                        60, 60,    /* AM  */
                        60, 60,    /* AF  */
                        50, 50,    /* LUCK */
                        0);

    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate returns 0");

    /* Without the floor, STR would be 28.  The min=30 clamp holds. */
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             30, "STR 32 - 32/8 = 28 clamped to min 30 (F0282 floor)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX],
             30, "STR max row also clamped to min 30");
    /* Sanity: surrounding stats still follow the 1/8th rule. */
    CHECK_EQ(c.Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR],
             53, "DEX 60 - 60/8 = 53 (1/8th rule still holds for unaffected stats)");
}

/* Different reincarnateStatPenalty values produce the expected
 * per-stat reduction (4 → 1/4th, 16 → 1/16th).  This pins the
 * `c->reincarnateStatPenalty` global-to-struct migration from the
 * Character.cpp:14 spec. */
static void test_per_stat_parity_respects_per_champion_divisor(void)
{
    CSB_V1_Champion c;
    printf("\n-- Test 3: per-champion reincarnateStatPenalty divisor --\n");

    /* divisor=4: STR 80 - 80/4 = 80 - 20 = 60 */
    build_dead_champion(&c,
                        40, 40,
                        40, 40,
                        40, 40,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        50, 50,
                        0);
    c.reincarnateStatPenalty = 4;
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate with divisor=4 returns 0");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             60, "STR 80 - 80/4 = 60 (reincarnateStatPenalty=4)");

    /* divisor=16: STR 80 - 80/16 = 80 - 5 = 75 */
    build_dead_champion(&c,
                        40, 40,
                        40, 40,
                        40, 40,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        50, 50,
                        0);
    c.reincarnateStatPenalty = 16;
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate with divisor=16 returns 0");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             75, "STR 80 - 80/16 = 75 (reincarnateStatPenalty=16)");

    /* divisor=0: spec leaves the divisor at 0 → fallback to 8 to
     * avoid a divide-by-zero.  With STR=80 the fallback gives
     * 80 - 80/8 = 80 - 10 = 70. */
    build_dead_champion(&c,
                        40, 40,
                        40, 40,
                        40, 40,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        80, 80,
                        50, 50,
                        0);
    c.reincarnateStatPenalty = 0;
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate with divisor=0 (zero-divisor fallback) returns 0");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             70, "STR 80 - 80/8 = 70 (divisor=0 falls back to 8)");
}

/* Already-alive champion: reincarnate must be a no-op success
 * (the C161 button is greyed out when the champion is alive per
 * DEFS.H:327-330).  This pins the early-return contract so the
 * command-dispatch chain can rely on `reincarnate() == 0` meaning
 * "no change" on an alive champion. */
static void test_reincarnate_already_alive_is_noop(void)
{
    CSB_V1_Champion c;
    int i;
    uint16_t expected_str;
    printf("\n-- Test 4: reincarnate on already-alive champion is a no-op success --\n");

    build_dead_champion(&c,
                        100, 100,
                         80,  80,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         50,  50,
                         0);
    /* undo the kill so the champion is alive */
    c.Attributes &= ~(uint16_t)CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    expected_str = c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR];

    CHECK(csb_v1_champion_is_dead(&c) == 0,
          "precondition: champion is alive (DEAD attribute cleared)");
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate returns 0 on alive champion (no-op success)");
    CHECK_EQ(c.CurrentHealth, 100,
             "HP current preserved at 100 (no-op on alive champion)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             (long)expected_str,
             "STR current preserved (no-op on alive champion)");
    /* skills untouched */
    {
        int intact = 1;
        for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
            if (c.Skills[i] != (uint8_t)(i + 1)) { intact = 0; break; }
        }
        CHECK(intact,
              "skills preserved on alive champion (no-op reincarnate)");
    }
    CHECK((c.Attributes & CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME) == 0,
          "NEEDS_RENAME NOT set on alive-champion no-op reincarnate");
}

/* NULL safety contract: both the per-champion reincarnate() and the
 * older ChampionState_Compat reincarnation_penalty_apply() must
 * tolerate NULL so command-dispatch never crashes on a missing
 * party slot. */
static void test_reincarnate_null_safety(void)
{
    CSB_V1_Champion c;
    printf("\n-- Test 5: NULL safety contract --\n");

    CHECK(csb_v1_champion_reincarnate(NULL) == -1,
          "reincarnate(NULL) returns -1 (defensive contract)");
    /* Non-dead champion with non-zero health still passes through
     * `is_dead` → `return 0` (no-op success). */
    memset(&c, 0, sizeof(c));
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate(zeroed, alive) returns 0 (zero-stat guard)");
    CHECK(csb_v1_champion_kill(NULL) == -1,
          "kill(NULL) returns -1 (defensive contract)");
    CHECK(csb_v1_champion_resurrect(NULL) == -1,
          "resurrect(NULL) returns -1 (defensive contract)");
    CHECK(csb_v1_champion_is_dead(NULL) == 0,
          "is_dead(NULL) returns 0 (defensive contract)");
}

/* Resurrect-vs-reincarnate parity: resurrection (C160) preserves
 * stats; reincarnation (C161) applies the penalty.  Both clear the
 * DEAD attribute.  This pins the panel-button contract so the
 * Hall-of-Champions C040 / F0282 dispatch can route the right
 * command. */
static void test_resurrect_versus_reincarnate_panel_contract(void)
{
    CSB_V1_Champion c;
    int i;
    printf("\n-- Test 6: resurrect (C160) preserves, reincarnate (C161) penalises --\n");

    /* Resurrect path ─────────────────────────────────────────────── */
    build_dead_champion(&c,
                        100, 100,
                         80,  80,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         50,  50,
                         0);
    CHECK(csb_v1_champion_resurrect(&c) == 0,
          "resurrect (C160) returns 0");
    CHECK(csb_v1_champion_is_dead(&c) == 0,
          "resurrect clears DEAD attribute");
    CHECK_EQ(c.CurrentHealth, 100,
             "resurrect: HP current restored to max 100 (no penalty)");
    CHECK_EQ(c.MaximumHealth, 100,
             "resurrect: HP max preserved at 100 (no penalty)");
    CHECK_EQ(c.CurrentStamina, 80,
             "resurrect: STA current restored to max 80");
    CHECK_EQ(c.MaximumStamina, 80,
             "resurrect: STA max preserved at 80");
    CHECK_EQ(c.CurrentMana, 60,
             "resurrect: MP current restored to max 60");
    CHECK_EQ(c.MaximumMana, 60,
             "resurrect: MP max preserved at 60");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             60, "resurrect: STR preserved at 60 (no -1/8th)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR],
             50, "resurrect: LUCK preserved at 50");
    /* skills preserved */
    {
        int intact = 1;
        for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
            if (c.Skills[i] != (uint8_t)(i + 1)) { intact = 0; break; }
        }
        CHECK(intact,
              "resurrect: skills preserved (no F0008_MAIN_ClearBytes)");
    }
    CHECK((c.Attributes & CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME) == 0,
          "resurrect: NEEDS_RENAME NOT set (no F0281_CHAMPION_Rename)");

    /* Reincarnate path ───────────────────────────────────────────── */
    build_dead_champion(&c,
                        100, 100,
                         80,  80,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         60,  60,
                         50,  50,
                         0);
    CHECK(csb_v1_champion_reincarnate(&c) == 0,
          "reincarnate (C161) returns 0");
    CHECK_EQ(c.CurrentHealth, 50,
             "reincarnate: HP current 100 -> 50 (CHANGE7_24 halve)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             53, "reincarnate: STR 60 - 60/8 = 53 (CHANGE7_24 -1/8th)");
    CHECK_EQ(c.Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR],
             50, "reincarnate: LUCK preserved at 50 (CHANGE7_24 exempts)");
    CHECK((c.Attributes & CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME) != 0,
          "reincarnate: NEEDS_RENAME set (F0281_CHAMPION_Rename)");
}

/* Source-evidence string contract: the reincarnation path is
 * declared as CSB CHANGE7_24 / REVIVE.C F0278 / F0281 / F0282 so
 * any probe or test that imports the evidence string can grep it
 * for the same tags. */
static void test_source_evidence_citation(void)
{
    const char *e = csb_v1_character_source_evidence();
    printf("\n-- Test 7: source-evidence citation contract --\n");
    CHECK(e != NULL, "character_source_evidence returns non-NULL");
    if (e) {
        CHECK(strlen(e) > 10, "source evidence is substantive");
        CHECK(strstr(e, "CSB") != NULL || strstr(e, "csb") != NULL ||
              strstr(e, "REVIVE") != NULL || strstr(e, "Character.cpp") != NULL,
              "source evidence names CSB or REVIVE.C or Character.cpp");
    }
}

int main(void)
{
    printf("=== CSB V1 champion per-stat parity (CHANGE7_24) ===\n");

    test_per_stat_parity_at_default_scaling();
    test_per_stat_parity_respects_minimum();
    test_per_stat_parity_respects_per_champion_divisor();
    test_reincarnate_already_alive_is_noop();
    test_reincarnate_null_safety();
    test_resurrect_versus_reincarnate_panel_contract();
    test_source_evidence_citation();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
