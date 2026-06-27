/*
 * test_csb_v1_reincarnation_penalty_pc34_compat.c
 *
 * Source-faithful CSB V1 Reincarnation Penalty (Champions GAP 2,
 * CHANGE7_24) over the ChampionState_Compat surface.
 *
 * ReDMCSB: REVIVE.C lines 809-822 (CHANGE7_24_IMPROVEMENT):
 *   - HP / MP / STA current and maximum halved (right-shift by 1, source-
 *     equivalent to divide by 2 with round-down).
 *   - For each non-Luck stat (C1..C6 per DEFS.H:745-750):
 *     one_eighth = Statistics[i][C1_CURRENT] >> 3;
 *     Statistics[i][C1_CURRENT] = Statistics[i][C0_MAXIMUM]
 *         = GetMaximumValue(Statistics[i][C2_MINIMUM],
 *                            Statistics[i][C1_CURRENT]
 *                            - one_eighth);
 *   - The C0_STATISTIC_LUCK row is skipped by the loop bounds
 *     (DEFS.H:744); ChampionState_Compat.attributes[] has 6
 *     non-Luck slots so Luck exemption is implicit.
 *
 * ReDMCSB: Character.cpp:14 — reincarnateStatPenalty default 8
 *   corresponds to `>>3`.  This shim honors the global by
 *   using `cur / statPenalty` so callers can override the
 *   divisor.  The compat floor is 0 because ChampionState
 *   attributes never go negative and there is no
 *   per-stat minimum row in ChampionState_Compat.
 *
 * This test is the data-free ChampionState_Compat surface
 * companion to `test_csb_v1_champion_per_stat_parity_pc34_compat`
 * (which exercises the modern CSB_V1_Champion surface over the
 * same CHANGE7_24 contract).
 */
#include "csb_v1_reincarnation_penalty_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static void init_typical_champion(struct ChampionState_Compat* c) {
    memset(c, 0, sizeof(*c));
    c->present        = 1;
    c->hp.maximum         = 100;
    c->hp.current         = 20;   /* catches source halving of current, not only max clamp */
    c->stamina.maximum    = 80;
    c->stamina.current    = 60;
    c->mana.maximum       = 60;
    c->mana.current       = 45;
    /* attributes: STR=18, DEX=18, WIS=18, VIT=18, AntiMagic=18, AntiFire=18, no Luck slot. */
    for (int i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        c->attributes[i]         = 18;
        c->attributeMaximums[i]  = 18;
    }
}

int main(void) {
    struct ChampionState_Compat champ;

    printf("=== CSB V1 reincarnation penalty (CHANGE7_24, ChampionState_Compat surface) ===\n");

    /* ---- DM1 baseline: mode disabled preserves stats ---- */
    csb_v1_reincarnation_mode_set(0);
    CHECK(csb_v1_reincarnation_mode_get() == 0,
          "default reincarnation mode is disabled");

    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.hp.maximum == 100,
          "DM1: hp.max preserved when mode off");
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 18,
          "DM1: Strength preserved when mode off");

    /* ---- CSB mode on: source-locked CHANGE7_24 contract ----
     * Per ReDMCSB REVIVE.C lines 809-815 + 815-822:
     *   - HP/Mana/Stamina max halved (>>1)
     *   - Per-stat: cur = max = cur - (cur >> 3) = cur - (cur/8)
     *     For 18: 18 - floor(18/8) = 18 - 2 = 16
     *     For 100 hp: 100 / 2 = 50
     */
    csb_v1_reincarnation_mode_set(1);
    CHECK(csb_v1_reincarnation_mode_get() == 1,
          "reincarnation mode can be enabled");

    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);

    /* HP / Mana / Stamina current and maximum halved (REVIVE.C lines 815-822). */
    CHECK(champ.hp.maximum == 50,
          "CSB: hp.maximum halved (100 -> 50) per ReDMCSB REVIVE.C MEDIA265");
    CHECK(champ.stamina.maximum == 40,
          "CSB: stamina.maximum halved (80 -> 40) per ReDMCSB REVIVE.C MEDIA265");
    CHECK(champ.mana.maximum == 30,
          "CSB: mana.maximum halved (60 -> 30) per ReDMCSB REVIVE.C MEDIA265");
    CHECK(champ.hp.current == 10,
          "CSB: hp.current halved (20 -> 10) per ReDMCSB REVIVE.C MEDIA265");
    CHECK(champ.stamina.current == 30,
          "CSB: stamina.current halved (60 -> 30) per ReDMCSB REVIVE.C MEDIA265");
    CHECK(champ.mana.current == 22,
          "CSB: mana.current halved with floor (45 -> 22) per ReDMCSB REVIVE.C MEDIA265");

    /* Per-stat 1/8th reduction (REVIVE.C lines 811-812).
     * 18 - floor(18/8) = 18 - 2 = 16. */
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH]  == 16,
          "CSB: Strength reduced 1/8th (18 -> 16) per ReDMCSB REVIVE.C MEDIA332");
    CHECK(champ.attributes[CHAMPION_ATTR_DEXTERITY] == 16,
          "CSB: Dexterity reduced 1/8th (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_WISDOM]    == 16,
          "CSB: Wisdom reduced 1/8th (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_VITALITY]  == 16,
          "CSB: Vitality reduced 1/8th (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_ANTIMAGIC] == 16,
          "CSB: AntiMagic reduced 1/8th (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_ANTIFIRE]  == 16,
          "CSB: AntiFire reduced 1/8th (18 -> 16)");

    /* Both current and maximum set to the same value (REVIVE.C line 812). */
    CHECK(champ.attributeMaximums[CHAMPION_ATTR_STRENGTH]  == 16,
          "CSB: attributeMaximums[STR] set to new current value (REVIVE.C line 812)");
    CHECK(champ.attributeMaximums[CHAMPION_ATTR_DEXTERITY] == 16,
          "CSB: attributeMaximums[DEX] set to new current value");
    CHECK(champ.attributeMaximums[CHAMPION_ATTR_ANTIFIRE]  == 16,
          "CSB: attributeMaximums[ANTIFIRE] set to new current value");

    /* ---- Toggling back to DM1 leaves a fresh champion untouched ---- */
    csb_v1_reincarnation_mode_set(0);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.hp.maximum == 100,
          "toggle off leaves fresh champion untouched");
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 18,
          "toggle off leaves attributes untouched");

    /* ---- Custom statPenalty=4 (Character.cpp:14 scaling global) ----
     * 18 - floor(18/4) = 18 - 4 = 14.  This case distinguishes the
     * source-locked 1/N reduction from a flat subtraction. */
    CSB_V1_ReincarnationConfig_Compat cfg = { 2, 4, 3 };
    csb_v1_reincarnation_config(&cfg);
    csb_v1_reincarnation_mode_set(1);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 14,
          "CSB: custom statPenalty=4 reduces Strength 18 -> 14 (1/4th per Character.cpp:14)");
    CHECK(champ.attributeMaximums[CHAMPION_ATTR_STRENGTH] == 14,
          "CSB: statPenalty=4 sets attributeMaximums[STR] to 14");

    /* ---- Custom statPenalty=16: 18 - floor(18/16) = 18 - 1 = 17 ---- */
    cfg.statPenalty = 16;
    csb_v1_reincarnation_config(&cfg);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 17,
          "CSB: statPenalty=16 reduces Strength 18 -> 17 (1/16th)");

    /* ---- Floor clamp: statPenalty=1 reduces 18 -> 0 ---- */
    cfg.statPenalty = 1;
    csb_v1_reincarnation_config(&cfg);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 0,
          "CSB: statPenalty=1 reduces Strength 18 -> 0 (full subtraction, floor clamp)");

    /* ---- Divisor zero is sanitised to 1 (avoid divide-by-zero) ---- */
    cfg.statPenalty = 0;
    csb_v1_reincarnation_config(&cfg);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 0,
          "CSB: statPenalty=0 sanitised to 1 (no divide-by-zero, full subtraction)");

    /* ---- NULL cfg resets to Character.cpp:14 defaults ---- */
    csb_v1_reincarnation_config(NULL);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 16,
          "CSB: NULL config resets statPenalty=8 default (18 -> 16)");

    /* ---- NULL champion is a safe no-op ---- */
    csb_v1_reincarnation_penalty_apply(NULL);
    CHECK(1, "NULL champion is a safe no-op");

    /* ---- Re-apply: source rule runs again on already-penalized values ---- */
    csb_v1_reincarnation_mode_set(1);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    /* Second apply: each attribute is 16, so 16 - floor(16/8) = 16 - 2 = 14. */
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 14,
          "CSB: second apply on 16 reduces to 14 (16 - 16/8)");
    CHECK(champ.hp.current == 5,
          "CSB: second apply halves hp.current 10 -> 5");
    CHECK(champ.hp.maximum == 25,
          "CSB: second apply halves hp.maximum 50 -> 25");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
