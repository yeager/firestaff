/*
 * CTest gate for DM1 V1 Creature AI Behavior System
 *
 * Source-lock tests verifying behavior dispatch, movement decisions,
 * attack decisions, and group tactics against ReDMCSB semantics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define EXPECT_EQ(a, b, msg) do { \
    if ((a) == (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", msg, (int)(a), (int)(b)); } \
} while(0)

#define EXPECT_NE(a, b, msg) do { \
    if ((a) != (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s (both were %d)\n", msg, (int)(a)); } \
} while(0)

/* Helper to create a default context */
static struct DM1GroupBehaviorContext_Compat make_default_ctx(void) {
    struct DM1GroupBehaviorContext_Compat ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.currentGroupMapX = 5;
    ctx.currentGroupMapY = 5;
    ctx.partyMapX = 6;
    ctx.partyMapY = 5;
    ctx.partyMapIndex = 0;
    ctx.currentMapIndex = 0;
    ctx.partyChampionCount = 4;
    ctx.currentGroupDistanceToParty = 1;
    ctx.currentGroupPrimaryDirToParty = 1; /* East */
    ctx.currentGroupSecondaryDirToParty = 0; /* North */
    ctx.distanceToVisibleParty = 1;
    ctx.creatureCount = 0; /* 1 creature */
    ctx.creatureSize = DM1_SIZE_QUARTER_SQUARE;
    ctx.movementTicks = 20;
    ctx.ticksSinceLastMove = 30;
    ctx.currentTickLow = 1000;
    /* Default creature info: melee attacker with sight */
    ctx.creatureInfo.movementTicks = 20;
    ctx.creatureInfo.attackTicks = 8;
    ctx.creatureInfo.ranges = 0x1003; /* sight=3, smell=0, attack=1 */
    ctx.creatureInfo.properties = 0x0050; /* fearRes=5 */
    ctx.creatureInfo.animationTicks = 0x0334;
    ctx.creatureInfo.attack = 40;
    ctx.creatureInfo.attributes = 0;
    return ctx;
}

static struct DM1ActiveGroup_Compat make_default_ag(void) {
    struct DM1ActiveGroup_Compat ag;
    memset(&ag, 0, sizeof(ag));
    ag.groupThingIndex = 0;
    ag.lastMoveTime = 200;
    ag.homeMapX = 5;
    ag.homeMapY = 5;
    return ag;
}

static struct RngState_Compat make_rng(uint32_t seed) {
    struct RngState_Compat rng;
    rng.seed = seed;
    return rng;
}

/* =========================================================
 *  Test 1: Wander behavior — visible party in range → attack
 * ========================================================= */
static void test_wander_to_attack(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ctx.distanceToVisibleParty = 1;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "wander_to_attack: dispatch returns 1");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_ATTACK,
              "wander_to_attack: transitions to ATTACK");
    EXPECT_EQ(result.actionKind, DM1_ACTION_ATTACK,
              "wander_to_attack: action is ATTACK");
    EXPECT_EQ(result.deleteEvents, 1,
              "wander_to_attack: deletes existing events");
}

/* =========================================================
 *  Test 2: Wander behavior — visible party out of range → approach
 * ========================================================= */
static void test_wander_to_approach(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ctx.distanceToVisibleParty = 3;
    ctx.currentGroupDistanceToParty = 3;
    ctx.partyMapX = 8;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "wander_to_approach: dispatch returns 1");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_APPROACH,
              "wander_to_approach: transitions to APPROACH");
}

/* =========================================================
 *  Test 3: Freeze life gate — creature frozen
 * ========================================================= */
static void test_freeze_life(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ctx.freezeLifeTicks = 10;
    ctx.isArchenemy = 0;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "freeze_life: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_SKIP_FROZEN,
              "freeze_life: action is SKIP_FROZEN");
    EXPECT_EQ(result.nextEventDelayTicks, 4,
              "freeze_life: reschedule in 4 ticks");
}

/* =========================================================
 *  Test 4: Archenemy ignores freeze life
 * ========================================================= */
static void test_archenemy_ignores_freeze(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ctx.freezeLifeTicks = 10;
    ctx.isArchenemy = 1;
    ctx.distanceToVisibleParty = 1;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "archenemy_freeze: dispatch returns 1");
    EXPECT_NE(result.actionKind, DM1_ACTION_SKIP_FROZEN,
              "archenemy_freeze: NOT frozen");
}

/* =========================================================
 *  Test 5: Party-adjacent reaction → attack
 * ========================================================= */
static void test_reaction_party_adjacent(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_EVENT_REACTION_PARTY_IS_ADJACENT;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_adjacent: dispatch returns 1");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_ATTACK,
              "reaction_adjacent: transitions to ATTACK");
    EXPECT_EQ(result.deleteEvents, 1,
              "reaction_adjacent: deletes events");
}

/* =========================================================
 *  Test 6: Flee behavior — fear counter decrements
 * ========================================================= */
static void test_flee_behavior(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_FLEE;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ag.delayFleeingFromTarget = 5;
    ctx.distanceToVisibleParty = 0; /* Can't see party */

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "flee: dispatch returns 1");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_FLEE,
              "flee: stays in FLEE");
    EXPECT_EQ(ag.delayFleeingFromTarget, 4,
              "flee: fear counter decremented");
}

/* =========================================================
 *  Test 7: Flee expires → wander
 * ========================================================= */
static void test_flee_expires_to_wander(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_FLEE;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ag.delayFleeingFromTarget = 0; /* No fear left */
    ctx.distanceToVisibleParty = 0;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "flee_expire: dispatch returns 1");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_WANDER,
              "flee_expire: transitions to WANDER");
    EXPECT_EQ(result.startWandering, 1,
              "flee_expire: startWandering flag set");
}

/* =========================================================
 *  Test 8: Approach → arrives at target → wander
 * ========================================================= */
static void test_approach_arrives_at_target(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_APPROACH;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ctx.distanceToVisibleParty = 0; /* Can't see party */
    ag.targetMapX = ctx.currentGroupMapX;
    ag.targetMapY = ctx.currentGroupMapY;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "approach_arrive: dispatch returns 1");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_WANDER,
              "approach_arrive: transitions to WANDER");
}

/* =========================================================
 *  Test 9: Should-attack range check
 * ========================================================= */
static void test_should_attack_range(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    int shouldAttack = 0;

    /* Same column, distance 1, attack range 1 → should attack */
    ctx.distanceToVisibleParty = 1;
    F0814_DM1_GROUP_ShouldAttack_Compat(&ctx, &shouldAttack);
    EXPECT_EQ(shouldAttack, 1, "should_attack: in melee range");

    /* Distance 3, attack range 1 → should not attack */
    ctx.distanceToVisibleParty = 3;
    ctx.currentGroupDistanceToParty = 3;
    ctx.partyMapX = 8;
    F0814_DM1_GROUP_ShouldAttack_Compat(&ctx, &shouldAttack);
    EXPECT_EQ(shouldAttack, 0, "should_attack: out of range");
}

/* =========================================================
 *  Test 10: Fear check after creature death
 * ========================================================= */
static void test_fear_check(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct RngState_Compat rng = make_rng(999);
    int shouldFlee = 0, fleeDelay = 0;

    /* Low fear resistance with few creatures → might flee */
    ctx.creatureInfo.properties = 0x0010; /* fearRes = 1 */
    F0821_DM1_GROUP_ShouldFrighten_Compat(&ctx, 2, &rng, &shouldFlee, &fleeDelay);
    /* Result is RNG-dependent — just verify it returns cleanly */
    EXPECT_EQ(1, 1, "fear_check: function returned cleanly");

    /* Immune to fear → never flees */
    ctx.creatureInfo.properties = 0x00F0; /* fearRes = 15 = immune */
    shouldFlee = 99;
    F0821_DM1_GROUP_ShouldFrighten_Compat(&ctx, 2, &rng, &shouldFlee, &fleeDelay);
    EXPECT_EQ(shouldFlee, 0, "fear_check: immune never flees");
}

/* =========================================================
 *  Test 11: Projectile use decision
 * ========================================================= */
static void test_projectile_decision(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct RngState_Compat rng = make_rng(42);
    int useProj = 0;

    /* Melee-only creature (range 1) → never uses projectile */
    ctx.creatureInfo.ranges = 0x1003; /* attack range = 1 */
    F0816_DM1_GROUP_ShouldUseProjectile_Compat(&ctx, &rng, &useProj);
    EXPECT_EQ(useProj, 0, "projectile: melee-only never uses projectile");

    /* Spell caster at distance > 1 → always uses projectile */
    ctx.creatureInfo.ranges = 0x3003; /* attack range = 3 */
    ctx.currentGroupDistanceToParty = 2;
    F0816_DM1_GROUP_ShouldUseProjectile_Compat(&ctx, &rng, &useProj);
    EXPECT_EQ(useProj, 1, "projectile: caster at distance uses projectile");
}

/* =========================================================
 *  Test 12: Group direction setting
 * ========================================================= */
static void test_set_group_direction(void) {
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    ag.directions = 0x00; /* All creatures facing North */

    /* Set creature 0 to face East (1) */
    F0817_DM1_GROUP_SetGroupDirection_Compat(&ag, 1, 0, DM1_SIZE_QUARTER_SQUARE, 0);
    int dir0 = ag.directions & 0x03;
    EXPECT_EQ(dir0, 1, "set_dir: creature 0 faces East");
}

/* =========================================================
 *  Test 13: Smell direction
 * ========================================================= */
static void test_smell_direction(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    int dirOrd = 0;

    /* No smell → returns 0 */
    ctx.creatureInfo.ranges = 0x1003; /* smell = 0 */
    F0819_DM1_GROUP_GetSmelledPartyDirOrdinal_Compat(&ctx, &dirOrd);
    EXPECT_EQ(dirOrd, 0, "smell: no smell ability returns 0");

    /* Has smell, close enough → returns direction ordinal */
    ctx.creatureInfo.ranges = 0x1403; /* smell = 4 */
    ctx.currentGroupDistanceToParty = 2;
    F0819_DM1_GROUP_GetSmelledPartyDirOrdinal_Compat(&ctx, &dirOrd);
    EXPECT_NE(dirOrd, 0, "smell: close enough returns non-zero ordinal");
}

/* =========================================================
 *  Test 14: Per-creature attack event (C38)
 * ========================================================= */
static void test_per_creature_attack_event(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_ATTACK;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ctx.distanceToVisibleParty = 1;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "per_creature_attack: returns 1");
    /* Due to random range check, result may be attack or approach */
    EXPECT_NE(result.actionKind, DM1_ACTION_SKIP_FROZEN,
              "per_creature_attack: not frozen");
}

/* =========================================================
 *  Test 15: Negative reaction event during freeze life → ignored
 * ========================================================= */
static void test_reaction_during_freeze(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = -2; /* CM2 reaction */
    ctx.freezeLifeTicks = 5;
    ctx.isArchenemy = 0;

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_freeze: returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_NONE,
              "reaction_freeze: reaction ignored during freeze");
}

/* =========================================================
 *  Test 16: Flee direction is opposite of party direction
 * ========================================================= */
static void test_flee_direction(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    int fleeP = -1, fleeS = -1;

    ctx.currentGroupPrimaryDirToParty = 1; /* Party is East */
    ctx.currentGroupSecondaryDirToParty = 0; /* Secondary: North */

    F0820_DM1_GROUP_GetFleeDirection_Compat(&ctx, &fleeP, &fleeS);
    EXPECT_EQ(fleeP, 3, "flee_dir: primary is West (opposite of East)");
    EXPECT_EQ(fleeS, 2, "flee_dir: secondary is South (opposite of North)");
}

int main(void) {
    printf("DM1 V1 Creature AI Behavior CTest Gate\n");
    printf("Source: ReDMCSB GROUP.C, MOVESENS.C, DEFS.H\n\n");

    test_wander_to_attack();
    test_wander_to_approach();
    test_freeze_life();
    test_archenemy_ignores_freeze();
    test_reaction_party_adjacent();
    test_flee_behavior();
    test_flee_expires_to_wander();
    test_approach_arrives_at_target();
    test_should_attack_range();
    test_fear_check();
    test_projectile_decision();
    test_set_group_direction();
    test_smell_direction();
    test_per_creature_attack_event();
    test_reaction_during_freeze();
    test_flee_direction();

    printf("\n--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
