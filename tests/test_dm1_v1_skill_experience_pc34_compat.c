/*
 * test_dm1_v1_skill_experience_pc34_compat.c — CTest gate for skill/experience system.
 *
 * Validates source-lock against ReDMCSB: CHAMPION.C (F0303, F0304),
 * DEFS.H (skill indices), PANEL.C (skill level names).
 */
#include "dm1_v1_skill_experience_pc34_compat.h"
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

#define ASSERT_GT(a, b) do { \
    if (!((a) > (b))) { \
        char buf[256]; \
        snprintf(buf, sizeof(buf), "%s > %s (got %d, threshold %d)", \
                 #a, #b, (int)(a), (int)(b)); \
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

/* ── Test: Skill index definitions ──────────────────────────────────── */
static int test_skill_indices(void) {
    TEST(skill_indices);

    /* ReDMCSB DEFS.H lines 757-776: verify all 20 skill indices */
    ASSERT_EQ(DM1_SKILL_IDX_FIGHTER,   0);
    ASSERT_EQ(DM1_SKILL_IDX_NINJA,     1);
    ASSERT_EQ(DM1_SKILL_IDX_PRIEST,    2);
    ASSERT_EQ(DM1_SKILL_IDX_WIZARD,    3);
    ASSERT_EQ(DM1_SKILL_IDX_SWING,     4);
    ASSERT_EQ(DM1_SKILL_IDX_THRUST,    5);
    ASSERT_EQ(DM1_SKILL_IDX_CLUB,      6);
    ASSERT_EQ(DM1_SKILL_IDX_PARRY,     7);
    ASSERT_EQ(DM1_SKILL_IDX_STEAL,     8);
    ASSERT_EQ(DM1_SKILL_IDX_FIGHT,     9);
    ASSERT_EQ(DM1_SKILL_IDX_THROW,    10);
    ASSERT_EQ(DM1_SKILL_IDX_SHOOT,    11);
    ASSERT_EQ(DM1_SKILL_IDX_IDENTIFY, 12);
    ASSERT_EQ(DM1_SKILL_IDX_HEAL,     13);
    ASSERT_EQ(DM1_SKILL_IDX_INFLUENCE,14);
    ASSERT_EQ(DM1_SKILL_IDX_DEFEND,   15);
    ASSERT_EQ(DM1_SKILL_IDX_FIRE,     16);
    ASSERT_EQ(DM1_SKILL_IDX_AIR,      17);
    ASSERT_EQ(DM1_SKILL_IDX_EARTH,    18);
    ASSERT_EQ(DM1_SKILL_IDX_WATER,    19);
    ASSERT_EQ(DM1_TOTAL_SKILL_COUNT,  20);
    ASSERT_EQ(DM1_BASE_SKILL_COUNT,    4);

    PASS();
    return 0;
}

/* ── Test: Base skill index mapping ─────────────────────────────────── */
static int test_base_skill_mapping(void) {
    TEST(base_skill_mapping);

    /* Base skills map to themselves */
    ASSERT_EQ(dm1_skill_get_base_index(0), 0);  /* Fighter → Fighter */
    ASSERT_EQ(dm1_skill_get_base_index(1), 1);  /* Ninja → Ninja */
    ASSERT_EQ(dm1_skill_get_base_index(2), 2);  /* Priest → Priest */
    ASSERT_EQ(dm1_skill_get_base_index(3), 3);  /* Wizard → Wizard */

    /* Fighter sub-skills (4-7) → Fighter (0) */
    /* ReDMCSB: (4-4)>>2=0, (5-4)>>2=0, (6-4)>>2=0, (7-4)>>2=0 */
    ASSERT_EQ(dm1_skill_get_base_index(4), 0);  /* Swing → Fighter */
    ASSERT_EQ(dm1_skill_get_base_index(5), 0);  /* Thrust → Fighter */
    ASSERT_EQ(dm1_skill_get_base_index(6), 0);  /* Club → Fighter */
    ASSERT_EQ(dm1_skill_get_base_index(7), 0);  /* Parry → Fighter */

    /* Ninja sub-skills (8-11) → Ninja (1) */
    ASSERT_EQ(dm1_skill_get_base_index(8),  1);  /* Steal → Ninja */
    ASSERT_EQ(dm1_skill_get_base_index(9),  1);  /* Fight → Ninja */
    ASSERT_EQ(dm1_skill_get_base_index(10), 1);  /* Throw → Ninja */
    ASSERT_EQ(dm1_skill_get_base_index(11), 1);  /* Shoot → Ninja */

    /* Priest sub-skills (12-15) → Priest (2) */
    ASSERT_EQ(dm1_skill_get_base_index(12), 2);  /* Identify → Priest */
    ASSERT_EQ(dm1_skill_get_base_index(13), 2);  /* Heal → Priest */
    ASSERT_EQ(dm1_skill_get_base_index(14), 2);  /* Influence → Priest */
    ASSERT_EQ(dm1_skill_get_base_index(15), 2);  /* Defend → Priest */

    /* Wizard sub-skills (16-19) → Wizard (3) */
    ASSERT_EQ(dm1_skill_get_base_index(16), 3);  /* Fire → Wizard */
    ASSERT_EQ(dm1_skill_get_base_index(17), 3);  /* Air → Wizard */
    ASSERT_EQ(dm1_skill_get_base_index(18), 3);  /* Earth → Wizard */
    ASSERT_EQ(dm1_skill_get_base_index(19), 3);  /* Water → Wizard */

    PASS();
    return 0;
}

/* ── Test: Experience → Level (F0303 algorithm) ─────────────────────── */
static int test_experience_to_level(void) {
    TEST(experience_to_level);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);

    /* Level 1: 0 XP */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 1);

    /* Level 1: 499 XP (just below threshold) */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 499;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 1);

    /* Level 2: 500 XP — first threshold.
     * F0303: 500 >= 500 → 500>>1=250, level=2. 250 < 500 → stop. */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 500;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 2);

    /* Level 3: 1000 XP.
     * 1000>=500 → 500, level=2. 500>=500 → 250, level=3. stop. */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 1000;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 3);

    /* Level 4: 2000 XP */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 2000;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 4);

    /* Level 5: 4000 XP */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 4000;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 5);

    /* Level 8: 32000 XP */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 32000;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 8);

    /* Level 15: 4096000 XP */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 4096000;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 15);

    PASS();
    return 0;
}

/* ── Test: Level thresholds match F0303 algorithm ───────────────────── */
static int test_level_thresholds(void) {
    TEST(level_thresholds);

    ASSERT_EQ(dm1_skill_level_threshold(1), 0);
    ASSERT_EQ(dm1_skill_level_threshold(2), 500);
    ASSERT_EQ(dm1_skill_level_threshold(3), 1000);
    ASSERT_EQ(dm1_skill_level_threshold(4), 2000);
    ASSERT_EQ(dm1_skill_level_threshold(5), 4000);
    ASSERT_EQ(dm1_skill_level_threshold(6), 8000);
    ASSERT_EQ(dm1_skill_level_threshold(7), 16000);
    ASSERT_EQ(dm1_skill_level_threshold(8), 32000);
    ASSERT_EQ(dm1_skill_level_threshold(9), 64000);
    ASSERT_EQ(dm1_skill_level_threshold(10), 128000);
    ASSERT_EQ(dm1_skill_level_threshold(15), 4096000);

    /* Verify consistency: threshold at level N gives level N */
    DM1_ChampionSkillState state;
    for (int lvl = 1; lvl <= 15; lvl++) {
        dm1_skill_state_init(&state);
        state.skills[0].experience = dm1_skill_level_threshold(lvl);
        int computed = dm1_skill_get_level(&state, 0, DM1_SKILL_FLAG_IGNORE_TEMP);
        ASSERT_EQ(computed, lvl);
    }

    PASS();
    return 0;
}

/* ── Test: Sub-skill level uses average of base+sub XP (F0303) ─────── */
static int test_subskill_level_averaging(void) {
    TEST(subskill_level_averaging);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);

    /* Set Swing (4) to 1000 XP, Fighter (0) to 0 XP.
     * Average = (1000 + 0) / 2 = 500 → level 2. */
    state.skills[DM1_SKILL_IDX_SWING].experience = 1000;
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 0;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_SWING,
              DM1_SKILL_FLAG_IGNORE_TEMP), 2);

    /* Set both to 1000 XP.
     * Average = (1000 + 1000) / 2 = 1000 → level 3. */
    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 1000;
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_SWING,
              DM1_SKILL_FLAG_IGNORE_TEMP), 3);

    /* Base skill level is NOT affected by sub-skill XP */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER,
              DM1_SKILL_FLAG_IGNORE_TEMP), 3);

    PASS();
    return 0;
}

/* ── Test: Temporary experience contributes to level ────────────────── */
static int test_temporary_experience(void) {
    TEST(temporary_experience);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);

    state.skills[DM1_SKILL_IDX_FIGHTER].experience = 400;
    state.skills[DM1_SKILL_IDX_FIGHTER].temporaryExperience = 100;

    /* With temp: 400 + 100 = 500 → level 2 */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER, 0), 2);

    /* Without temp (IGNORE_TEMP flag): 400 < 500 → level 1 */
    ASSERT_EQ(dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER,
              DM1_SKILL_FLAG_IGNORE_TEMP), 1);

    PASS();
    return 0;
}

/* ── Test: Skill level names (PANEL.C G0428) ────────────────────────── */
static int test_skill_level_names(void) {
    TEST(skill_level_names);

    ASSERT_STREQ(dm1_skill_level_name(0),  "NONE");
    ASSERT_STREQ(dm1_skill_level_name(1),  "NEOPHYTE");
    ASSERT_STREQ(dm1_skill_level_name(2),  "NOVICE");
    ASSERT_STREQ(dm1_skill_level_name(3),  "APPRENTICE");
    ASSERT_STREQ(dm1_skill_level_name(4),  "JOURNEYMAN");
    ASSERT_STREQ(dm1_skill_level_name(5),  "CRAFTSMAN");
    ASSERT_STREQ(dm1_skill_level_name(6),  "ARTISAN");
    ASSERT_STREQ(dm1_skill_level_name(7),  "ADEPT");
    ASSERT_STREQ(dm1_skill_level_name(8),  "EXPERT");
    ASSERT_STREQ(dm1_skill_level_name(9),  "LO MASTER");
    ASSERT_STREQ(dm1_skill_level_name(15), "ARCHMASTER");

    PASS();
    return 0;
}

/* ── Test: Skill index names ────────────────────────────────────────── */
static int test_skill_index_names(void) {
    TEST(skill_index_names);

    ASSERT_STREQ(dm1_skill_index_name(0),  "FIGHTER");
    ASSERT_STREQ(dm1_skill_index_name(4),  "SWING");
    ASSERT_STREQ(dm1_skill_index_name(8),  "STEAL");
    ASSERT_STREQ(dm1_skill_index_name(13), "HEAL");
    ASSERT_STREQ(dm1_skill_index_name(16), "FIRE");
    ASSERT_STREQ(dm1_skill_index_name(19), "WATER");
    ASSERT_STREQ(dm1_skill_index_name(-1), "UNKNOWN");
    ASSERT_STREQ(dm1_skill_index_name(20), "UNKNOWN");

    PASS();
    return 0;
}

/* ── Test: Experience gain with level-up (F0304) ────────────────────── */
static int test_add_experience_levelup(void) {
    TEST(add_experience_levelup);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 12345;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 0;
    ctx.gameTime = 200;  /* > 150 past last attack → combat penalty doesn't apply for base */
    ctx.partyIsResting = 0;

    /* Add 500 XP to Fighter (base skill) — should trigger level 1→2 */
    DM1_LevelUpBonuses b = dm1_skill_add_experience(
        &state, DM1_SKILL_IDX_FIGHTER, 500, &ctx, &rng);

    int level = dm1_skill_get_level(&state, DM1_SKILL_IDX_FIGHTER,
                                    DM1_SKILL_FLAG_IGNORE_TEMP);
    ASSERT_EQ(level, 2);

    /* Fighter level-up should give strength and/or dexterity */
    ASSERT_GE(b.maxHealthDelta, 1);
    ASSERT_GE(b.maxStaminaDelta, 0);

    PASS();
    return 0;
}

/* ── Test: Sub-skill XP propagates to base skill (F0304) ────────────── */
static int test_subskill_xp_propagation(void) {
    TEST(subskill_xp_propagation);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 54321;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 190;  /* Recent attack → within 25 ticks = combat bonus */
    ctx.gameTime = 200;
    ctx.partyIsResting = 0;

    /* Add XP to Swing (Fighter sub-skill) */
    dm1_skill_add_experience(&state, DM1_SKILL_IDX_SWING, 300, &ctx, &rng);

    /* Swing should have XP */
    ASSERT_GT(dm1_skill_get_experience(&state, DM1_SKILL_IDX_SWING, 0), 0);

    /* Fighter base should also have gained XP (F0304 line 892) */
    ASSERT_GT(dm1_skill_get_experience(&state, DM1_SKILL_IDX_FIGHTER, 0), 0);

    PASS();
    return 0;
}

/* ── Test: Combat proximity halves XP for combat sub-skills (F0304) ── */
static int test_combat_proximity_penalty(void) {
    TEST(combat_proximity_penalty);

    DM1_ChampionSkillState state1, state2;
    dm1_skill_state_init(&state1);
    dm1_skill_state_init(&state2);
    uint32_t rng1 = 11111, rng2 = 11111;

    /* Context with recent attack (no penalty) */
    DM1_SkillContext ctxRecent;
    ctxRecent.mapDifficulty = 1;
    ctxRecent.lastCreatureAttackTime = 100;
    ctxRecent.gameTime = 200;
    ctxRecent.partyIsResting = 0;

    /* Context without recent attack (penalty applies) */
    DM1_SkillContext ctxOld;
    ctxOld.mapDifficulty = 1;
    ctxOld.lastCreatureAttackTime = 10;  /* 200 - 10 = 190 > 150 → penalty */
    ctxOld.gameTime = 200;
    ctxOld.partyIsResting = 0;

    /* Add 1000 XP to Swing with recent attack */
    dm1_skill_add_experience(&state1, DM1_SKILL_IDX_SWING, 1000, &ctxRecent, &rng1);
    /* Add 1000 XP to Swing without recent attack (halved) */
    dm1_skill_add_experience(&state2, DM1_SKILL_IDX_SWING, 1000, &ctxOld, &rng2);

    /* State1 should have more XP because no penalty + combat bonus */
    int32_t xp1 = dm1_skill_get_experience(&state1, DM1_SKILL_IDX_SWING, 0);
    int32_t xp2 = dm1_skill_get_experience(&state2, DM1_SKILL_IDX_SWING, 0);
    ASSERT_GT(xp1, xp2);

    PASS();
    return 0;
}

/* ── Test: Map difficulty multiplier (F0304) ────────────────────────── */
static int test_difficulty_multiplier(void) {
    TEST(difficulty_multiplier);

    DM1_ChampionSkillState state1, state2;
    dm1_skill_state_init(&state1);
    dm1_skill_state_init(&state2);
    uint32_t rng1 = 99999, rng2 = 99999;

    DM1_SkillContext ctx1, ctx2;
    ctx1.mapDifficulty = 1;
    ctx1.lastCreatureAttackTime = 0;
    ctx1.gameTime = 200;
    ctx1.partyIsResting = 0;

    ctx2 = ctx1;
    ctx2.mapDifficulty = 3;  /* 3x difficulty */

    dm1_skill_add_experience(&state1, DM1_SKILL_IDX_FIGHTER, 100, &ctx1, &rng1);
    dm1_skill_add_experience(&state2, DM1_SKILL_IDX_FIGHTER, 100, &ctx2, &rng2);

    int32_t xp1 = dm1_skill_get_experience(&state1, DM1_SKILL_IDX_FIGHTER, 0);
    int32_t xp2 = dm1_skill_get_experience(&state2, DM1_SKILL_IDX_FIGHTER, 0);

    /* Difficulty 3 should give 3x XP */
    ASSERT_EQ(xp2, xp1 * 3);

    PASS();
    return 0;
}

/* ── Test: Wizard level-up gives mana (F0304) ───────────────────────── */
static int test_wizard_levelup_mana(void) {
    TEST(wizard_levelup_mana);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 77777;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 0;
    ctx.gameTime = 200;
    ctx.partyIsResting = 0;

    /* Give enough XP to level up Wizard */
    DM1_LevelUpBonuses b = dm1_skill_add_experience(
        &state, DM1_SKILL_IDX_WIZARD, 500, &ctx, &rng);

    /* Wizard level-up should grant mana */
    ASSERT_GT(b.maxManaDelta, 0);

    PASS();
    return 0;
}

/* ── Test: Priest level-up gives mana and wisdom (F0304) ────────────── */
static int test_priest_levelup(void) {
    TEST(priest_levelup);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 33333;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 0;
    ctx.gameTime = 200;
    ctx.partyIsResting = 0;

    DM1_LevelUpBonuses b = dm1_skill_add_experience(
        &state, DM1_SKILL_IDX_PRIEST, 500, &ctx, &rng);

    ASSERT_GT(b.maxManaDelta, 0);
    ASSERT_GE(b.maxHealthDelta, 1);

    PASS();
    return 0;
}

/* ── Test: Null safety ──────────────────────────────────────────────── */
static int test_null_safety(void) {
    TEST(null_safety);

    /* All functions should handle NULL gracefully */
    ASSERT_EQ(dm1_skill_get_level(NULL, 0, 0), 1);
    ASSERT_EQ(dm1_skill_get_experience(NULL, 0, 0), 0);
    ASSERT_EQ(dm1_skill_get_base_index(-1), 0);

    DM1_SkillContext ctx = {0};
    uint32_t rng = 1;
    DM1_LevelUpBonuses b = dm1_skill_add_experience(NULL, 0, 100, &ctx, &rng);
    ASSERT_EQ(b.maxHealthDelta, 0);

    dm1_skill_state_init(NULL);  /* Should not crash */

    PASS();
    return 0;
}

/* ── Test: Temporary XP cap at 32000 (F0304 line 888) ───────────────── */
static int test_temp_xp_cap(void) {
    TEST(temp_xp_cap);

    DM1_ChampionSkillState state;
    dm1_skill_state_init(&state);
    uint32_t rng = 55555;

    DM1_SkillContext ctx;
    ctx.mapDifficulty = 1;
    ctx.lastCreatureAttackTime = 0;
    ctx.gameTime = 200;
    ctx.partyIsResting = 0;

    /* Add massive XP repeatedly */
    for (int i = 0; i < 100; i++) {
        dm1_skill_add_experience(&state, DM1_SKILL_IDX_FIGHTER, 10000, &ctx, &rng);
    }

    /* Temp XP should not exceed 32000 + 100 (one last addition before cap) */
    /* Actually, the cap check is "if (tempXP < 32000)" so once >= 32000, no more adds */
    /* But because we add 100 max per call, worst case is just under 32100 */
    /* In practice it should stay ≤ 32100 */
    ASSERT_GE(32200, (int)state.skills[DM1_SKILL_IDX_FIGHTER].temporaryExperience);

    PASS();
    return 0;
}

/* ── Main ───────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== DM1 V1 Skill/Experience System Source-Lock Tests ===\n");

    int rc = 0;
    rc |= test_skill_indices();
    rc |= test_base_skill_mapping();
    rc |= test_experience_to_level();
    rc |= test_level_thresholds();
    rc |= test_subskill_level_averaging();
    rc |= test_temporary_experience();
    rc |= test_skill_level_names();
    rc |= test_skill_index_names();
    rc |= test_add_experience_levelup();
    rc |= test_subskill_xp_propagation();
    rc |= test_combat_proximity_penalty();
    rc |= test_difficulty_multiplier();
    rc |= test_wizard_levelup_mana();
    rc |= test_priest_levelup();
    rc |= test_null_safety();
    rc |= test_temp_xp_cap();

    printf("\n%d/%d tests passed\n", g_tests_passed, g_tests_run);

    if (g_tests_passed == g_tests_run) {
        printf("ALL TESTS PASSED ✓\n");
        return 0;
    } else {
        printf("SOME TESTS FAILED ✗\n");
        return 1;
    }
}
