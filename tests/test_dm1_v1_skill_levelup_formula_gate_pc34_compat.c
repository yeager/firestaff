/*
 * test_dm1_v1_skill_levelup_formula_gate_pc34_compat.c
 *
 * Focused DM1 V1 (PC 3.4) practice-based level-up regression.
 *
 * This gate is intentionally narrow: it locks ONE deterministic
 * champion/action fixture (War Cry → Parry / Influence) and verifies
 * the source-locked formula gates from
 *   - CHAMPION.C F0303 (XP → level via "while >= 500 halve" + base/sub avg)
 *   - CHAMPION.C F0304 (XP routing: sub skill XP propagates to base skill)
 *   - MENU.C:382, 427  (G0496 / G0497 PC 3.4 = Atari ST 1.2+ tables)
 *   - MENU.C:949, 987  (War Cry secondary 12 XP → C14_SKILL_INFLUENCE)
 *
 * It does NOT touch balance numbers, action names, or any other action
 * beyond War Cry; it only checks that ONE clean end-to-end exercise of
 * the practice-XP path crosses the level-up threshold at exactly the
 * source-locked XP amounts.
 */
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) do { \
    g_tests_run++; \
    printf("  TEST: %s ... ", #name); \
} while(0)

#define PASS() do { \
    g_tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define FAIL(msg) do { \
    printf("FAIL: %s\n", msg); \
    return 1; \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        char buf[256]; \
        snprintf(buf, sizeof(buf), "%s == %s (got %d, expected %d)", \
                 #a, #b, (int)(a), (int)(b)); \
        FAIL(buf); \
    } \
} while(0)

#define ASSERT_STREQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        char buf[256]; \
        snprintf(buf, sizeof(buf), "%s == %s (got '%s', expected '%s')", \
                 #a, #b, (a), (b)); \
        FAIL(buf); \
    } \
} while(0)

#define ASSERT_GE(a, b) do { \
    if (!((a) >= (b))) { \
        char buf[256]; \
        snprintf(buf, sizeof(buf), "%s >= %s (got %d, threshold %d)", \
                 #a, #b, (int)(a), (int)(b)); \
        FAIL(buf); \
    } \
} while(0)

/* ── Test 1: GRAPHIC560 G0496/G0497 fixture for War Cry ────────────── */
static int test_war_cry_route_from_graphic560(void) {
    TEST(war_cry_route_from_graphic560);

    /* ReDMCSB MENU.C:382 G0496_auc_Graphic560_ActionSkillIndex[8] = 7 (PARRY)
     *                MENU.C:427 G0497_auc_Graphic560_ActionExperienceGain[8] = 7
     * PC 3.4 inherits Atari ST 1.2+ branch (MENU.C:397 commentary). */
    DM1_ActionXpRoute route;
    int ok = dm1_v1_action_xp_route(DM1_ACTION_WAR_CRY, &route);
    ASSERT_EQ(ok, 1);
    ASSERT_EQ(route.valid, 1);
    ASSERT_EQ(route.skillIndex, DM1_SKILL_IDX_PARRY);
    ASSERT_EQ(route.baseSkillIndex, DM1_SKILL_IDX_FIGHTER);
    ASSERT_EQ(route.experienceGain, 7);

    PASS();
    return 0;
}

/* ── Test 2: Secondary INFLUENCE grant from War Cry fright path ────── */
static int test_war_cry_secondary_influence_grant(void) {
    TEST(war_cry_secondary_influence_grant);

    /* ReDMCSB MENU.C:949 commentary + MENU.C:987 F0304_CHAMPION_AddSkillExperience
     * call: War Cry fright path grants an extra 12 XP to C14_SKILL_INFLUENCE,
     * in addition to the primary 7 XP to C07_SKILL_PARRY from G0497. This is
     * the only action in DM1 v1.2+ that grants XP in two skills. */
    ASSERT_EQ(DM1_WAR_CRY_SECONDARY_INFLUENCE_XP, 12);

    /* Drain the fixture through the existing level-up API to verify the
     * secondary grant integrates cleanly with the level-up formula. */
    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 0xC0DE0001u;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 0;  /* War Cry is not a combat-sub-skill:
                                     * C14 is a hidden priest sub-skill, the
                                     * "swing..shoot" combat-window halving at
                                     * CHAMPION.C:860 does NOT apply. */
    ctx.gameTime = 1000;
    ctx.partyIsResting = 0;

    /* Apply the 12 XP fright grant to INFLUENCE (C14). */
    DM1_LevelUpBonuses b_inf = dm1_skill_add_experience(
        &state, DM1_SKILL_IDX_INFLUENCE, DM1_WAR_CRY_SECONDARY_INFLUENCE_XP, &ctx, &rng);

    /* F0303 CHAMPION.C:755-768: hidden-skill level uses (base+sub)/2.
     * Priest (C02) starts at 0, INFLUENCE (C14) at 12 → average 6 → still
     * level 1 (below 500 threshold). */
    int levelPriest = dm1_skill_get_level(&state, DM1_SKILL_IDX_PRIEST,
                                          DM1_SKILL_FLAG_IGNORE_TEMP);
    ASSERT_EQ(levelPriest, 1);
    int levelInfluence = dm1_skill_get_level(&state, DM1_SKILL_IDX_INFLUENCE,
                                             DM1_SKILL_FLAG_IGNORE_TEMP);
    ASSERT_EQ(levelInfluence, 1);

    /* No level-up occurred → stat bonuses all zero. */
    ASSERT_EQ(b_inf.maxManaDelta, 0);
    ASSERT_EQ(b_inf.wisdomDelta, 0);

    /* The Priest base skill should NOT receive XP because INFLUENCE is a
     * sub-skill; F0304 line 891-893 propagates sub→base only for SWING..SHOOT
     * style "hidden sub-skill" range. F0303 (level calc) DOES average sub
     * and base, but F0304 (XP grant) only adds to base if the granted skill
     * is a sub-skill — which INFLUENCE (C14) IS. So base priest should
     * receive 12. */
    int32_t priestXp = dm1_skill_get_experience(&state, DM1_SKILL_IDX_PRIEST, 0);
    ASSERT_EQ(priestXp, 12);

    PASS();
    return 0;
}

/* ── Test 3: Combined War Cry primary + secondary routing ──────────── */
static int test_war_cry_combined_practice_routing(void) {
    TEST(war_cry_combined_practice_routing);

    /* ReDMCSB MENU.C:947-987 (fright) + MENU.C:1627 (action XP dispatch):
     * a single War Cry action emits TWO F0304 calls with different
     * (skill, xp) tuples: (PARRY, 7) and (INFLUENCE, 12). */
    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 0xC0DE0002u;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 900;  /* 100 ticks before gameTime: inside
                                       * the 150-tick window at
                                       * CHAMPION.C:860 (so no halving) AND
                                       * outside the 25-tick recent window at
                                       * CHAMPION.C:883 (so no doubling).
                                       * This isolates the practice XP
                                       * formula from combat-window modifiers. */
    ctx.gameTime = 1000;
    ctx.partyIsResting = 0;

    /* Look up the routing for WAR CRY through the GRAPHIC560 fixture. */
    DM1_ActionXpRoute route;
    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_WAR_CRY, &route), 1);

    /* Apply primary XP grant from G0497. */
    DM1_LevelUpBonuses b_parry = dm1_skill_add_experience(
        &state, route.skillIndex, route.experienceGain, &ctx, &rng);
    /* Apply secondary fright-path grant from MENU.C:949. */
    DM1_LevelUpBonuses b_inf = dm1_skill_add_experience(
        &state, DM1_SKILL_IDX_INFLUENCE, DM1_WAR_CRY_SECONDARY_INFLUENCE_XP, &ctx, &rng);

    /* F0304 propagates the primary 7 XP from PARRY (sub) into FIGHTER (base). */
    int32_t parryXp = dm1_skill_get_experience(&state, DM1_SKILL_IDX_PARRY, 0);
    int32_t fighterXp = dm1_skill_get_experience(&state, DM1_SKILL_IDX_FIGHTER, 0);
    int32_t influenceXp = dm1_skill_get_experience(&state, DM1_SKILL_IDX_INFLUENCE, 0);
    int32_t priestXp = dm1_skill_get_experience(&state, DM1_SKILL_IDX_PRIEST, 0);
    ASSERT_EQ(parryXp, 7);
    ASSERT_EQ(fighterXp, 7);
    ASSERT_EQ(influenceXp, 12);
    ASSERT_EQ(priestXp, 12);

    /* No level-up yet: 7 < 500 and 12 < 500. */
    ASSERT_EQ(b_parry.maxHealthDelta, 0);
    ASSERT_EQ(b_parry.strengthDelta, 0);
    ASSERT_EQ(b_parry.dexterityDelta, 0);
    ASSERT_EQ(b_inf.maxManaDelta, 0);
    ASSERT_EQ(b_inf.wisdomDelta, 0);

    /* Fighter level = 1 (raw 7 XP), PARRY level uses F0303 average (7+7)/2 = 7 → still 1.
     * Priest = 1 (raw 12), INFLUENCE = (12+12)/2 = 12 → still 1. */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER,
                                  DM1_SKILL_FLAG_IGNORE_TEMP), 1);
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_PARRY,
                                  DM1_SKILL_FLAG_IGNORE_TEMP), 1);
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_PRIEST,
                                  DM1_SKILL_FLAG_IGNORE_TEMP), 1);
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_INFLUENCE,
                                  DM1_SKILL_FLAG_IGNORE_TEMP), 1);

    PASS();
    return 0;
}

/* ── Test 4: Practice gate crosses level-up threshold exactly at 500 ─ */
static int test_practice_gate_crosses_levelup_at_500(void) {
    TEST(practice_gate_crosses_levelup_at_500);

    /* ReDMCSB CHAMPION.C F0303 line 765-768:
     *   L0908_i_SkillLevel = 1;
     *   while (L0907_Experience >= 500) { L0907_Experience >>= 1; L0908_i_SkillLevel++; }
     * Source-locked threshold: at exactly 500 XP, level moves 1 → 2.
     *
     * We model a champion who has been practicing the PARRY skill by
     * repeatedly performing War Cry. 7 XP per War Cry × 72 = 504 XP,
     * which is the smallest integer N such that 7*N >= 500. The 71st
     * War Cry brings PARRY to 497 (still level 1); the 72nd pushes it
     * to 504 (level 2) and propagates 504 to FIGHTER. */
    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 0xC0DE0003u;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 0;  /* 100 ticks before gameTime: outside
                                     * the 25-tick recent window at
                                     * CHAMPION.C:883 (so no doubling) AND
                                     * outside the 150-tick stale window at
                                     * CHAMPION.C:860 (so no halving).
                                     * Practice XP stays at exactly 7 per
                                     * War Cry. */
    ctx.gameTime = 100;
    ctx.partyIsResting = 0;

    /* At t=0: 0 XP, level 1. */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 1);
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_PARRY, 0), 1);

    /* 71 War Crys: 71 * 7 = 497 XP. */
    for (int i = 0; i < 71; ++i) {
        dm1_skill_add_experience(&state, DM1_SKILL_IDX_PARRY, 7, &ctx, &rng);
    }
    int32_t parryXp71 = dm1_skill_get_experience(&state, DM1_SKILL_IDX_PARRY, 0);
    int32_t fighterXp71 = dm1_skill_get_experience(&state, DM1_SKILL_IDX_FIGHTER, 0);
    ASSERT_EQ(parryXp71, 497);
    ASSERT_EQ(fighterXp71, 497);
    /* PARRY level uses (497 + 497) / 2 = 497, F0303 loop: 497 < 500 → level 1.
     * Level check uses IGNORE_TEMP to mirror F0303 line 882 ("get level before
     * adding XP") which passes MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE; temp XP
     * (which has accumulated 71) would otherwise push the level across. */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_PARRY,
                                  DM1_SKILL_FLAG_IGNORE_TEMP), 1);
    /* FIGHTER level uses raw 497, F0303 loop: 497 < 500 → level 1. */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER,
                                  DM1_SKILL_FLAG_IGNORE_TEMP), 1);

    /* 72nd War Cry: 72 * 7 = 504 XP → level 1 → 2. */
    DM1_LevelUpBonuses b = dm1_skill_add_experience(
        &state, DM1_SKILL_IDX_PARRY, 7, &ctx, &rng);
    int32_t parryXp72 = dm1_skill_get_experience(&state, DM1_SKILL_IDX_PARRY, 0);
    int32_t fighterXp72 = dm1_skill_get_experience(&state, DM1_SKILL_IDX_FIGHTER, 0);
    ASSERT_EQ(parryXp72, 504);
    ASSERT_EQ(fighterXp72, 504);

    /* PARRY level = (504 + 504) / 2 = 504. F0303 loop: 504 >= 500 → 252, level 2.
     * 252 < 500 → stop. Level 2. */
    int parryLevel = dm1_skill_get_level(&state, DM1_SKILL_IDX_PARRY,
                                         DM1_SKILL_FLAG_IGNORE_TEMP);
    ASSERT_EQ(parryLevel, 2);
    /* FIGHTER level = raw 504. Same threshold crossing. Level 2. */
    int fighterLevel = dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER,
                                           DM1_SKILL_FLAG_IGNORE_TEMP);
    ASSERT_EQ(fighterLevel, 2);

    /* Level-up name lookup (PANEL.C G0428). */
    ASSERT_STREQ(dm1_skill_level_name(parryLevel), "NOVICE");
    ASSERT_STREQ(dm1_skill_level_name(fighterLevel), "NOVICE");

    /* F0304 stat bonuses for Fighter (base 0): strength=major (1..2),
     * dexterity=minor (0..1), maxHealth = level*3 + random(level*1.5 + 1).
     * We only check the bonus was actually applied (non-zero). */
    ASSERT_GE(b.strengthDelta, 1);
    ASSERT_GE(b.maxHealthDelta, 1);

    PASS();
    return 0;
}

/* ── Test 5: Out-of-range action index returns invalid route ───────── */
static int test_route_bounds_check(void) {
    TEST(route_bounds_check);

    DM1_ActionXpRoute route;
    /* Below range */
    int ok = dm1_v1_action_xp_route(-1, &route);
    ASSERT_EQ(ok, 0);
    ASSERT_EQ(route.valid, 0);
    /* Above range */
    ok = dm1_v1_action_xp_route(DM1_GRAPHIC560_ACTION_COUNT, &route);
    ASSERT_EQ(ok, 0);
    ASSERT_EQ(route.valid, 0);
    /* NULL out pointer is tolerated as a no-op. */
    ok = dm1_v1_action_xp_route(DM1_ACTION_WAR_CRY, NULL);
    ASSERT_EQ(ok, 0);
    /* First and last entries round-trip. */
    ok = dm1_v1_action_xp_route(0, &route);
    ASSERT_EQ(ok, 1);
    ASSERT_EQ(route.skillIndex, 0);
    ASSERT_EQ(route.experienceGain, 0);
    ok = dm1_v1_action_xp_route(DM1_GRAPHIC560_ACTION_COUNT - 1, &route);
    ASSERT_EQ(ok, 1);
    ASSERT_EQ(route.skillIndex, 3);  /* FUSE → INVOKE/WIZARD */
    ASSERT_EQ(route.experienceGain, 1);

    PASS();
    return 0;
}

/* ── Test 6: Sanity-check a few adjacent action routes for stability ─ */
static int test_action_route_neighbors(void) {
    TEST(action_route_neighbors);

    /* Lock three more deterministic entries to catch silent table drift:
     *   - PARRY (idx 17)    → 17 XP, sub 7 → base 0 FIGHTER
     *   - SWING (idx 13)    → 6 XP,  sub 4 → base 0 FIGHTER
     *   - HEAL  (idx 36)    → 0 XP (PC 3.4 inherits "v1.2 and above" 0)
     */
    DM1_ActionXpRoute r;

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_PARRY, &r), 1);
    ASSERT_EQ(r.skillIndex, DM1_SKILL_IDX_PARRY);
    ASSERT_EQ(r.baseSkillIndex, DM1_SKILL_IDX_FIGHTER);
    ASSERT_EQ(r.experienceGain, 17);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_SWING, &r), 1);
    ASSERT_EQ(r.skillIndex, DM1_SKILL_IDX_SWING);
    ASSERT_EQ(r.baseSkillIndex, DM1_SKILL_IDX_FIGHTER);
    ASSERT_EQ(r.experienceGain, 6);

    ASSERT_EQ(dm1_v1_action_xp_route(DM1_ACTION_HEAL, &r), 1);
    ASSERT_EQ(r.skillIndex, DM1_SKILL_IDX_HEAL);
    ASSERT_EQ(r.baseSkillIndex, DM1_SKILL_IDX_PRIEST);
    ASSERT_EQ(r.experienceGain, 0);

    PASS();
    return 0;
}

/* ── Main ───────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== DM1 V1 Practice-Based Level-Up Formula Gate (PC 3.4) ===\n");
    printf("Fixture: War Cry (action 8) -> PARRY (skill 7) + INFLUENCE (skill 14)\n");
    printf("Sources: ReDMCSB CHAMPION.C F0303/F0304, MENU.C G0496/G0497 (PC 3.4 = Atari ST 1.2+)\n\n");

    int rc = 0;
    rc |= test_war_cry_route_from_graphic560();
    rc |= test_war_cry_secondary_influence_grant();
    rc |= test_war_cry_combined_practice_routing();
    rc |= test_practice_gate_crosses_levelup_at_500();
    rc |= test_route_bounds_check();
    rc |= test_action_route_neighbors();

    printf("\n%d/%d tests passed\n", g_tests_passed, g_tests_run);

    if (rc == 0 && g_tests_passed == g_tests_run) {
        printf("ALL TESTS PASSED ✓\n");
        return 0;
    } else {
        printf("SOME TESTS FAILED ✗\n");
        return 1;
    }
}
