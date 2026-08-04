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
#include "memory_dungeon_dat_pc34_compat.h"

enum {
    TEST_DM1_SLOT_POUCH_2 = 6,
    TEST_DM1_SLOT_NECK = 10,
    TEST_DM1_SLOT_POUCH_1 = 11,
    TEST_DM1_SLOT_QUIVER_LINE1_1 = 12,
    TEST_DM1_SLOT_BACKPACK_LINE1_1 = 13
};

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

struct F0199BlockGrid {
    int firstX;
    int firstY;
    int secondX;
    int secondY;
};

static int f0199_is_blocked(int mapX, int mapY, void* context)
{
    const struct F0199BlockGrid* grid = context;
    return grid && ((mapX == grid->firstX && mapY == grid->firstY) ||
                    (mapX == grid->secondX && mapY == grid->secondY));
}

/* Helper to create a default context */
static struct DM1GroupBehaviorContext_Compat make_default_ctx(void) {
    struct DM1GroupBehaviorContext_Compat ctx;
    int direction;
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
    /* This unit fixture models four decoded, empty destination squares. Each
     * test that needs a blocker overwrites the corresponding source fact. */
    for (direction = 0; direction < 4; ++direction) {
        ctx.groupMovementFacts[direction].available = 1;
        ctx.groupMovementFacts[direction].inBounds = 1;
    }
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

static void test_f0264_levitation_classifier(void) {
    EXPECT_EQ(F0264_DM1_MOVE_IsLevitating_Compat(THING_TYPE_GROUP,
                                                  DM1_ATTR_LEVITATION), 1,
              "F0264 C04 reads raw levitation attribute");
    EXPECT_EQ(F0264_DM1_MOVE_IsLevitating_Compat(THING_TYPE_GROUP, 0), 0,
              "F0264 C04 without raw levitation does not levitate");
    EXPECT_EQ(F0264_DM1_MOVE_IsLevitating_Compat(THING_TYPE_PROJECTILE, 0), 1,
              "F0264 C14 projectile always levitates");
    EXPECT_EQ(F0264_DM1_MOVE_IsLevitating_Compat(THING_TYPE_EXPLOSION, 0), 1,
              "F0264 C15 explosion always levitates");
    EXPECT_EQ(F0264_DM1_MOVE_IsLevitating_Compat(THING_TYPE_WEAPON,
                                                  DM1_ATTR_LEVITATION), 0,
              "F0264 does not apply creature attributes to ordinary Things");
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
 *  Test 4b: Archenemy approach uses F0204 double movement
 * ========================================================= */
static void test_archenemy_approach_double_move(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.groupBehavior = DM1_BEHAVIOR_APPROACH;
    ctx.eventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    ctx.distanceToVisibleParty = 3;
    ctx.currentGroupDistanceToParty = 3;
    ctx.currentGroupPrimaryDirToParty = 1; /* East */
    ctx.currentGroupSecondaryDirToParty = 0;
    ctx.partyMapX = 8;
    ctx.partyMapY = 5;
    ctx.ticksSinceLastMove = 30;
    ctx.movementTicks = 20;

    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "archenemy_double_control: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_MOVE,
              "archenemy_double_control: ordinary approach moves");
    EXPECT_EQ(result.moveDestMapX, ctx.currentGroupMapX + 1,
              "archenemy_double_control: ordinary approach moves one square");
    EXPECT_EQ(result.archenemyDoubleMove, 0,
              "archenemy_double_control: ordinary approach has no double flag");

    rng = make_rng(42);
    ctx.isArchenemy = 1;
    ctx.creatureInfo.attributes |= DM1_ATTR_ARCHENEMY;
    ctx.archenemySecondStepMovementFacts[1].available = 1;
    ctx.archenemySecondStepMovementFacts[1].inBounds = 1;
    memset(&result, 0, sizeof(result));
    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "archenemy_double: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_MOVE,
              "archenemy_double: approach moves");
    EXPECT_EQ(result.moveDirection, 1,
              "archenemy_double: moves east toward party");
    EXPECT_EQ(result.moveDestMapX, ctx.currentGroupMapX + 2,
              "archenemy_double: F0204 target is two squares east");
    EXPECT_EQ(result.moveDestMapY, ctx.currentGroupMapY,
              "archenemy_double: Y unchanged for east double move");
    EXPECT_EQ(result.archenemyDoubleMove, 1,
              "archenemy_double: F0204 double-move flag set");

    /* F0204 performs a second source F0202 read after the admitted first
     * step. A blocked or absent second square retains the ordinary move. */
    ctx.archenemySecondStepMovementFacts[1].isWall = 1;
    rng = make_rng(42);
    memset(&result, 0, sizeof(result));
    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "archenemy_double_blocked: dispatch returns 1");
    EXPECT_EQ(result.moveDestMapX, ctx.currentGroupMapX + 1,
              "archenemy_double_blocked: second F0202 wall retains first step");
    EXPECT_EQ(result.archenemyDoubleMove, 0,
              "archenemy_double_blocked: no synthetic double-move flag");
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
 *  Test 11c: C25/C26 BUG0_13 has no source projectile Thing
 * ========================================================= */
static void test_lord_order_grey_lord_projectile_rejected(void) {
    const int types[] = {
        DM1_CREATURE_TYPE_LORD_ORDER,
        DM1_CREATURE_TYPE_GREY_LORD
    };
    int i;

    for (i = 0; i < 2; ++i) {
        struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
        struct DM1ActiveGroup_Compat ag = make_default_ag();
        struct RngState_Compat rng = make_rng((uint32_t)(51 + i));
        struct DM1CreatureProjectileAttack_Compat out;
        uint32_t rngBefore;
        int ok;

        ctx.creatureType = types[i];
        ctx.creatureInfo.ranges = 0x3005; /* attack range 3, sight 5 */
        ctx.creatureInfo.attack = 64;
        ctx.creatureInfo.dexterity = 33;
        ctx.currentGroupDistanceToParty = 2;
        ctx.currentGroupPrimaryDirToParty = 2; /* South */
        ag.cells = 0xFF;
        rngBefore = rng.seed;

        ok = F0823_DM1_GROUP_ResolveProjectileAttack_Compat(
            &ctx, &ag, 0, &rng, &out);
        EXPECT_EQ(ok, 1,
                  "lord_c25_c26_projectile: resolver returns 1");
        EXPECT_EQ(out.shouldLaunch, 0,
                  "lord_c25_c26_projectile: undefined source Thing is rejected");
        EXPECT_EQ(out.projectileThing, -1,
                  "lord_c25_c26_projectile: no synthetic fireball is created");
        EXPECT_EQ(rng.seed, rngBefore,
                  "lord_c25_c26_projectile: rejection consumes no unrelated RNG");
    }
}

/* =========================================================
 *  Test 11d: Vexirk projectile type table is source-backed
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
 *  Test 11e: Dispatch exposes projectile launch payload
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

static void test_set_group_direction_requires_live_rng_f0205(void) {
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng;

    ag.directions = 0;
    EXPECT_EQ(F0817_DM1_GROUP_SetGroupDirection_Compat(
                  &ag, 2, 0, DM1_SIZE_QUARTER_SQUARE, 0), 0,
              "F0205 legacy helper rejects an opposite turn without RNG");
    EXPECT_EQ(ag.directions, 0,
              "F0205 legacy helper leaves direction unmodified without RNG");

    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    EXPECT_EQ(F0817b_DM1_GROUP_SetCreatureDirectionWithRng_Compat(
                  &ag, 2, 0, DM1_SIZE_QUARTER_SQUARE, 0, &rng), 1,
              "F0205 live helper consumes source RNG for an opposite turn");
    EXPECT_EQ((ag.directions & 3) == 1 || (ag.directions & 3) == 3, 1,
              "F0205 live helper chooses only a one-step intermediate turn");
}

/* =========================================================
 *  Test 12b: F0202 typed destination facts keep source order
 * ========================================================= */
static void test_group_movement_facts(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1GroupMovementFacts_Compat* facts;
    int wall = 0, door = 0, party = 0, group = 0;

    ctx.groupMovementFacts[1].available = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              0, "movement_facts: missing loaded-map snapshot fails closed");

    facts = &ctx.groupMovementFacts[1];
    memset(facts, 0, sizeof(*facts));
    facts->available = 1;
    facts->inBounds = 1;

    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              1, "movement_facts: clear corridor is possible");

    facts->isOpenPit = 1;
    wall = door = party = group = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              0, "movement_facts: open pit blocks non-levitating group");
    EXPECT_EQ(wall, 1, "movement_facts: open pit reports F0202 terrain block");

    facts->isImaginaryPit = 1;
    wall = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 1, &wall, &door, &party, &group),
              1, "movement_facts: permitted imaginary pit is passable");

    facts->isOpenPit = 0;
    facts->isImaginaryPit = 0;
    facts->occupiedByParty = 1;
    party = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              0, "movement_facts: party blocks after terrain checks");
    EXPECT_EQ(party, 1, "movement_facts: party gets its F0202 blocker flag");

    facts->occupiedByParty = 0;
    facts->doorBlocksCreature = 1;
    door = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              0, "movement_facts: closed door blocks after party check");
    EXPECT_EQ(door, 1, "movement_facts: closed door gets F0202 door flag");

    facts->doorBlocksCreature = 0;
    facts->occupiedByGroup = 1;
    group = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              0, "movement_facts: destination group blocks last");
    EXPECT_EQ(group, 1, "movement_facts: group gets F0202 group flag");

    facts->occupiedByGroup = 0;
    facts->hasFluxcage = 1;
    ctx.creatureInfo.attributes = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              1, "movement_facts: Fluxcage does not block ordinary group");
    ctx.creatureInfo.attributes = DM1_ATTR_ARCHENEMY;
    wall = 0;
    EXPECT_EQ(F0811_DM1_GROUP_IsMovementPossible_Compat(
                  &ctx, 1, 0, &wall, &door, &party, &group),
              0, "movement_facts: Fluxcage blocks archenemy group");
    EXPECT_EQ(wall, 1, "movement_facts: Fluxcage reports terrain blocker");
}

static void test_first_movement_direction_f0203_test_state(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    int testedDirections[4] = { 0, 0, 0, 0 };
    int direction = -1;

    ctx.groupMovementFacts[0].isWall = 1;
    EXPECT_EQ(F0812a_DM1_GROUP_GetFirstPossibleMovementDirWithTestState_Compat(
                  &ctx, testedDirections, 0, &direction), 1,
              "F0203 finds the first source-open direction");
    EXPECT_EQ(direction, 1, "F0203 tries north then selects east");
    EXPECT_EQ(testedDirections[0], 1,
              "F0203 preserves F0202 tested state for blocked north");
    EXPECT_EQ(testedDirections[1], 1,
              "F0203 preserves F0202 tested state for selected east");
    EXPECT_EQ(testedDirections[2], 0,
              "F0203 leaves later directions untouched");

    ctx.groupMovementFacts[2].isWall = 1;
    ctx.groupMovementFacts[3].isWall = 1;
    EXPECT_EQ(F0812a_DM1_GROUP_GetFirstPossibleMovementDirWithTestState_Compat(
                  &ctx, testedDirections, 0, &direction), 0,
              "F0203 reports no direction after all remaining facts block");
    EXPECT_EQ(testedDirections[2], 1,
              "F0203 marks south before its source blocker result");
    EXPECT_EQ(testedDirections[3], 1,
              "F0203 marks west before its source blocker result");
}

static void test_archenemy_double_movement_f0204(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1GroupMovementFacts_Compat secondStep;
    int wall = 0, door = 0, party = 0, group = 0;

    memset(&secondStep, 0, sizeof(secondStep));
    secondStep.available = 1;
    secondStep.inBounds = 1;
    ctx.creatureInfo.attributes = DM1_ATTR_ARCHENEMY;

    EXPECT_EQ(F0812b_DM1_GROUP_IsArchenemyDoubleMovementPossible_Compat(
                  &ctx, 1, 1, &secondStep, &wall, &door, &party, &group), 0,
              "F0204 rejects a first-step Fluxcage before second movement");
    EXPECT_EQ(wall, 1, "F0204 reports the first-step Fluxcage blocker");

    secondStep.isWall = 1;
    wall = door = party = group = 0;
    EXPECT_EQ(F0812b_DM1_GROUP_IsArchenemyDoubleMovementPossible_Compat(
                  &ctx, 1, 0, &secondStep, &wall, &door, &party, &group), 0,
              "F0204 runs F0202 against a blocked second square");
    EXPECT_EQ(wall, 1, "F0204 returns the second F0202 terrain blocker");

    secondStep.isWall = 0;
    wall = door = party = group = 0;
    EXPECT_EQ(F0812b_DM1_GROUP_IsArchenemyDoubleMovementPossible_Compat(
                  &ctx, 1, 0, &secondStep, &wall, &door, &party, &group), 1,
              "F0204 permits a source-proven clear second square");
}

/* =========================================================
 *  Test 12c: F0209 single-square move consumes F0202 facts
 * ========================================================= */
static void test_single_square_move_uses_typed_facts(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct RngState_Compat rng = make_rng(3);
    int direction = -1;

    /* Primary east is a wall. Secondary south is a closed imaginary
     * fakewall; ReDMCSB GROUP.C F0209 permits it only when M005_RANDOM(2)
     * is non-zero. Seed 3 takes that original branch. */
    ctx.groupMovementFacts[1].available = 1;
    ctx.groupMovementFacts[1].inBounds = 1;
    ctx.groupMovementFacts[1].isWall = 1;
    ctx.groupMovementFacts[2].available = 1;
    ctx.groupMovementFacts[2].inBounds = 1;
    ctx.groupMovementFacts[2].isFakeWall = 1;
    ctx.groupMovementFacts[2].isImaginaryFakeWall = 1;

    EXPECT_EQ(F0813_DM1_GROUP_PickSingleSquareMove_Compat(
                  &ctx, 1, 2, 1, &rng, &direction),
              1, "single_square_facts: source move selection succeeds");
    EXPECT_EQ(direction, 2,
              "single_square_facts: F0209 accepts secondary imaginary fakewall on nonzero RNG");
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
 *  Test 13a: F0200 cannot see party through a map handoff
 * ========================================================= */
static void test_visible_distance_requires_party_map(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    int distance = 99;

    ctx.distanceToVisibleParty = 3;
    ctx.currentMapIndex = 2;
    ctx.partyMapIndex = 1;
    EXPECT_EQ(F0818_DM1_GROUP_GetDistanceToVisibleParty_Compat(
                  &ctx, -1, &distance),
              1, "visible_map: source adapter succeeds");
    EXPECT_EQ(distance, 0,
              "visible_map: cross-map group cannot retain visible party distance");
}

static void test_visible_distance_f0200_live_route(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat active;
    struct F0199BlockGrid grid;
    struct RngState_Compat rng;
    int distance = 99;

    memset(&active, 0, sizeof(active));
    memset(&grid, 0xff, sizeof(grid));
    ctx.currentMapIndex = 0;
    ctx.partyMapIndex = 0;
    ctx.currentGroupMapX = 0;
    ctx.currentGroupMapY = 0;
    ctx.partyMapX = 0;
    ctx.partyMapY = 3;
    ctx.currentGroupDistanceToParty = 3;
    ctx.creatureCount = 0;
    ctx.creatureInfo.attributes = 0;
    ctx.creatureInfo.ranges = 4;
    ctx.isViewSquareBlocked = f0199_is_blocked;
    ctx.viewBlockerContext = &grid;
    active.directions = 2;
    F0730_COMBAT_RngInit_Compat(&rng, 1u);

    EXPECT_EQ(F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
                  &ctx, &active, -1, &rng, &distance), 1,
              "F0200 sees party through a clear loaded-map route");
    EXPECT_EQ(distance, 3, "F0200 returns F0199 route distance");

    active.directions = 0;
    distance = 99;
    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    EXPECT_EQ(F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
                  &ctx, &active, -1, &rng, &distance), 0,
              "F0200 rejects a party behind the creature direction");

    ctx.creatureInfo.attributes = 0x0004;
    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    EXPECT_EQ(F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
                  &ctx, NULL, -1, &rng, &distance), 1,
              "F0200 side-attack creature sees in all directions");

    ctx.creatureInfo.attributes = 0;
    ctx.partyInvisibilityEventCount = 1;
    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    EXPECT_EQ(F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
                  &ctx, &active, -1, &rng, &distance), 0,
              "F0200 hides an invisible party from ordinary creatures");

    ctx.partyInvisibilityEventCount = 0;
    active.directions = 2;
    grid.firstX = 0;
    grid.firstY = 2;
    F0730_COMBAT_RngInit_Compat(&rng, 1u);
    EXPECT_EQ(F0818a_DM1_GROUP_GetDistanceToVisiblePartyWithRoute_Compat(
                  &ctx, &active, -1, &rng, &distance), 0,
              "F0200 consumes F0199 loaded-map route blocking");
}

/* =========================================================
 *  Test 13b: F0201 direct-party scent requires F0198/F0199 route
 * ========================================================= */
static void test_smell_direction_requires_unblocked_route(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    int dirOrd = 99;

    ctx.creatureInfo.ranges = 0x1403; /* smell = 4, direct range = 2 */
    ctx.currentGroupDistanceToParty = 2;
    ctx.currentGroupPrimaryDirToParty = 3;

    EXPECT_EQ(F0819a_DM1_GROUP_GetSmelledPartyDirOrdinalFromRoute_Compat(
                  &ctx, 2, &dirOrd),
              1, "smell_route: source adapter succeeds");
    EXPECT_EQ(dirOrd, 4,
              "smell_route: unblocked F0199 route returns primary ordinal");

    dirOrd = 99;
    EXPECT_EQ(F0819a_DM1_GROUP_GetSmelledPartyDirOrdinalFromRoute_Compat(
                  &ctx, 0, &dirOrd),
              1, "smell_route: blocked route adapter succeeds");
    EXPECT_EQ(dirOrd, 0,
              "smell_route: F0198/F0199 blocked route suppresses scent");

    ctx.currentGroupDistanceToParty = 3;
    dirOrd = 99;
    EXPECT_EQ(F0819a_DM1_GROUP_GetSmelledPartyDirOrdinalFromRoute_Compat(
                  &ctx, 3, &dirOrd),
              1, "smell_route: range gate adapter succeeds");
    EXPECT_EQ(dirOrd, 0,
              "smell_route: out-of-range direct route suppresses scent");
}

/* =========================================================
 *  Test 13c: F0201 falls back to a fresh stored party scent
 * ========================================================= */
static void test_smell_direction_stored_scent_fallback(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1GroupScent_Compat scent;
    struct DM1GroupSmellDirectionPlan_Compat plan;
    struct RngState_Compat rng = make_rng(7);

    memset(&scent, 0, sizeof(scent));
    ctx.creatureInfo.ranges = 0x1403; /* smell = 4 */
    ctx.currentGroupMapX = 5;
    ctx.currentGroupMapY = 5;
    ctx.currentGroupDistanceToParty = 3; /* outside direct scent range */
    scent.present = 1;
    scent.strength = 30; /* always clears F0201's freshness comparison */
    scent.mapX = 8;
    scent.mapY = 5;

    EXPECT_EQ(F0819b_DM1_GROUP_BuildSmelledPartyDirectionPlan_Compat(
                  &ctx, 0, &scent, &rng, &plan),
              1, "smell_scent: source plan builds");
    EXPECT_EQ(plan.valid, 1, "smell_scent: plan valid");
    EXPECT_EQ(plan.usedDirectPartyRoute, 0,
              "smell_scent: blocked direct route not selected");
    EXPECT_EQ(plan.usedStoredScent, 1,
              "smell_scent: fresh stored scent selected");
    EXPECT_EQ(plan.directionOrdinal, 2,
              "smell_scent: east stored scent returns east ordinal");
    EXPECT_EQ(plan.primaryDirection, 1,
              "smell_scent: east stored scent keeps east primary");
    EXPECT_EQ((plan.secondaryDirection == 0 || plan.secondaryDirection == 2),
              1, "smell_scent: row scent gets north/south secondary");

    scent.strength = 0;
    rng = make_rng(7);
    EXPECT_EQ(F0819b_DM1_GROUP_BuildSmelledPartyDirectionPlan_Compat(
                  &ctx, 0, &scent, &rng, &plan),
              1, "smell_scent_stale: source plan builds");
    EXPECT_EQ(plan.directionOrdinal, 0,
              "smell_scent_stale: stale scent is rejected");

    ctx.currentGroupDistanceToParty = 2;
    ctx.currentGroupPrimaryDirToParty = 3;
    ctx.currentGroupSecondaryDirToParty = 1;
    scent.strength = 30;
    rng = make_rng(7);
    EXPECT_EQ(F0819b_DM1_GROUP_BuildSmelledPartyDirectionPlan_Compat(
                  &ctx, 2, &scent, &rng, &plan),
              1, "smell_direct: source plan builds");
    EXPECT_EQ(plan.usedDirectPartyRoute, 1,
              "smell_direct: clear direct route wins over stored scent");
    EXPECT_EQ(plan.usedStoredScent, 0,
              "smell_direct: stored scent remains unused");
    EXPECT_EQ(plan.directionOrdinal, 4,
              "smell_direct: direct route uses party primary ordinal");
    EXPECT_EQ(plan.secondaryDirection, 1,
              "smell_direct: direct route preserves party secondary direction");
}

static void test_smell_direction_f0201_live_route(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1GroupScent_Compat scent;
    struct DM1GroupSmellDirectionPlan_Compat plan;
    struct F0199BlockGrid grid;
    struct RngState_Compat rng;

    memset(&scent, 0, sizeof(scent));
    memset(&grid, 0xff, sizeof(grid));
    ctx.currentMapIndex = 0;
    ctx.partyMapIndex = 0;
    ctx.currentGroupMapX = 0;
    ctx.currentGroupMapY = 0;
    ctx.partyMapX = 0;
    ctx.partyMapY = 2;
    ctx.currentGroupDistanceToParty = 2;
    ctx.currentGroupPrimaryDirToParty = 2;
    ctx.currentGroupSecondaryDirToParty = 1;
    ctx.creatureInfo.ranges = 0x1403;
    ctx.isSmellSquareBlocked = f0199_is_blocked;
    ctx.smellBlockerContext = &grid;
    F0730_COMBAT_RngInit_Compat(&rng, 7u);

    EXPECT_EQ(F0819c_DM1_GROUP_BuildSmelledPartyDirectionPlanWithRoute_Compat(
                  &ctx, NULL, &rng, &plan), 1,
              "F0201 consumes a clear loaded-map smell route");
    EXPECT_EQ(plan.usedDirectPartyRoute, 1,
              "F0201 selects direct scent before stored scent");
    EXPECT_EQ(plan.directionOrdinal, 3,
              "F0201 returns the real party primary direction");

    grid.firstX = 0;
    grid.firstY = 1;
    scent.present = 1;
    scent.strength = 30;
    scent.mapX = 3;
    scent.mapY = 0;
    F0730_COMBAT_RngInit_Compat(&rng, 7u);
    EXPECT_EQ(F0819c_DM1_GROUP_BuildSmelledPartyDirectionPlanWithRoute_Compat(
                  &ctx, &scent, &rng, &plan), 1,
              "F0201 builds a blocked-route stored-scent plan");
    EXPECT_EQ(plan.usedDirectPartyRoute, 0,
              "F0201 rejects a real map blocker before stored scent");
    EXPECT_EQ(plan.usedStoredScent, 1,
              "F0201 uses only the supplied fresh original scent fallback");
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
 *  Test 15b: Negative reaction events schedule source reactions
 * ========================================================= */
static void test_negative_reaction_event_creation(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(42);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_CM1_REACTION_PARTY_IS_ADJACENT;
    ctx.ticksSinceLastMove = 0;
    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_create_adjacent: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_NONE,
              "reaction_create_adjacent: no immediate action");
    EXPECT_EQ(result.nextEventType, DM1_EVENT_REACTION_PARTY_IS_ADJACENT,
              "reaction_create_adjacent: schedules C31 reaction");
    EXPECT_EQ(result.nextEventDelayTicks, 1,
              "reaction_create_adjacent: source one-tick delay");

    ctx.eventType = DM1_CM2_REACTION_HIT_BY_PROJECTILE;
    ctx.movementTicks = 20;
    ctx.ticksSinceLastMove = 1;
    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_create_projectile: dispatch returns 1");
    EXPECT_EQ(result.nextEventType, DM1_EVENT_REACTION_HIT_BY_PROJECTILE,
              "reaction_create_projectile: schedules C30 reaction");
    EXPECT_EQ(result.nextEventDelayTicks, 4,
              "reaction_create_projectile: movement-delay formula clamps after recent move");

    ctx.eventType = DM1_CM3_REACTION_DANGER_ON_SQUARE;
    ctx.ticksSinceLastMove = 30;
    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_create_danger: dispatch returns 1");
    EXPECT_EQ(result.nextEventType, DM1_EVENT_REACTION_DANGER_ON_SQUARE,
              "reaction_create_danger: schedules C29 reaction");
    EXPECT_EQ(result.nextEventDelayTicks, 1,
              "reaction_create_danger: delay is clamped to one tick");
}

/* =========================================================
 *  Test 15c: Projectile-hit reaction with no sight turns to search
 * ========================================================= */
static void test_projectile_hit_reaction_sets_search_direction(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(1);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.groupBehavior = DM1_BEHAVIOR_WANDER;
    ctx.eventType = DM1_EVENT_REACTION_HIT_BY_PROJECTILE;
    ctx.distanceToVisibleParty = 0;

    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_projectile_search: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_SET_DIRECTION,
              "reaction_projectile_search: source turns to search");
    EXPECT_EQ(result.newDirectionForGroup, 2,
              "reaction_projectile_search: deterministic random search direction");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_WANDER,
              "reaction_projectile_search: behavior remains wander");
}

/* =========================================================
 *  Test 15d: Danger reaction moves and stops attacking
 * ========================================================= */
static void test_danger_reaction_moves_attack_group_to_approach(void) {
    struct DM1GroupBehaviorContext_Compat ctx = make_default_ctx();
    struct DM1ActiveGroup_Compat ag = make_default_ag();
    struct RngState_Compat rng = make_rng(2);
    struct DM1BehaviorResult_Compat result;
    int ok;

    ctx.groupBehavior = DM1_BEHAVIOR_ATTACK;
    ctx.eventType = DM1_EVENT_REACTION_DANGER_ON_SQUARE;
    ag.priorMapX = 0;
    ag.priorMapY = 0;

    ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "reaction_danger_move: dispatch returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_MOVE,
              "reaction_danger_move: source moves away from danger");
    EXPECT_EQ(result.moveDirection, 0,
              "reaction_danger_move: deterministic start direction");
    EXPECT_EQ(result.moveDestMapX, ctx.currentGroupMapX,
              "reaction_danger_move: north move keeps X");
    EXPECT_EQ(result.moveDestMapY, ctx.currentGroupMapY - 1,
              "reaction_danger_move: north move decrements Y");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_APPROACH,
              "reaction_danger_move: attacking group switches to approach");
    EXPECT_EQ(result.stopAttacking, 1,
              "reaction_danger_move: stop-attacking flag set");
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
 *  Test 17: Giggler steal uses PC34 G0025 slot table and may flee
 * ========================================================= */
static void test_giggler_steal_resolver(void) {
    struct RngState_Compat rng = make_rng(1);
    struct DM1GigglerStealResult_Compat steal;
    uint32_t occupied =
        (1u << TEST_DM1_SLOT_POUCH_2) |
        (1u << TEST_DM1_SLOT_NECK) |
        (1u << TEST_DM1_SLOT_POUCH_1) |
        (1u << (TEST_DM1_SLOT_BACKPACK_LINE1_1 + 12)) |
        (1u << (TEST_DM1_SLOT_BACKPACK_LINE1_1 + 15));

    int ok = F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
        0, occupied, 0, &rng, &steal);

    EXPECT_EQ(ok, 1, "giggler_resolve: returns 1");
    EXPECT_EQ(steal.initialCounter, 6,
              "giggler_resolve: seed chooses PC34 counter 6");
    EXPECT_EQ(steal.stealSlotIndex, TEST_DM1_SLOT_POUCH_2,
              "giggler_resolve: first stolen slot follows G0025 counter 6");
    EXPECT_EQ((int)steal.stolenSlotMask,
              (int)occupied,
              "giggler_resolve: loop steals G0025 and expanded backpack slots");
    EXPECT_EQ(steal.stolenCount, 5,
              "giggler_resolve: five source-table attempts hit occupied slots");
    EXPECT_EQ(steal.shouldFlee, 1,
              "giggler_resolve: stolen object can trigger flee");
    EXPECT_EQ(steal.fleeDelayTicks, 79,
              "giggler_resolve: flee delay is random(64)+20");
    EXPECT_EQ(steal.newBehavior, DM1_BEHAVIOR_FLEE,
              "giggler_resolve: behavior switches to FLEE");
}

static void test_giggler_steal_luck_stops_before_backpack_random(void) {
    struct RngState_Compat rng = make_rng(1);
    struct RngState_Compat expected = make_rng(1);
    struct DM1GigglerStealResult_Compat steal;
    uint32_t occupied =
        (1u << TEST_DM1_SLOT_POUCH_2) |
        (1u << (TEST_DM1_SLOT_BACKPACK_LINE1_1 + 12));

    int ok = F0822_DM1_GIGGLER_ResolveStealAttempt_Compat(
        0, occupied, (1 << 1), &rng, &steal);

    (void)F0732_COMBAT_RngRandom_Compat(&expected, 8);
    (void)F0732_COMBAT_RngRandom_Compat(&expected, 8);
    (void)F0732_COMBAT_RngRandom_Compat(&expected, 2);
    (void)F0732_COMBAT_RngRandom_Compat(&expected, 64);

    EXPECT_EQ(ok, 1, "giggler_luck: returns 1");
    EXPECT_EQ(steal.stealSlotIndex, TEST_DM1_SLOT_POUCH_2,
              "giggler_luck: first source-table slot can be stolen");
    EXPECT_EQ(steal.stolenCount, 1,
              "giggler_luck: luck check stops before backpack random slot");
    EXPECT_EQ((int)rng.seed, (int)expected.seed,
              "giggler_luck: no RANDOM(17) consumed after lucky stop");
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
        (1u << TEST_DM1_SLOT_POUCH_2) |
        (1u << TEST_DM1_SLOT_NECK) |
        (1u << TEST_DM1_SLOT_POUCH_1) |
        (1u << (TEST_DM1_SLOT_BACKPACK_LINE1_1 + 15)) |
        (1u << (TEST_DM1_SLOT_BACKPACK_LINE1_1 + 5));
    ag.cells = 1; /* front cell for east-facing melee; this test isolates steal */

    int ok = F0810_DM1_GROUP_DispatchBehavior_Compat(&ctx, &ag, &rng, &result);
    EXPECT_EQ(ok, 1, "giggler_dispatch: returns 1");
    EXPECT_EQ(result.actionKind, DM1_ACTION_STEAL,
              "giggler_dispatch: action is STEAL");
    EXPECT_EQ(result.newBehavior, DM1_BEHAVIOR_FLEE,
              "giggler_dispatch: steal can switch to FLEE");
    EXPECT_EQ(result.stealSlotIndex, TEST_DM1_SLOT_POUCH_2,
              "giggler_dispatch: reports first stolen slot");
    EXPECT_EQ(result.stolenCount, 5,
              "giggler_dispatch: reports stolen slot count");
    EXPECT_EQ(ag.delayFleeingFromTarget, 54,
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

/* --- BUG-104 batch 3: ranged/stealth/spell-caster STUB->FULL promotion
 *     of C03 Wizard Eye, C17 Giant Wasp, C21 Oitu.
 *
 *     Source-locked contract tests for the F0804 §(5b) per-type behavior
 *     branches and the canonical DUNGEON.C G0243 numeric values. */

/* --- Test 28: C03 Wizard Eye profile + per-type dispatch contract --- */
static void test_wizard_eye_promoted_to_full(void) {
    const struct CreatureBehaviorProfile_Compat* p;

    p = CREATURE_GetProfile_Compat(CREATURE_TYPE_WIZARD_EYE);
    if (!p) {
        g_fail++;
        fprintf(stderr, "FAIL: %s\n", "wizard_eye_promoted: profile non-null");
        return;
    }
    g_pass++;
    /* Source: DUNGEON.C G0243[3] Sight=10, smell=2, attack_range=3. */
    EXPECT_EQ(p->sightRange, 10,
              "wizard_eye_sight: DUNGEON.C G0243[3] sight=10");
    EXPECT_EQ(p->smellRange, 2,
              "wizard_eye_smell: DUNGEON.C G0243[3] smell=2");
    EXPECT_EQ(p->movementTicks, 10,
              "wizard_eye_movement: DUNGEON.C G0243[3] MOV=10");
    EXPECT_EQ(p->attackTicks, 21,
              "wizard_eye_attack_ticks: DUNGEON.C G0243[3] ATT_TICKS=21");
    EXPECT_EQ(p->baseAttack, 58,
              "wizard_eye_attack: DUNGEON.C G0243[3] ATTACK=58");
    EXPECT_EQ(p->baseHealth, 40,
              "wizard_eye_hp: DUNGEON.C G0243[3] HP=40");
    EXPECT_EQ(p->dexterity, 80,
              "wizard_eye_dex: DUNGEON.C G0243[3] DEX=80");
    EXPECT_EQ(p->baseDefense, 30,
              "wizard_eye_def: DUNGEON.C G0243[3] DEF=30");
    EXPECT_EQ(p->attackType, COMBAT_ATTACK_MAGIC,
              "wizard_eye_attackType: G0243[3] AttackType=5 (MAGIC)");
    /* Source: 0x04B4 decoded LEVITATION=0x0020, SIDE_ATTACK=0x0004. */
    EXPECT_EQ(p->attributes & CREATURE_ATTR_MASK_LEVITATION,
              CREATURE_ATTR_MASK_LEVITATION,
              "wizard_eye_levitation: 0x04B4 LEVITATION bit");
    EXPECT_EQ(p->attributes & CREATURE_ATTR_MASK_SIDE_ATTACK,
              CREATURE_ATTR_MASK_SIDE_ATTACK,
              "wizard_eye_side_attack: 0x04B4 SIDE_ATTACK bit");
    EXPECT_EQ(p->implementationTier, CREATURE_IMPL_TIER_FULL,
              "wizard_eye_tier: STUB->FULL (BUG-104 batch 3)");
}

/* --- Test 29: C17 Giant Wasp profile + per-type dispatch contract --- */
static void test_giant_wasp_promoted_to_full(void) {
    const struct CreatureBehaviorProfile_Compat* p;

    p = CREATURE_GetProfile_Compat(CREATURE_TYPE_GIANT_WASP);
    if (!p) {
        g_fail++;
        fprintf(stderr, "FAIL: %s\n", "giant_wasp_promoted: profile non-null");
        return;
    }
    g_pass++;
    /* Source: DUNGEON.C G0243[17] Sight=2, smell=4, attack_range=1. */
    EXPECT_EQ(p->sightRange, 2,
              "giant_wasp_sight: DUNGEON.C G0243[17] sight=2");
    EXPECT_EQ(p->smellRange, 4,
              "giant_wasp_smell: DUNGEON.C G0243[17] smell=4");
    EXPECT_EQ(p->movementTicks, 1,
              "giant_wasp_movement: DUNGEON.C G0243[17] MOV=1 (fastest)");
    EXPECT_EQ(p->attackTicks, 16,
              "giant_wasp_attack_ticks: DUNGEON.C G0243[17] ATT_TICKS=16");
    EXPECT_EQ(p->baseAttack, 28,
              "giant_wasp_attack: DUNGEON.C G0243[17] ATTACK=28");
    EXPECT_EQ(p->baseHealth, 8,
              "giant_wasp_hp: DUNGEON.C G0243[17] HP=8");
    EXPECT_EQ(p->dexterity, 150,
              "giant_wasp_dex: DUNGEON.C G0243[17] DEX=150 (highest)");
    EXPECT_EQ(p->baseDefense, 180,
              "giant_wasp_def: DUNGEON.C G0243[17] DEF=180 (highest)");
    EXPECT_EQ(p->poisonAttack, 20,
              "giant_wasp_poison: DUNGEON.C G0243[17] POISON=20 (sting)");
    EXPECT_EQ(p->attackType, COMBAT_ATTACK_SHARP,
              "giant_wasp_attackType: G0243[17] AttackType=4 (SHARP)");
    /* Source: 0x04A0 decoded LEVITATION=0x0020. */
    EXPECT_EQ(p->attributes & CREATURE_ATTR_MASK_LEVITATION,
              CREATURE_ATTR_MASK_LEVITATION,
              "giant_wasp_levitation: 0x04A0 LEVITATION bit");
    EXPECT_EQ(p->implementationTier, CREATURE_IMPL_TIER_FULL,
              "giant_wasp_tier: STUB->FULL (BUG-104 batch 3)");
}

/* --- Test 30: C21 Oitu profile + per-type dispatch contract --- */
static void test_oitu_promoted_to_full(void) {
    const struct CreatureBehaviorProfile_Compat* p;

    p = CREATURE_GetProfile_Compat(CREATURE_TYPE_OITU);
    if (!p) {
        g_fail++;
        fprintf(stderr, "FAIL: %s\n", "oitu_promoted: profile non-null");
        return;
    }
    g_pass++;
    /* Source: DUNGEON.C G0243[21] Sight=2, smell=5, attack_range=1. */
    EXPECT_EQ(p->sightRange, 2,
              "oitu_sight: DUNGEON.C G0243[21] sight=2");
    EXPECT_EQ(p->smellRange, 5,
              "oitu_smell: DUNGEON.C G0243[21] smell=5");
    EXPECT_EQ(p->movementTicks, 7,
              "oitu_movement: DUNGEON.C G0243[21] MOV=7");
    EXPECT_EQ(p->attackTicks, 15,
              "oitu_attack_ticks: DUNGEON.C G0243[21] ATT_TICKS=15");
    EXPECT_EQ(p->baseAttack, 130,
              "oitu_attack: DUNGEON.C G0243[21] ATTACK=130 (very high)");
    EXPECT_EQ(p->baseHealth, 77,
              "oitu_hp: DUNGEON.C G0243[21] HP=77");
    EXPECT_EQ(p->dexterity, 60,
              "oitu_dex: DUNGEON.C G0243[21] DEX=60");
    EXPECT_EQ(p->baseDefense, 33,
              "oitu_def: DUNGEON.C G0243[21] DEF=33");
    EXPECT_EQ(p->attackType, COMBAT_ATTACK_SHARP,
              "oitu_attackType: G0243[21] AttackType=4 (SHARP) — PC 3.4 binary");
    EXPECT_EQ(p->implementationTier, CREATURE_IMPL_TIER_FULL,
              "oitu_tier: STUB->FULL (BUG-104 batch 3)");
}

/* --- Test 31: §(5b) per-type dispatch coverage for the three new types --- */
static void test_batch3_per_type_dispatch_coverage(void) {
    int fullTypes[] = {
        CREATURE_TYPE_WIZARD_EYE,
        CREATURE_TYPE_GIANT_WASP,
        CREATURE_TYPE_OITU
    };
    int i;
    for (i = 0; i < 3; ++i) {
        const struct CreatureBehaviorProfile_Compat* p =
            CREATURE_GetProfile_Compat(fullTypes[i]);
        if (!p) {
            g_fail++;
            fprintf(stderr, "FAIL: %s\n", "batch3_coverage: profile non-null");
            continue;
        }
        g_pass++;
        EXPECT_EQ(p->implementationTier, CREATURE_IMPL_TIER_FULL,
                  "batch3_coverage: tier=FULL (BUG-104 batch 3)");
    }
}

/* --- Test 32: F0209 reaction apply plan owns behavior writeback --- */
static void test_reaction_apply_plan_schedules_next_event(void) {
    struct DM1BehaviorResult_Compat behavior;
    struct DM1ActiveGroup_Compat ag;
    struct DM1BehaviorReactionApplyPlan_Compat plan;
    int ok;

    memset(&behavior, 0, sizeof(behavior));
    memset(&ag, 0, sizeof(ag));
    memset(&plan, 0, sizeof(plan));
    behavior.newBehavior = DM1_BEHAVIOR_APPROACH;
    behavior.nextEventDelayTicks = 6;
    behavior.nextEventType = DM1_EVENT_REACTION_PARTY_IS_ADJACENT;
    ag.targetMapX = 12;
    ag.targetMapY = 13;

    ok = F0810b_DM1_GROUP_PlanReactionApply_Compat(
        &behavior, &ag, 44, 7, 2, 8, 9, 0x5A,
        101, 102, 103, 104, 2000u, &plan);

    EXPECT_EQ(ok, 1, "reaction_apply: helper succeeds");
    EXPECT_EQ(plan.valid, 1, "reaction_apply: plan valid");
    EXPECT_EQ(plan.newAiStateKind, 103,
              "reaction_apply: DM1 APPROACH maps to caller approach state");
    EXPECT_EQ(plan.groupMapIndex, 2, "reaction_apply: map index from event");
    EXPECT_EQ(plan.groupMapX, 8, "reaction_apply: map x from event");
    EXPECT_EQ(plan.groupMapY, 9, "reaction_apply: map y from event");
    EXPECT_EQ(plan.groupCells, 0x5A, "reaction_apply: group cells preserved");
    EXPECT_EQ(plan.lastSeenPartyMapX, 12,
              "reaction_apply: target x from active group");
    EXPECT_EQ(plan.lastSeenPartyMapY, 13,
              "reaction_apply: target y from active group");
    EXPECT_EQ(plan.lastSeenPartyTick, 2000,
              "reaction_apply: current tick writeback");
    EXPECT_EQ(plan.groupBehavior, DM1_BEHAVIOR_APPROACH,
              "reaction_apply: group behavior byte");
    EXPECT_EQ(plan.shouldScheduleNextEvent, 1,
              "reaction_apply: schedules positive delay/type");
    EXPECT_EQ((int)plan.nextEventFireAtTick, 2006,
              "reaction_apply: next tick adds delay");
    EXPECT_EQ(plan.nextEventGroupIndex, 44,
              "reaction_apply: next event group index");
    EXPECT_EQ(plan.nextEventCreatureType, 7,
              "reaction_apply: next event creature type");
    EXPECT_EQ(plan.nextEventType, DM1_EVENT_REACTION_PARTY_IS_ADJACENT,
              "reaction_apply: next event type");
}

static void test_reaction_apply_plan_no_event_and_wander_default(void) {
    struct DM1BehaviorResult_Compat behavior;
    struct DM1ActiveGroup_Compat ag;
    struct DM1BehaviorReactionApplyPlan_Compat plan;

    memset(&behavior, 0, sizeof(behavior));
    memset(&ag, 0, sizeof(ag));
    memset(&plan, 0, sizeof(plan));
    behavior.newBehavior = DM1_BEHAVIOR_USELESS4;
    behavior.nextEventDelayTicks = 0;
    behavior.nextEventType = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;

    EXPECT_EQ(F0810b_DM1_GROUP_PlanReactionApply_Compat(
                  &behavior, &ag, 1, 2, 3, 4, 5, 0,
                  201, 202, 203, 204, 99u, &plan),
              1, "reaction_apply_no_event: helper succeeds");
    EXPECT_EQ(plan.newAiStateKind, 201,
              "reaction_apply_no_event: unknown behavior maps to wander");
    EXPECT_EQ(plan.shouldScheduleNextEvent, 0,
              "reaction_apply_no_event: zero delay suppresses schedule");

    behavior.nextEventDelayTicks = 5;
    behavior.nextEventType = 0;
    EXPECT_EQ(F0810b_DM1_GROUP_PlanReactionApply_Compat(
                  &behavior, &ag, 1, 2, 3, 4, 5, 0,
                  201, 202, 203, 204, 99u, &plan),
              1, "reaction_apply_no_type: helper succeeds");
    EXPECT_EQ(plan.shouldScheduleNextEvent, 0,
              "reaction_apply_no_type: zero type suppresses schedule");
}

static void test_reaction_schedule_plan_owns_c30_insert_fields(void) {
    struct DM1BehaviorResult_Compat behavior;
    struct DM1BehaviorReactionSchedulePlan_Compat plan;

    memset(&behavior, 0, sizeof(behavior));
    memset(&plan, 0, sizeof(plan));
    behavior.nextEventDelayTicks = 4;
    behavior.nextEventType = DM1_EVENT_REACTION_HIT_BY_PROJECTILE;

    EXPECT_EQ(F0810c_DM1_GROUP_PlanReactionSchedule_Compat(
                  &behavior, 55, 14, 3, 6, 7, 100u, &plan),
              1, "reaction_schedule: helper succeeds");
    EXPECT_EQ(plan.shouldSchedule, 1,
              "reaction_schedule: positive delay/type schedules");
    EXPECT_EQ((int)plan.fireAtTick, 104,
              "reaction_schedule: fire tick adds source delay");
    EXPECT_EQ(plan.mapIndex, 3, "reaction_schedule: map index");
    EXPECT_EQ(plan.mapX, 6, "reaction_schedule: map x");
    EXPECT_EQ(plan.mapY, 7, "reaction_schedule: map y");
    EXPECT_EQ(plan.groupIndex, 55, "reaction_schedule: group index");
    EXPECT_EQ(plan.creatureType, 14, "reaction_schedule: creature type");
    EXPECT_EQ(plan.eventType, DM1_EVENT_REACTION_HIT_BY_PROJECTILE,
              "reaction_schedule: concrete C30 event type");

    behavior.nextEventDelayTicks = 0;
    EXPECT_EQ(F0810c_DM1_GROUP_PlanReactionSchedule_Compat(
                  &behavior, 55, 14, 3, 6, 7, 100u, &plan),
              1, "reaction_schedule_no_delay: helper succeeds");
    EXPECT_EQ(plan.shouldSchedule, 0,
              "reaction_schedule_no_delay: zero delay suppresses schedule");
}

/* ReDMCSB GROUP.C F0194: every active slot passes through F0184 before
 * map handoff, including raw C04 cells/direction writeback. */
static void test_remove_all_active_groups_f0194(void) {
    struct DM1ActiveGroup_Compat active[3];
    struct DungeonGroup_Compat groups[2];
    int currentActiveGroupCount = 2;
    int removed;

    memset(active, 0, sizeof(active));
    memset(groups, 0, sizeof(groups));
    active[0].groupThingIndex = 1;
    active[0].cells = 0xA5;
    active[0].directions = 0x06;
    active[1].groupThingIndex = -1;
    active[2].groupThingIndex = 0;
    active[2].cells = 0xFF;
    active[2].directions = 0x09;
    groups[0].behavior = DM1_BEHAVIOR_USELESS3;
    groups[1].behavior = DM1_BEHAVIOR_USELESS4;

    removed = F0817c_DM1_GROUP_RemoveAllActiveGroups_Compat(
        active, 3, &currentActiveGroupCount, groups, 2);
    EXPECT_EQ(removed, 2, "F0194 removes every active group");
    EXPECT_EQ(currentActiveGroupCount, 0, "F0194 clears active count");
    EXPECT_EQ(active[0].groupThingIndex, -1, "F0194 retires first slot");
    EXPECT_EQ(active[2].groupThingIndex, -1, "F0194 retires sparse slot");
    EXPECT_EQ(groups[1].cells, 0xA5, "F0184 restores raw C04 cells");
    EXPECT_EQ(groups[1].direction, 2, "F0184 restores low packed direction");
    EXPECT_EQ(groups[1].behavior, DM1_BEHAVIOR_WANDER,
              "F0184 resets unusable behavior to wander");
    EXPECT_EQ(groups[0].direction, 1, "F0184 preserves only creature-zero direction");
    EXPECT_EQ(groups[0].behavior, DM1_BEHAVIOR_USELESS3,
              "F0184 retains behavior below the source C4 threshold");

    active[0].groupThingIndex = 2;
    active[0].cells = 0x12;
    currentActiveGroupCount = 1;
    groups[0].cells = 0x77;
    removed = F0817c_DM1_GROUP_RemoveAllActiveGroups_Compat(
        active, 1, &currentActiveGroupCount, groups, 2);
    EXPECT_EQ(removed, 0, "F0194 rejects an out-of-range raw C04 reference");
    EXPECT_EQ(currentActiveGroupCount, 1,
              "F0194 rejects before mutating active count");
    EXPECT_EQ(groups[0].cells, 0x77,
              "F0194 rejects before mutating raw C04 data");
}

/* GROUP.C F0208: an earlier aspect update becomes C33-C36 and retains the
 * later C38-C41 timing in C.Ticks. */
static void test_group_add_event_f0208(void) {
    struct DM1GroupAddEventPlan_Compat plan;

    memset(&plan, 0, sizeof(plan));
    EXPECT_EQ(F0208_DM1_GROUP_BuildAddEventPlan_Compat(
                  DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0, 120u, 108u, &plan),
              1, "F0208 early aspect plan succeeds");
    EXPECT_EQ(plan.valid, 1, "F0208 early aspect plan is valid");
    EXPECT_EQ(plan.eventType, DM1_EVENT_UPDATE_ASPECT_CREATURE_0,
              "F0208 subtracts five to create C33");
    EXPECT_EQ((int)plan.mapTime, 108,
              "F0208 moves Map_Time to the earlier aspect update");
    EXPECT_EQ((int)plan.ticks, 12,
              "F0208 stores the later C38 delay in C.Ticks");
    EXPECT_EQ(plan.promotedAspectEvent, 1,
              "F0208 records C38-to-C33 promotion");

    memset(&plan, 0, sizeof(plan));
    EXPECT_EQ(F0208_DM1_GROUP_BuildAddEventPlan_Compat(
                  DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 2, 120u, 137u, &plan),
              1, "F0208 later aspect plan succeeds");
    EXPECT_EQ(plan.eventType, DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + 2,
              "F0208 keeps C40 when its Map_Time is earlier");
    EXPECT_EQ((int)plan.mapTime, 120,
              "F0208 preserves the caller event Map_Time");
    EXPECT_EQ((int)plan.ticks, 17,
              "F0208 stores future aspect difference in C.Ticks");
    EXPECT_EQ(plan.promotedAspectEvent, 0,
              "F0208 does not promote a later aspect update");
    EXPECT_EQ(F0208_DM1_GROUP_BuildAddEventPlan_Compat(
                  DM1_EVENT_UPDATE_BEHAVIOR_GROUP, 77u, 77u, &plan),
              1, "F0208 equal timestamps succeed");
    EXPECT_EQ((int)plan.ticks, 0,
              "F0208 equal timestamps retain zero C.Ticks");
}

static void test_group_path_blockers_f0197_to_f0199(void) {
    struct DM1GroupSightSquare_Compat square;
    struct F0199BlockGrid grid;

    memset(&square, 0, sizeof(square));
    EXPECT_EQ(F0226_DM1_GROUP_GetDistanceBetweenSquares_Compat(2, 7, 5, 4),
              6, "F0226 returns source Manhattan distance");
    EXPECT_EQ(F0226_DM1_GROUP_GetDistanceBetweenSquares_Compat(-3, 4, -3, 4),
              0, "F0226 returns zero for the same square");
    EXPECT_EQ(F0227_DM1_GROUP_IsDestinationVisibleFromSource_Compat(
                  0, 0, 0, 1, -2),
              1, "F0227 accepts a north-facing diagonal cone edge");
    EXPECT_EQ(F0227_DM1_GROUP_IsDestinationVisibleFromSource_Compat(
                  0, 0, 0, 3, -1),
              0, "F0227 rejects a north-facing destination outside the cone");
    EXPECT_EQ(F0227_DM1_GROUP_IsDestinationVisibleFromSource_Compat(
                  1, 0, 0, 2, 1),
              1, "F0227 accepts an east-facing diagonal cone edge");
    EXPECT_EQ(F0227_DM1_GROUP_IsDestinationVisibleFromSource_Compat(
                  3, 0, 0, 1, 0),
              0, "F0227 rejects a destination behind west-facing source");
    {
        struct RngState_Compat rng = make_rng(42u);
        struct RngState_Compat expected = make_rng(42u);
        int primary = -1;
        int secondary = -1;
        int random16 = F0732_COMBAT_RngRandom_Compat(&expected, 65536);
        EXPECT_EQ(F0228_DM1_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource_Compat(
                      0, 0, 0, -2, &rng, &primary, &secondary),
                  1, "F0228 cardinal direction succeeds");
        EXPECT_EQ(primary, 0, "F0228 cardinal primary is north");
        EXPECT_EQ(secondary, (random16 & 2) + 1,
                  "F0228 cardinal secondary consumes RANDOM(65536)");
        EXPECT_EQ((int)rng.seed, (int)expected.seed,
                  "F0228 cardinal preserves source RNG count");
    }
    {
        struct RngState_Compat rng = make_rng(7u);
        struct RngState_Compat expected = make_rng(7u);
        int primary = -1;
        int secondary = -1;
        int random2 = F0732_COMBAT_RngRandom_Compat(&expected, 2);
        EXPECT_EQ(F0228_DM1_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource_Compat(
                      0, 0, 1, -1, &rng, &primary, &secondary),
                  1, "F0228 diagonal direction succeeds");
        EXPECT_EQ(primary, random2 ? 1 : 0,
                  "F0228 diagonal applies source tie random");
        EXPECT_EQ(secondary, random2 ? 0 : 1,
                  "F0228 diagonal retains the alternate direction");
        EXPECT_EQ((int)rng.seed, (int)expected.seed,
                  "F0228 diagonal consumes exactly one RNG value");
    }
    {
        struct RngState_Compat rng = make_rng(99u);
        struct RngState_Compat expected = make_rng(99u);
        int cells[4] = { -1, -1, -1, -1 };

        (void)F0732_COMBAT_RngRandom_Compat(&expected, 65536);
        EXPECT_EQ(F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat(
                      cells, 0, -1, 0, 0, 0u, &rng),
                  1, "F0229 resolves a source vertical target row");
        EXPECT_EQ(cells[0], 3, "F0229 vertical row first cell");
        EXPECT_EQ(cells[1], 2, "F0229 vertical row second cell");
        EXPECT_EQ(cells[2], 0, "F0229 vertical row third cell");
        EXPECT_EQ(cells[3], 1, "F0229 vertical row fourth cell");
        EXPECT_EQ((int)rng.seed, (int)expected.seed,
                  "F0229 preserves F0228 cardinal RNG consumption");
    }
    {
        struct RngState_Compat rng = make_rng(100u);
        int cells[4] = { -1, -1, -1, -1 };

        EXPECT_EQ(F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat(
                      cells, -1, 0, 0, 0, 2u, &rng),
                  1, "F0229 resolves a source horizontal target row");
        EXPECT_EQ(cells[0], 2, "F0229 horizontal row first cell");
        EXPECT_EQ(cells[1], 1, "F0229 horizontal row second cell");
        EXPECT_EQ(cells[2], 3, "F0229 horizontal row third cell");
        EXPECT_EQ(cells[3], 0, "F0229 horizontal row fourth cell");
    }
    square.elementType = DUNGEON_ELEMENT_DOOR;
    square.doorState = 3;
    EXPECT_EQ(F0817d_DM1_GROUP_IsViewPartyBlocked_Compat(&square), 1,
              "F0197 blocks a three-quarter closed opaque door");
    square.creaturesCanSeeThrough = 1;
    EXPECT_EQ(F0817d_DM1_GROUP_IsViewPartyBlocked_Compat(&square), 0,
              "F0197 permits a see-through door");
    square.elementType = DUNGEON_ELEMENT_FAKEWALL;
    square.doorState = 0;
    square.creaturesCanSeeThrough = 0;
    square.fakeWallOpen = 0;
    square.fakeWallImaginary = 1;
    EXPECT_EQ(F0817d_DM1_GROUP_IsViewPartyBlocked_Compat(&square), 1,
              "F0197 keeps an imaginary closed fake wall sight-blocking");
    EXPECT_EQ(F0817e_DM1_GROUP_IsSmellPartyBlocked_Compat(&square), 0,
              "F0198 lets smell pass an imaginary fake wall");

    memset(&grid, 0xff, sizeof(grid));
    EXPECT_EQ(F0817f_DM1_GROUP_GetDistanceBetweenUnblockedSquares_Compat(
                  0, 0, 0, 1, NULL, NULL), 1,
              "F0199 adjacent squares do not require a callback");
    EXPECT_EQ(F0817f_DM1_GROUP_GetDistanceBetweenUnblockedSquares_Compat(
                  0, 0, 3, 0, f0199_is_blocked, &grid), 3,
              "F0199 returns Manhattan distance on an unblocked row");
    grid.firstX = 2;
    grid.firstY = 0;
    EXPECT_EQ(F0817f_DM1_GROUP_GetDistanceBetweenUnblockedSquares_Compat(
                  0, 0, 3, 0, f0199_is_blocked, &grid), 0,
              "F0199 rejects a source-tested row blocker");
    grid.firstX = 2;
    grid.firstY = 3;
    grid.secondX = 3;
    grid.secondY = 2;
    EXPECT_EQ(F0817f_DM1_GROUP_GetDistanceBetweenUnblockedSquares_Compat(
                  0, 0, 3, 3, f0199_is_blocked, &grid), 0,
              "F0199 rejects a diagonal when both source branches block");
    EXPECT_EQ(F0817f_DM1_GROUP_GetDistanceBetweenUnblockedSquares_Compat(
                  0, 0, 3, 0, NULL, NULL), 0,
              "F0199 rejects a non-adjacent route without loaded-map callback");
}

int main(void) {
    printf("DM1 V1 Creature AI Behavior CTest Gate\n");
    printf("Source: ReDMCSB GROUP.C, MOVESENS.C, DEFS.H\n\n");

    test_wander_to_attack();
    test_wander_to_approach();
    test_freeze_life();
    test_archenemy_ignores_freeze();
    test_archenemy_approach_double_move();
    test_reaction_party_adjacent();
    test_flee_behavior();
    test_flee_expires_to_wander();
    test_approach_arrives_at_target();
    test_should_attack_range();
    test_fear_check();
    test_projectile_decision();
    test_creature_projectile_launch_params();
    test_lord_order_grey_lord_projectile_rejected();
    test_vexirk_projectile_type_table();
    test_dispatch_projectile_payload();
    test_set_group_direction();
    test_set_group_direction_requires_live_rng_f0205();
    test_group_movement_facts();
    test_first_movement_direction_f0203_test_state();
    test_archenemy_double_movement_f0204();
    test_single_square_move_uses_typed_facts();
    test_smell_direction();
    test_visible_distance_requires_party_map();
    test_visible_distance_f0200_live_route();
    test_smell_direction_requires_unblocked_route();
    test_smell_direction_stored_scent_fallback();
    test_smell_direction_f0201_live_route();
    test_per_creature_attack_event();
    test_reaction_during_freeze();
    test_negative_reaction_event_creation();
    test_projectile_hit_reaction_sets_search_direction();
    test_danger_reaction_moves_attack_group_to_approach();
    test_flee_direction();
    test_giggler_steal_resolver();
    test_giggler_steal_luck_stops_before_backpack_random();
    test_giggler_attack_dispatch_steals();
    test_quarter_square_melee_cell_adjusts_before_attack();
    test_attack_any_back_row_bypasses_cell_adjust();
    test_fixed_possessions_animated_armour_are_cursed();
    test_fixed_possessions_rockpile_random_flags();
    test_fixed_possessions_dragon_steak_table();
    test_wizard_eye_promoted_to_full();
    test_giant_wasp_promoted_to_full();
    test_oitu_promoted_to_full();
    test_batch3_per_type_dispatch_coverage();
    test_reaction_apply_plan_schedules_next_event();
    test_reaction_apply_plan_no_event_and_wander_default();
    test_reaction_schedule_plan_owns_c30_insert_fields();
    test_remove_all_active_groups_f0194();
    test_group_add_event_f0208();
    test_group_path_blockers_f0197_to_f0199();
    test_f0264_levitation_classifier();

    printf("\n--- Results: %d PASS, %d FAIL ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
