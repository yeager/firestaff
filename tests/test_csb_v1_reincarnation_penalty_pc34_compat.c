/*
 * test_csb_v1_reincarnation_penalty_pc34_compat.c
 *
 * CSB V1 Reincarnation Penalty (Champions GAP 2, CHANGE7_24).
 * Source-locked per CSB:REVIVE.C CHANGE7_24 + Character.cpp:14
 * (reincarnateAttributePenalty=2, reincarnateStatPenalty=8,
 * randomPoints=3).  HP/MP/STA halved, other stats
 * -attributePenalty, Luck exempt.
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
    c->stamina.maximum    = 80;
    c->mana.maximum       = 60;
    /* attributes: STR=18, DEX=18, WIS=18, VIT=18, AntiMagic=18, AntiFire=18, no Luck slot. */
    for (int i = 0; i < CHAMPION_ATTR_COUNT; ++i) {
        c->attributes[i] = 18;
    }
}

int main(void) {
    struct ChampionState_Compat champ;

    printf("=== CSB V1 reincarnation penalty (CHANGE7_24) ===\n");

    /* Default: mode disabled (DM1 behaviour, stats preserved). */
    csb_v1_reincarnation_mode_set(0);
    CHECK(csb_v1_reincarnation_mode_get() == 0,
          "default reincarnation mode is disabled");

    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.hp.maximum == 100,
          "DM1: hp.max preserved when mode off");
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 18,
          "DM1: Strength preserved when mode off");

    /* CSB: mode on, penalty applied. */
    csb_v1_reincarnation_mode_set(1);
    CHECK(csb_v1_reincarnation_mode_get() == 1,
          "reincarnation mode can be enabled");

    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    /* HP halved. */
    CHECK(champ.hp.maximum == 50,
          "CSB: hp.max halved (100 -> 50)");
    /* Each non-Luck stat reduced by attributePenalty=2. */
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH]  == 16,
          "CSB: Strength reduced by 2 (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_DEXTERITY] == 16,
          "CSB: Dexterity reduced by 2 (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_WISDOM]    == 16,
          "CSB: Wisdom reduced by 2 (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_VITALITY]  == 16,
          "CSB: Vitality reduced by 2 (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_ANTIMAGIC] == 16,
          "CSB: AntiMagic reduced by 2 (18 -> 16)");
    CHECK(champ.attributes[CHAMPION_ATTR_ANTIFIRE]  == 16,
          "CSB: AntiFire reduced by 2 (18 -> 16)");

    /* Toggling back to DM1 leaves a fresh champion untouched. */
    csb_v1_reincarnation_mode_set(0);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.hp.maximum == 100,
          "toggle off leaves fresh champion untouched");

    /* Custom attributePenalty (e.g. 4). */
    CSB_V1_ReincarnationConfig_Compat cfg = { 4, 8, 3 };
    csb_v1_reincarnation_config(&cfg);
    csb_v1_reincarnation_mode_set(1);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 14,
          "CSB: custom attributePenalty=4 reduces Strength 18 -> 14");

    /* Clamp at 0. */
    cfg.attributePenalty = 100;
    csb_v1_reincarnation_config(&cfg);
    init_typical_champion(&champ);
    csb_v1_reincarnation_penalty_apply(&champ);
    CHECK(champ.attributes[CHAMPION_ATTR_STRENGTH] == 0,
          "large penalty clamps stat to 0");

    /* NULL cfg resets to defaults. */
    csb_v1_reincarnation_config(NULL);
    CHECK(csb_v1_reincarnation_mode_get() == 1,
          "config reset does not change mode");

    /* NULL champion is a safe no-op. */
    csb_v1_reincarnation_penalty_apply(NULL);
    CHECK(1, "NULL champion is a safe no-op");

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
