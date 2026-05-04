/**
 * test_dm1_v1_endgame_system_pc34_compat.c — CTest gate for DM1 V1 Endgame System
 *
 * Validates source-locked endgame constants, Firestaff assembly,
 * Fuse action evaluation, Fuse sequence state machine, and ending params
 * against ReDMCSB reference values.
 */
#ifndef COMPILE_H
#define COMPILE_H
#define STATICFUNCTION static
#define SEPARATOR ,
#define FINAL_SEPARATOR )
#define HUGE
#define huge
#endif

#include "dm1_v1_endgame_system_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail = 0;

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL: %s: expected %d, got %d\n", (msg), (int)(b), (int)(a)); \
        g_fail = 1; \
    } \
} while(0)

#define ASSERT_NEQ(a, b, msg) do { \
    if ((a) == (b)) { \
        fprintf(stderr, "FAIL: %s: expected != %d\n", (msg), (int)(b)); \
        g_fail = 1; \
    } \
} while(0)

#define ASSERT_NOT_NULL(p, msg) do { \
    if (!(p)) { \
        fprintf(stderr, "FAIL: %s: expected non-NULL\n", (msg)); \
        g_fail = 1; \
    } \
} while(0)

/* ── Test: Creature constants match DEFS.H ──────────────────────── */
static void test_creature_constants(void)
{
    printf("  creature constants...\n");
    ASSERT_EQ(DM1_CREATURE_LORD_CHAOS_ID, 23, "C23_CREATURE_LORD_CHAOS");
    ASSERT_EQ(DM1_CREATURE_RED_DRAGON_ID, 24, "C24_CREATURE_RED_DRAGON");
    ASSERT_EQ(DM1_CREATURE_LORD_ORDER_ID, 25, "C25_CREATURE_LORD_ORDER");
    ASSERT_EQ(DM1_CREATURE_GREY_LORD_ID,  26, "C26_CREATURE_GREY_LORD");
    ASSERT_EQ(DM1_SINGLE_CENTERED_CREATURE, 0xFF, "C0xFF_SINGLE_CENTERED_CREATURE");
}

/* ── Test: Action constants match DEFS.H ────────────────────────── */
static void test_action_constants(void)
{
    printf("  action constants...\n");
    ASSERT_EQ(DM1_ACTION_FLUXCAGE, 35, "C035_ACTION_FLUXCAGE");
    ASSERT_EQ(DM1_ACTION_FUSE,     43, "C043_ACTION_FUSE");
}

/* ── Test: Item icon constants match DEFS.H ─────────────────────── */
static void test_item_constants(void)
{
    printf("  item icon constants...\n");
    ASSERT_EQ(DM1_ICON_WEAPON_THE_FIRESTAFF,          27, "C027_ICON_WEAPON_THE_FIRESTAFF");
    ASSERT_EQ(DM1_ICON_WEAPON_THE_FIRESTAFF_COMPLETE,  28, "C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE");
    ASSERT_EQ(DM1_ICON_JUNK_GEM_OF_AGES,             120, "C120_ICON_JUNK_GEM_OF_AGES");
}

/* ── Test: Explosion constants match DEFS.H ─────────────────────── */
static void test_explosion_constants(void)
{
    printf("  explosion constants...\n");
    ASSERT_EQ(DM1_EXPLOSION_FLUXCAGE_TYPE,     50,     "C050_EXPLOSION_FLUXCAGE");
    ASSERT_EQ(DM1_THING_EXPLOSION_FIREBALL,    0xFF80, "C0xFF80_THING_EXPLOSION_FIREBALL");
    ASSERT_EQ(DM1_THING_EXPLOSION_HARM_NONMAT, 0xFF83, "C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL");
}

/* ── Test: Firestaff assembly logic ─────────────────────────────── */
static void test_firestaff_assembly(void)
{
    printf("  Firestaff assembly...\n");

    /* Base Firestaff + Gem of Ages = can assemble */
    ASSERT_EQ(DM1_Endgame_CanAssembleFirestaff(27, 120), 1,
              "base Firestaff + Gem of Ages can assemble");

    /* Reverse order also valid */
    ASSERT_EQ(DM1_Endgame_CanAssembleFirestaff(120, 27), 1,
              "Gem of Ages + base Firestaff can assemble (reverse)");

    /* Already complete — cannot assemble */
    ASSERT_EQ(DM1_Endgame_CanAssembleFirestaff(28, 120), 0,
              "complete Firestaff + Gem cannot re-assemble");

    /* Random items — cannot assemble */
    ASSERT_EQ(DM1_Endgame_CanAssembleFirestaff(10, 15), 0,
              "random items cannot assemble");

    /* Assembly result */
    ASSERT_EQ(DM1_Endgame_GetAssembledFirestaffIcon(27, 120), 28,
              "assembled icon is C028");
    ASSERT_EQ(DM1_Endgame_GetAssembledFirestaffIcon(10, 15), 10,
              "non-assembling returns original icon");
}

/* ── Test: Firestaff skill bonus (CHAMPION.C:771-774) ───────────── */
static void test_firestaff_skill_bonus(void)
{
    printf("  Firestaff skill bonus...\n");
    ASSERT_EQ(DM1_Endgame_GetFirestaffSkillBonus(27), 1, "base Firestaff +1");
    ASSERT_EQ(DM1_Endgame_GetFirestaffSkillBonus(28), 2, "complete Firestaff +2");
    ASSERT_EQ(DM1_Endgame_GetFirestaffSkillBonus(10), 0, "other item +0");
}

/* ── Test: Fluxcage counting ────────────────────────────────────── */
static void test_fluxcage_count(void)
{
    printf("  Fluxcage counting...\n");
    {
        int32_t all4[4] = {1, 1, 1, 1};
        ASSERT_EQ(DM1_Endgame_CountFluxcagesAroundSquare(all4), 4, "all 4 fluxcages");
    }
    {
        int32_t three[4] = {1, 0, 1, 1};
        ASSERT_EQ(DM1_Endgame_CountFluxcagesAroundSquare(three), 3, "3 fluxcages");
    }
    {
        int32_t none[4] = {0, 0, 0, 0};
        ASSERT_EQ(DM1_Endgame_CountFluxcagesAroundSquare(none), 0, "0 fluxcages");
    }
    ASSERT_EQ(DM1_Endgame_CountFluxcagesAroundSquare(NULL), 0, "NULL fluxcages");
}

/* ── Test: Lord Chaos identification ────────────────────────────── */
static void test_lord_chaos_identification(void)
{
    printf("  Lord Chaos identification...\n");
    ASSERT_EQ(DM1_Endgame_IsLordChaosOnSquare(23), 1, "creature 23 = Lord Chaos");
    ASSERT_EQ(DM1_Endgame_IsLordChaosOnSquare(25), 0, "creature 25 != Lord Chaos");
    ASSERT_EQ(DM1_Endgame_IsLordChaosOnSquare(-1), 0, "no creature != Lord Chaos");
    ASSERT_EQ(DM1_Endgame_IsLordChaosOnSquare(0),  0, "creature 0 != Lord Chaos");
}

/* ── Test: Fuse action — Lord Chaos not present ──────────────────── */
static void test_fuse_action_no_lord_chaos(void)
{
    printf("  Fuse action — no Lord Chaos...\n");
    DM1EndgameFuseActionResult result;
    int32_t fluxcages[4] = {0, 0, 0, 0};
    DM1_Endgame_EvaluateFuseAction(5, 5, 32, 32, 10, fluxcages, 1, &result);
    ASSERT_EQ(result.lordChaosPresent, 0, "Lord Chaos not present");
    ASSERT_EQ(result.fuseSequenceTriggered, 0, "fuse sequence not triggered");
}

/* ── Test: Fuse action — Lord Chaos escapes ─────────────────────── */
static void test_fuse_action_chaos_escapes(void)
{
    printf("  Fuse action — Lord Chaos escapes...\n");
    DM1EndgameFuseActionResult result;
    int32_t fluxcages[4] = {1, 1, 1, 0}; /* 3 fluxcages, one gap */
    DM1_Endgame_EvaluateFuseAction(5, 5, 32, 32, 23, fluxcages, 1, &result);
    ASSERT_EQ(result.lordChaosPresent, 1, "Lord Chaos present");
    ASSERT_EQ(result.fluxcageCount, 3, "3 fluxcages");
    ASSERT_EQ(result.lordChaosEscaped, 1, "Lord Chaos escaped");
    ASSERT_EQ(result.fuseSequenceTriggered, 0, "fuse not triggered (escaped)");
}

/* ── Test: Fuse action — Lord Chaos trapped (4 fluxcages) ───────── */
static void test_fuse_action_chaos_trapped(void)
{
    printf("  Fuse action — Lord Chaos trapped...\n");
    DM1EndgameFuseActionResult result;
    int32_t fluxcages[4] = {1, 1, 1, 1}; /* fully surrounded */
    DM1_Endgame_EvaluateFuseAction(5, 5, 32, 32, 23, fluxcages, 0, &result);
    ASSERT_EQ(result.lordChaosPresent, 1, "Lord Chaos present");
    ASSERT_EQ(result.fluxcageCount, 4, "4 fluxcages");
    ASSERT_EQ(result.lordChaosEscaped, 0, "Lord Chaos cannot escape");
    ASSERT_EQ(result.fuseSequenceTriggered, 1, "fuse sequence triggered");
}

/* ── Test: Fuse action — out of bounds ──────────────────────────── */
static void test_fuse_action_out_of_bounds(void)
{
    printf("  Fuse action — out of bounds...\n");
    DM1EndgameFuseActionResult result;
    int32_t fluxcages[4] = {0, 0, 0, 0};
    DM1_Endgame_EvaluateFuseAction(-1, 5, 32, 32, 23, fluxcages, 0, &result);
    ASSERT_EQ(result.fuseSequenceTriggered, 0, "out of bounds — no trigger");
    ASSERT_EQ(result.lordChaosPresent, 0, "out of bounds — not evaluated");
}

/* ── Test: Chaos/Order cycling creature type ─────────────────────── */
static void test_cycle_creature_type(void)
{
    printf("  Chaos/Order cycling...\n");
    /* Source: (switchCount & 1) ? LORD_ORDER : LORD_CHAOS */
    ASSERT_EQ(DM1_Endgame_GetCycleCreatureType(1), 25, "switch 1 -> Lord Order");
    ASSERT_EQ(DM1_Endgame_GetCycleCreatureType(2), 23, "switch 2 -> Lord Chaos");
    ASSERT_EQ(DM1_Endgame_GetCycleCreatureType(3), 25, "switch 3 -> Lord Order");
    ASSERT_EQ(DM1_Endgame_GetCycleCreatureType(4), 23, "switch 4 -> Lord Chaos");
    ASSERT_EQ(DM1_Endgame_GetCycleCreatureType(5), 25, "switch 5 -> Lord Order");
}

/* ── Test: Fuse sequence state machine ──────────────────────────── */
static void test_fuse_sequence_full(void)
{
    printf("  Fuse sequence full run...\n");
    DM1EndgameFuseState state;
    int steps = 0;
    int maxSteps = 500; /* safety limit */

    DM1_Endgame_FuseSequence_Init(&state, 10, 10);
    ASSERT_EQ(state.phase, DM1_FUSE_PHASE_INIT, "initial phase is INIT");
    ASSERT_EQ(state.lordChaosHealth, 10000, "initial health 10000");
    ASSERT_EQ(state.lordChaosMapX, 10, "Lord Chaos X = 10");
    ASSERT_EQ(state.lordChaosMapY, 10, "Lord Chaos Y = 10");

    while (DM1_Endgame_FuseSequence_Step(&state) && steps < maxSteps) {
        steps++;
        ASSERT_NOT_NULL(state.lastSourceEvidence, "evidence set each step");
    }

    ASSERT_EQ(state.phase, DM1_FUSE_PHASE_COMPLETE, "final phase is COMPLETE");
    ASSERT_EQ(state.gameWon, 1, "game_won is set");
    ASSERT_EQ(state.currentCreatureType, DM1_CREATURE_GREY_LORD_ID,
              "final creature type is Grey Lord (26)");
    ASSERT_EQ(state.doNotDrawFluxcages, 1, "doNotDrawFluxcages set");
    ASSERT_EQ(state.restartAllowed, 0, "restart not allowed after win");
    printf("    completed in %d steps\n", steps);

    /* Verify it passed through key phases by checking creature type transitions */
    /* The Grey Lord should be the final type */
    ASSERT_EQ(DM1_Endgame_FuseSequence_GetCreatureType(&state),
              DM1_CREATURE_GREY_LORD_ID, "GetCreatureType returns Grey Lord");
}

/* ── Test: Fuse sequence explosion params ────────────────────────── */
static void test_fuse_sequence_explosions(void)
{
    printf("  Fuse sequence explosion params...\n");
    DM1EndgameFuseState state;
    int32_t expType, expAttack;
    int found_fireball = 0;
    int found_harm = 0;
    int maxSteps = 500;
    int steps = 0;

    DM1_Endgame_FuseSequence_Init(&state, 10, 10);
    while (DM1_Endgame_FuseSequence_Step(&state) && steps < maxSteps) {
        steps++;
        if (DM1_Endgame_FuseSequence_GetExplosionParams(&state, &expType, &expAttack)) {
            if (expType == (int32_t)DM1_THING_EXPLOSION_FIREBALL) found_fireball = 1;
            if (expType == (int32_t)DM1_THING_EXPLOSION_HARM_NONMAT) found_harm = 1;
        }
    }

    ASSERT_EQ(found_fireball, 1, "fireball explosions occurred during sequence");
    ASSERT_EQ(found_harm, 1, "harm-non-material explosions occurred during sequence");
}

/* ── Test: Ending parameters ─────────────────────────────────────── */
static void test_ending_params(void)
{
    printf("  ending parameters...\n");
    const DM1EndgameEndingParams* params = DM1_Endgame_GetEndingParams();
    ASSERT_NOT_NULL(params, "ending params not NULL");
    ASSERT_EQ(params->finalDelayTicks, 600, "final delay 600 ticks");
    ASSERT_EQ(params->restartAllowedAfterWin, 0, "restart not allowed");
    ASSERT_EQ(params->endgameCalledWithTrue, 1, "endgame called with TRUE");
    ASSERT_EQ(params->victoryMusicId, 2, "victory music ID 2");
    ASSERT_NOT_NULL(params->sourceEvidence, "evidence not NULL");
}

/* ── Test: Phase enum count ──────────────────────────────────────── */
static void test_phase_enum(void)
{
    printf("  phase enum...\n");
    ASSERT_EQ(DM1_FUSE_PHASE_COUNT, DM1_FUSE_PHASE_COMPLETE,
              "PHASE_COUNT == PHASE_COMPLETE");
    /* Verify phases are sequential starting from 0 */
    ASSERT_EQ(DM1_FUSE_PHASE_INIT, 0, "INIT = 0");
    ASSERT_EQ(DM1_FUSE_PHASE_COMPLETE, 13, "COMPLETE = 13 (14 phases total)");
}

/* ── Test: Source evidence ───────────────────────────────────────── */
static void test_source_evidence(void)
{
    printf("  source evidence...\n");
    const char* ev = DM1_Endgame_System_GetSourceEvidence();
    ASSERT_NOT_NULL(ev, "global source evidence");
    /* Verify it mentions the key source files */
    ASSERT_NOT_NULL(strstr(ev, "ENDGAME.C"), "evidence mentions ENDGAME.C");
    ASSERT_NOT_NULL(strstr(ev, "PROJEXPL.C"), "evidence mentions PROJEXPL.C");
    ASSERT_NOT_NULL(strstr(ev, "CHAMPION.C"), "evidence mentions CHAMPION.C");
    ASSERT_NOT_NULL(strstr(ev, "DEFS.H"), "evidence mentions DEFS.H");
    ASSERT_NOT_NULL(strstr(ev, "CSBWin"), "evidence mentions CSBWin");
}

int main(void)
{
    printf("=== DM1 V1 Endgame System CTest Gate ===\n");

    test_creature_constants();
    test_action_constants();
    test_item_constants();
    test_explosion_constants();
    test_firestaff_assembly();
    test_firestaff_skill_bonus();
    test_fluxcage_count();
    test_lord_chaos_identification();
    test_fuse_action_no_lord_chaos();
    test_fuse_action_chaos_escapes();
    test_fuse_action_chaos_trapped();
    test_fuse_action_out_of_bounds();
    test_cycle_creature_type();
    test_fuse_sequence_full();
    test_fuse_sequence_explosions();
    test_ending_params();
    test_phase_enum();
    test_source_evidence();

    if (g_fail) {
        fprintf(stderr, "\n*** FAILURES DETECTED ***\n");
        return 1;
    }
    printf("\nAll endgame system tests PASSED.\n");
    return 0;
}
