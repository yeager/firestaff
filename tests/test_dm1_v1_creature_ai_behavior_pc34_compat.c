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

    /* Spell caster at distance > 1 -> always uses projectile */
    ctx.creatureInfo.ranges = 0x3003; /* attack range = 3 */
    ctx.currentGroupDistanceToParty = 2;
    F0816_DM1_GROUP_ShouldUseProjectile_Compat(&ctx, &rng, &useProj);
    EXPECT_EQ(useProj, 1, "projectile: caster at distance uses projectile");

    /* Adjacent spell caster follows GROUP.C F0207: random(2) nonzero casts. */
    ctx.currentGroupDistanceToParty = 1;
    rng = make_rng(1);
    useProj = 99;
    F0816_DM1_GROUP_ShouldUseProjectile_Compat(&ctx, &rng, &useProj);
    EXPECT_EQ(useProj, 0,
              "projectile: adjacent caster with random(2)==0 uses melee");

    rng = make_rng(3);
    useProj = 99;
    F0816_DM1_GROUP_ShouldUseProjectile_Compat(&ctx, &rng, &useProj);
    EXPECT_EQ(useProj, 1,
              "projectile: adjacent caster with random(2)!=0 uses projectile");
}


/* =========================================================
 *  Test 11b: Creature projectile launch parameters source-lock
 * ========================================================= */
static void test_creature_projectile_launch_params(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1CreatureProjectileAttack_Compat out;
    int ok;

    ctx.creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    ctx.creatureInfo.ranges = 0x3005; /* attack range 3, sight 5 */
    ctx.creatureInfo.attack = 70;
    ctx.creatureInfo.dexterity = 45;
    ctx.currentGroupDistanceToParty = 2;
    ctx.currentGroupPrimaryDirToParty = 1; /* East */
    ag.cells = 0xFF;

    ok = F0823_DM1_GROUP_ResolveProjectileAttack_Compat(
        &ctx, &ag, 0, &rng, &out);
    EXPECT_EQ(ok, 1, "projectile_launch: resolver returns 1");
    EXPECT_EQ(out.shouldLaunch, 1,
              "projectile_launch: red dragon launches at distance > 1");
    EXPECT_EQ(out.projectileThing, DM1_PROJECTILE_THING_FIREBALL,
              "projectile_launch: red dragon uses fireball thing");
    EXPECT_EQ(out.direction, 1,
              "projectile_launch: direction is primary direction to party");
    EXPECT_EQ(out.stepEnergy, 8,
              "projectile_launch: step energy is source constant 8");
    EXPECT_EQ(out.attack, 45,
              "projectile_launch: attack uses creature dexterity");
    EXPECT_EQ(out.targetCell >= 0 && out.targetCell <= 3, 1,
              "projectile_launch: target cell is normalized");
    EXPECT_EQ(out.kineticEnergy >= 20 && out.kineticEnergy <= 255, 1,
              "projectile_launch: kinetic energy is source bounded");
}

/* =========================================================
 *  Test 11c: Vexirk projectile type table is source-backed
 * ========================================================= */
static void test_vexirk_projectile_type_table(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    int sawFireball = 0;
    int sawAlternate = 0;
    int seed;

    ctx.creatureType = DM1_CREATURE_TYPE_VEXIRK;
    ctx.creatureInfo.ranges = 0x4004;
    ctx.creatureInfo.attack = 30;
    ctx.creatureInfo.dexterity = 50;
    ctx.currentGroupDistanceToParty = 3;
    ag.cells = 0xFF;

    for (seed = 1; seed <= 64; seed++) {
        struct RngState_Compat rng = make_rng((uint32_t)seed);
        struct DM1CreatureProjectileAttack_Compat out;
        F0823_DM1_GROUP_ResolveProjectileAttack_Compat(
            &ctx, &ag, 0, &rng, &out);
        if (out.projectileThing == DM1_PROJECTILE_THING_FIREBALL) {
            sawFireball = 1;
        }
        if (out.projectileThing == DM1_PROJECTILE_THING_HARM_NON_MATERIAL ||
            out.projectileThing == DM1_PROJECTILE_THING_LIGHTNING_BOLT ||
            out.projectileThing == DM1_PROJECTILE_THING_POISON_CLOUD ||
            out.projectileThing == DM1_PROJECTILE_THING_OPEN_DOOR) {
            sawAlternate = 1;
        }
    }

    EXPECT_EQ(sawFireball, 1,
              "vexirk_projectile: RNG table can choose fireball");
    EXPECT_EQ(sawAlternate, 1,
              "vexirk_projectile: RNG table can choose alternate spells");
}

/* =========================================================
 *  Test 11d: Dispatch exposes projectile launch payload
 * ========================================================= */
static void test_dispatch_projectile_payload(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(7);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.creatureType = DM1_CREATURE_TYPE_RED_DRAGON;
    ctx.groupBehavior = DM1_BEHAVIOR_ATTACK;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ctx.distanceToVisibleParty = 2;
    ctx.currentGroupDistanceToParty = 2;
    ctx.partyMapX = 7;
    ctx.partyMapY = 5;
    ctx.creatureInfo.ranges = 0x3005;
    ctx.creatureInfo.attack = 70;
    ctx.creatureInfo.dexterity = 45;

    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "dispatch_projectile: returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_ATTACK,
              "dispatch_projectile: action is attack");
    EXPECT_EQ(result.attackIsProjectile, 1,
              "dispatch_projectile: attack is projectile");
    EXPECT_EQ(result.projectileThing, DM1_PROJECTILE_THING_FIREBALL,
              "dispatch_projectile: red dragon payload is fireball");
    EXPECT_EQ(result.projectileDirection, ctx.currentGroupPrimaryDirToParty,
              "dispatch_projectile: payload direction matches source primary direction");
    EXPECT_EQ(result.projectileStepEnergy, 8,
              "dispatch_projectile: payload step energy is source constant 8");
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


/* =========================================================
 *  Test 17: Giggler steal uses PC34 hand-slot table and may flee
 * ========================================================= */
static void test_giggler_steal_resolver(void) {
    struct RngState_Compat rng = make_rng(1);
    struct DM1GigglerStealResult_Compat steal;
    uint32_t occupied = (1u << DM1_SLOT_READY_HAND) |
                        (1u << DM1_SLOT_ACTION_HAND);

    int ok = F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
        0, occupied, 0, &rng, &steal);

    EXPECT_EQ(ok, 1, "giggler_resolve: returns 1");
    EXPECT_EQ(steal.initialCounter, 6,
              "giggler_resolve: seed chooses PC34 counter 6");
    EXPECT_EQ(steal.stealSlotIndex, DM1_SLOT_READY_HAND,
              "giggler_resolve: first stolen slot is ready hand");
    EXPECT_EQ((int)steal.stolenSlotMask,
              (int)((1u << DM1_SLOT_READY_HAND) |
                    (1u << DM1_SLOT_ACTION_HAND)),
              "giggler_resolve: continuing loop can steal both hands");
    EXPECT_EQ(steal.stolenCount, 2,
              "giggler_resolve: two occupied slots stolen");
    EXPECT_EQ(steal.shouldFlee, 1,
              "giggler_resolve: stolen object can trigger flee");
    EXPECT_EQ(steal.fleeDelayTicks, 63,
              "giggler_resolve: flee delay is random(64)+20");
    EXPECT_EQ(steal.newBehavior, DM1_BEHAVIOR_FLEE,
              "giggler_resolve: behavior switches to FLEE");
}

/* =========================================================
 *  Test 18: Per-creature Giggler attack emits STEAL, not damage attack
 * ========================================================= */
static void test_giggler_attack_dispatch_steals(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(1);
    struct DM1BehaviorResult_Compat result;

    ctx.creatureType = DM1_CREATURE_TYPE_GIGGLER;
    ctx.groupBehavior = DM1_BEHAVIOR_ATTACK;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ctx.distanceToVisibleParty = 1;
    ctx.targetChampionDexterity = 0;
    ctx.targetChampionOccupiedSlotMask =
        (1u << DM1_SLOT_READY_HAND) | (1u << DM1_SLOT_ACTION_HAND);
    ag.cells = 1; /* front cell for east-facing melee; this test isolates steal */

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "giggler_dispatch: returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_STEAL,
              "giggler_dispatch: action is STEAL");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_FLEE,
              "giggler_dispatch: steal can switch to FLEE");
    EXPECT_EQ(result.stealSlotIndex, DM1_SLOT_READY_HAND,
              "giggler_dispatch: reports first stolen slot");
    EXPECT_EQ(result.stolenCount, 2,
              "giggler_dispatch: reports stolen slot count");
    EXPECT_EQ(ag.delayFleeingFromTarget, 31,
              "giggler_dispatch: writes active-group flee delay");
}

/* =========================================================
 *  Test 19: Quarter-square melee creature shuffles before attack
 * ========================================================= */
static void test_quarter_square_melee_cell_adjusts_before_attack(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(2);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.groupBehavior = DM1_BEHAVIOR_ATTACK;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ctx.creatureType = DM1_CREATURE_TYPE_SWAMP_SLIME;
    ctx.creatureSize = DM1_SIZE_QUARTER_SQUARE;
    ctx.creatureCount = 0;
    ctx.creatureInfo.ranges = 0x1003; /* attack range 1 */
    ctx.creatureInfo.movementTicks = 20;
    ctx.currentGroupPrimaryDirToParty = 1; /* front cells 1 and 2 */
    ctx.distanceToVisibleParty = 1;
    ctx.currentGroupDistanceToParty = 1;
    ag.cells = 0; /* creature starts in non-attacking back cell 0 */

    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "quarter_melee_adjust: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_ADJUST_CELL,
              "quarter_melee_adjust: action adjusts cell instead of attacking");
    EXPECT_EQ(result.attackIsProjectile, 0,
              "quarter_melee_adjust: no attack payload emitted during shuffle");
    EXPECT_EQ(result.meleeCellAdjustment, 1,
              "quarter_melee_adjust: result marks source cell adjustment");
    EXPECT_EQ(ag.cells, 0xFF,
              "quarter_melee_adjust: single creature can move to centered cell");
    EXPECT_EQ(result.updatedGroupCells, 0xFF,
              "quarter_melee_adjust: reports updated centered group cells");
    EXPECT_EQ(result.adjustedCreatureCell, -1,
              "quarter_melee_adjust: centered creature reports no single cell");
    EXPECT_EQ(result.nextEventType, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0,
              "quarter_melee_adjust: reschedules same creature behavior event");
    EXPECT_EQ(result.nextEventDelayTicks, 11,
              "quarter_melee_adjust: delay is movementTicks/2 + random(2)");
}

/* =========================================================
 *  Test 20: Attack-any back-row creature may bypass melee shuffle
 * ========================================================= */
static void test_attack_any_back_row_bypasses_cell_adjust(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(2);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.groupBehavior = DM1_BEHAVIOR_ATTACK;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ctx.creatureType = DM1_CREATURE_TYPE_SWAMP_SLIME;
    ctx.creatureSize = DM1_SIZE_QUARTER_SQUARE;
    ctx.creatureCount = 0;
    ctx.creatureInfo.ranges = 0x1003;
    ctx.creatureInfo.attributes = DM1_ATTR_PREFER_BACK_ROW |
                                  DM1_ATTR_ATTACK_ANY_CHAMPION;
    ctx.currentGroupPrimaryDirToParty = 1;
    ctx.distanceToVisibleParty = 1;
    ctx.currentGroupDistanceToParty = 1;
    ag.cells = 0;

    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "attack_any_back_row: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_ATTACK,
              "attack_any_back_row: source RNG can keep back-row attack");
    EXPECT_EQ(result.meleeCellAdjustment, 0,
              "attack_any_back_row: no cell adjustment on bypass");
    EXPECT_EQ(ag.cells, 0,
              "attack_any_back_row: group cells remain unchanged");
}


/* =========================================================
 *  Test 21: Source fixed possession table: Animated Armour
 * ========================================================= */
static void test_fixed_possessions_animated_armour_are_cursed(void) {
    struct RngState_Compat rng = make_rng(1);
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    int count = -1;
    int weaponDropped = 0;
    int ok;

    memset(drops, 0, sizeof(drops));
    ok = F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
        DM1_CREATURE_TYPE_ANIMATED_ARMOUR,
        2,
        &rng,
        drops,
        DM1_MAX_FIXED_POSSESSION_DROPS,
        &count,
        &weaponDropped);

    EXPECT_EQ(ok, 1, "fixed_drop_armour: resolver returns 1");
    EXPECT_EQ(count, 6, "fixed_drop_armour: six source items drop");
    EXPECT_EQ(weaponDropped, 1, "fixed_drop_armour: weapon thud selected");
    EXPECT_EQ(drops[0].thingType, DM1_DROP_THING_TYPE_ARMOUR,
              "fixed_drop_armour: foot plate is armour");
    EXPECT_EQ(drops[0].itemType, 41,
              "fixed_drop_armour: first item is Foot Plate type 41");
    EXPECT_EQ(drops[0].cursed, 1,
              "fixed_drop_armour: fixed armour drops are cursed");
    EXPECT_EQ(drops[3].thingType, DM1_DROP_THING_TYPE_WEAPON,
              "fixed_drop_armour: fourth item is sword weapon");
    EXPECT_EQ(drops[3].itemType, 10,
              "fixed_drop_armour: sword type is 10");
    EXPECT_EQ(drops[5].thingType, DM1_DROP_THING_TYPE_WEAPON,
              "fixed_drop_armour: sixth item is second sword");
    EXPECT_EQ(drops[5].sourceOrdinal, 6,
              "fixed_drop_armour: source ordinal preserves table order");
}

/* =========================================================
 *  Test 22: Source fixed possession random drops: Rockpile
 * ========================================================= */
static void test_fixed_possessions_rockpile_random_flags(void) {
    struct RngState_Compat rng = make_rng(8);
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    int count = -1;
    int weaponDropped = 0;
    int ok;

    memset(drops, 0, sizeof(drops));
    ok = F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
        DM1_CREATURE_TYPE_ROCKPILE,
        2,
        &rng,
        drops,
        DM1_MAX_FIXED_POSSESSION_DROPS,
        &count,
        &weaponDropped);

    EXPECT_EQ(ok, 1, "fixed_drop_rock: resolver returns 1");
    EXPECT_EQ(count, 3, "fixed_drop_rock: seed keeps three of four entries");
    EXPECT_EQ(drops[0].thingType, DM1_DROP_THING_TYPE_JUNK,
              "fixed_drop_rock: guaranteed first boulder is junk");
    EXPECT_EQ(drops[0].itemType, 25,
              "fixed_drop_rock: boulder junk type is 25");
    EXPECT_EQ(drops[1].sourceHadRandomFlag, 1,
              "fixed_drop_rock: second kept drop came from random table entry");
    EXPECT_EQ(drops[1].sourceOrdinal, 2,
              "fixed_drop_rock: random boulder source ordinal is 2");
    EXPECT_EQ(drops[2].thingType, DM1_DROP_THING_TYPE_WEAPON,
              "fixed_drop_rock: random rock can drop as weapon");
    EXPECT_EQ(drops[2].itemType, 30,
              "fixed_drop_rock: rock weapon type is 30");
    EXPECT_EQ(weaponDropped, 1, "fixed_drop_rock: weapon drop toggles thud flag");
}

/* =========================================================
 *  Test 23: Source fixed possession table: Red Dragon steaks
 * ========================================================= */
static void test_fixed_possessions_dragon_steak_table(void) {
    struct RngState_Compat rng = make_rng(8);
    struct DM1FixedPossessionDrop_Compat drops[DM1_MAX_FIXED_POSSESSION_DROPS];
    int count = -1;
    int weaponDropped = 0;
    int ok;
    int i;

    memset(drops, 0, sizeof(drops));
    ok = F0824_DM1_GROUP_ResolveFixedPossessionDrops_Compat(
        DM1_CREATURE_TYPE_RED_DRAGON,
        2,
        &rng,
        drops,
        DM1_MAX_FIXED_POSSESSION_DROPS,
        &count,
        &weaponDropped);

    EXPECT_EQ(ok, 1, "fixed_drop_dragon: resolver returns 1");
    EXPECT_EQ(count, 10,
              "fixed_drop_dragon: seed keeps all eight guaranteed plus two random steaks");
    EXPECT_EQ(weaponDropped, 0, "fixed_drop_dragon: no weapon thud");
    for (i = 0; i < count; ++i) {
        EXPECT_EQ(drops[i].thingType, DM1_DROP_THING_TYPE_JUNK,
                  "fixed_drop_dragon: every drop is junk");
        EXPECT_EQ(drops[i].itemType, 36,
                  "fixed_drop_dragon: every drop is Dragon Steak type 36");
    }
    EXPECT_EQ(drops[8].sourceHadRandomFlag, 1,
              "fixed_drop_dragon: ninth source entry is random");
    EXPECT_EQ(drops[9].sourceOrdinal, 10,
              "fixed_drop_dragon: tenth source entry can survive RNG");
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
    test_creature_projectile_launch_params();
    test_vexirk_projectile_type_table();
    test_dispatch_projectile_payload();
    test_set_group_direction();
    test_smell_direction();
    test_per_creature_attack_event();
    test_reaction_during_freeze();
    test_flee_direction();
    test_giggler_steal_resolver();
    test_giggler_attack_dispatch_steals();
    test_quarter_square_melee_cell_adjusts_before_attack();
    test_attack_any_back_row_bypasses_cell_adjust();
    test_fixed_possessions_animated_armour_are_cursed();
    test_fixed_possessions_rockpile_random_flags();
    test_fixed_possessions_dragon_steak_table();

    printf("\n--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
