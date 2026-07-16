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
static unsigned short g_test_first_thing = THING_ENDOFLIST;
static int g_test_fluxcages[4];

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

static unsigned short make_test_thing(unsigned type, unsigned index)
{
    return (unsigned short)(((type & 0x0fu) << 10) | (index & 0x03ffu));
}

unsigned short F0511_DUNGEON_GetSquareFirstThing_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY)
{
    (void)dungeon; (void)things; (void)mapIndex; (void)mapX; (void)mapY;
    return g_test_first_thing;
}

unsigned short F0512_DUNGEON_GetThingNext_Compat(
    const struct DungeonThings_Compat* things, unsigned short thing)
{
    int index;
    if (!things || THING_GET_TYPE(thing) != THING_TYPE_GROUP) return THING_ENDOFLIST;
    index = (int)THING_GET_INDEX(thing);
    if (index < 0 || index >= things->groupCount || !things->groups) return THING_ENDOFLIST;
    return things->groups[index].next;
}

int F0221_GROUP_IsFluxcageOnSquare_Compat(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    int mapIndex, int mapX, int mapY, int* outIsFluxcage)
{
    int slot;
    (void)dungeon; (void)things; (void)mapIndex; (void)mapY;
    slot = mapX < 0 ? 0 : (mapX > 0 ? 1 : 2);
    if (outIsFluxcage) *outIsFluxcage = g_test_fluxcages[slot & 3];
    return 1;
}

int F0514_DUNGEON_LinkThingToList_Compat(
    struct DungeonDatState_Compat* dungeon, struct DungeonThings_Compat* things,
    unsigned short thingToLink, unsigned short thingInList,
    int mapIndex, int mapX, int mapY)
{
    (void)dungeon; (void)things; (void)thingToLink; (void)thingInList;
    (void)mapIndex; (void)mapX; (void)mapY;
    return 1;
}

int F0515_DUNGEON_UnlinkThingFromList_Compat(
    struct DungeonDatState_Compat* dungeon, struct DungeonThings_Compat* things,
    unsigned short thingToUnlink, unsigned short thingInList,
    int mapIndex, int mapX, int mapY)
{
    (void)dungeon; (void)things; (void)thingToUnlink; (void)thingInList;
    (void)mapIndex; (void)mapX; (void)mapY;
    return 1;
}

int F0732_COMBAT_RngRandom_Compat(struct RngState_Compat* rng, int modulus)
{
    (void)rng;
    return modulus > 0 ? 0 : 0;
}

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

static void test_f0222_raw_lord_chaos_thing(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[3];
    unsigned char squares[6];
    unsigned short firstThing = make_test_thing(THING_TYPE_GROUP, 0);
    unsigned short base = 0, found = 0;
    int allowed = 0;
    memset(&dungeon, 0, sizeof(dungeon)); memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles)); memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups)); memset(squares, 0, sizeof(squares));
    printf("  F0222 raw Lord Chaos Thing...\n");
    squares[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    squares[1] = (unsigned char)(DUNGEON_ELEMENT_TELEPORTER << 5);
    squares[2] = (unsigned char)(DUNGEON_ELEMENT_PIT << 5);
    squares[3] = (unsigned char)(DUNGEON_ELEMENT_DOOR << 5);
    squares[4] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    squares[5] = (unsigned char)(DUNGEON_ELEMENT_STAIRS << 5);
    map.width = 2; map.height = 3; tiles.squareData = squares; tiles.squareCount = 6;
    dungeon.header.mapCount = 1; dungeon.maps = &map; dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1; dungeon.columnsCumulativeSquareFirstThingCount = &base;
    dungeon.dungeonColumnCount = 1;
    things.loaded = 1; things.squareFirstThings = &firstThing;
    things.squareFirstThingCount = 1; things.groups = groups; things.groupCount = 3;
    groups[0].next = make_test_thing(THING_TYPE_GROUP, 1);
    groups[0].creatureType = 3;
    groups[1].next = make_test_thing(THING_TYPE_GROUP, 2);
    groups[1].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    groups[2].next = THING_ENDOFLIST;
    groups[2].creatureType = 7;
    g_test_first_thing = firstThing;
    ASSERT_EQ(DM1_Endgame_F0222_GetLordChaosThingPc34Compat(
                  &dungeon, &things, 0, 0, 0, &found), 1, "F0222 query succeeds");
    ASSERT_EQ(found, make_test_thing(THING_TYPE_GROUP, 1),
              "F0222 returns non-leading raw Lord Chaos Thing");
    groups[1].creatureType = DM1_CREATURE_LORD_ORDER_ID;
    found = THING_NONE;
    ASSERT_EQ(DM1_Endgame_F0222_GetLordChaosThingPc34Compat(
                  &dungeon, &things, 0, 0, 0, &found), 1,
              "F0222 absence scan succeeds");
    ASSERT_EQ(found, 0, "F0222 absence does not synthesize a Lord Chaos Thing");
    groups[1].creatureType = DM1_CREATURE_LORD_CHAOS_ID;
    ASSERT_EQ(DM1_Endgame_F0223_IsLordChaosAllowedPc34Compat(
                  &dungeon, 0, 0, 0, &allowed), 1, "F0223 corridor query succeeds");
    ASSERT_EQ(allowed, 1, "F0223 accepts corridors");
    ASSERT_EQ(DM1_Endgame_F0223_IsLordChaosAllowedPc34Compat(
                  &dungeon, 0, 0, 1, &allowed), 1, "F0223 teleporter query succeeds");
    ASSERT_EQ(allowed, 1, "F0223 accepts teleporters");
    ASSERT_EQ(DM1_Endgame_F0223_IsLordChaosAllowedPc34Compat(
                  &dungeon, 0, 0, 2, &allowed), 1, "F0223 pit query succeeds");
    ASSERT_EQ(allowed, 1, "F0223 accepts pits");
    ASSERT_EQ(DM1_Endgame_F0223_IsLordChaosAllowedPc34Compat(
                  &dungeon, 0, 1, 0, &allowed), 1, "F0223 query succeeds");
    ASSERT_EQ(allowed, 1, "F0223 accepts doors");
    ASSERT_EQ(DM1_Endgame_F0223_IsLordChaosAllowedPc34Compat(
                  &dungeon, 0, 1, 1, &allowed), 1, "F0223 wall query succeeds");
    ASSERT_EQ(allowed, 0, "F0223 rejects walls");
    ASSERT_EQ(DM1_Endgame_F0223_IsLordChaosAllowedPc34Compat(
                  &dungeon, 0, 1, 2, &allowed), 1, "F0223 stairs query succeeds");
    ASSERT_EQ(allowed, 0, "F0223 rejects stairs");
}

static void test_f0224_fluxcage_plan_source_event_fields(void)
{
    DM1EndgameF0224FluxcageActionInput input;
    DM1EndgameF0224FluxcageActionPlan plan;
    memset(&input, 0, sizeof(input));
    printf("  F0224 fluxcage action plan fields...\n");
    input.mapIndex = 2;
    input.mapX = 5;
    input.mapY = 6;
    input.squareType = DUNGEON_ELEMENT_CORRIDOR;
    input.gameTime = 1234;
    input.hasUnusedExplosionThing = 1;
    input.explosionThing = make_test_thing(THING_TYPE_EXPLOSION, 17);
    input.lordChaosAdjacent[DM1_ENDGAME_F0224_ADJACENT_NORTH] = 1;
    input.fluxcagesAroundAdjacentLordChaos
        [DM1_ENDGAME_F0224_ADJACENT_NORTH]
        [DM1_ENDGAME_F0224_ADJACENT_WEST] = 1;
    input.fluxcagesAroundAdjacentLordChaos
        [DM1_ENDGAME_F0224_ADJACENT_NORTH]
        [DM1_ENDGAME_F0224_ADJACENT_EAST] = 1;

    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 plan builds");
    ASSERT_EQ(plan.valid, 1, "F0224 plan valid");
    ASSERT_EQ(plan.createdFluxcage, 1, "F0224 creates fluxcage");
    ASSERT_EQ(plan.linkedExplosionThing, 1, "F0224 links C15 explosion Thing");
    ASSERT_EQ(plan.explosionThing, input.explosionThing,
              "F0224 preserves allocated explosion Thing");
    ASSERT_EQ(plan.explosionType, DM1_EXPLOSION_FLUXCAGE_TYPE,
              "F0224 writes C050 fluxcage type");
    ASSERT_EQ(plan.removeEventType, DM1_EVENT_REMOVE_FLUXCAGE,
              "F0224 schedules C24 remove-fluxcage event");
    ASSERT_EQ(plan.removeEventPriority, 0, "F0224 C24 priority is zero");
    ASSERT_EQ((int)plan.removeEventGameTime, 1334,
              "F0224 C24 time is GameTime + 100");
    ASSERT_EQ(plan.removeEventMapIndex, 2, "F0224 C24 map is current map");
    ASSERT_EQ(plan.removeEventMapX, 5, "F0224 C24 stores target x");
    ASSERT_EQ(plan.removeEventMapY, 6, "F0224 C24 stores target y");
    ASSERT_EQ(plan.removeEventSlotThing, input.explosionThing,
              "F0224 C24 C.Slot stores the raw C15 Thing");
    ASSERT_EQ(plan.checkedLordChaosAdjacentIndex,
              DM1_ENDGAME_F0224_ADJACENT_NORTH,
              "F0224 checks north Lord Chaos first");
    ASSERT_EQ(plan.lordChaosMapX, 5, "F0224 north Lord Chaos x");
    ASSERT_EQ(plan.lordChaosMapY, 5, "F0224 north Lord Chaos y");
    ASSERT_EQ(plan.otherFluxcageCount, 2,
              "F0224 counts two other fluxcage squares");
    ASSERT_EQ(plan.scheduledDangerReaction, 1,
              "F0224 schedules danger reaction at count two");
    ASSERT_EQ(plan.reactionEventType, DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE,
              "F0224 danger reaction is C29");
}

static void test_f0224_fluxcage_plan_noop_gates(void)
{
    DM1EndgameF0224FluxcageActionInput input;
    DM1EndgameF0224FluxcageActionPlan plan;
    memset(&input, 0, sizeof(input));
    printf("  F0224 fluxcage action no-op gates...\n");
    input.mapX = 1;
    input.mapY = 2;
    input.squareType = DUNGEON_ELEMENT_WALL;
    input.hasUnusedExplosionThing = 1;
    input.explosionThing = make_test_thing(THING_TYPE_EXPLOSION, 3);
    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 wall plan builds");
    ASSERT_EQ(plan.blockedWallOrStairs, 1, "F0224 wall target returns early");
    ASSERT_EQ(plan.createdFluxcage, 0, "F0224 wall target creates no cage");
    ASSERT_EQ(plan.removeEventType, 0, "F0224 wall target schedules no C24");

    input.squareType = DUNGEON_ELEMENT_STAIRS;
    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 stairs plan builds");
    ASSERT_EQ(plan.blockedWallOrStairs, 1, "F0224 stairs target returns early");
    ASSERT_EQ(plan.createdFluxcage, 0, "F0224 stairs target creates no cage");

    input.squareType = DUNGEON_ELEMENT_CORRIDOR;
    input.hasUnusedExplosionThing = 0;
    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 no-slot plan builds");
    ASSERT_EQ(plan.blockedNoUnusedExplosionThing, 1,
              "F0224 no unused C15 Thing returns early");
    ASSERT_EQ(plan.createdFluxcage, 0, "F0224 no-slot creates no cage");

    input.hasUnusedExplosionThing = 1;
    input.explosionThing = make_test_thing(THING_TYPE_GROUP, 0);
    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 wrong-thing plan builds");
    ASSERT_EQ(plan.blockedNoUnusedExplosionThing, 1,
              "F0224 requires an unused C15 explosion Thing");
}

static void test_f0224_fluxcage_plan_adjacent_order(void)
{
    DM1EndgameF0224FluxcageActionInput input;
    DM1EndgameF0224FluxcageActionPlan plan;
    memset(&input, 0, sizeof(input));
    printf("  F0224 fluxcage adjacent Lord Chaos order...\n");
    input.mapX = 10;
    input.mapY = 10;
    input.squareType = DUNGEON_ELEMENT_CORRIDOR;
    input.hasUnusedExplosionThing = 1;
    input.explosionThing = make_test_thing(THING_TYPE_EXPLOSION, 9);
    input.lordChaosAdjacent[DM1_ENDGAME_F0224_ADJACENT_WEST] = 1;
    input.lordChaosAdjacent[DM1_ENDGAME_F0224_ADJACENT_EAST] = 1;
    input.fluxcagesAroundAdjacentLordChaos
        [DM1_ENDGAME_F0224_ADJACENT_WEST]
        [DM1_ENDGAME_F0224_ADJACENT_NORTH] = 1;
    input.fluxcagesAroundAdjacentLordChaos
        [DM1_ENDGAME_F0224_ADJACENT_WEST]
        [DM1_ENDGAME_F0224_ADJACENT_SOUTH] = 1;
    input.fluxcagesAroundAdjacentLordChaos
        [DM1_ENDGAME_F0224_ADJACENT_EAST]
        [DM1_ENDGAME_F0224_ADJACENT_NORTH] = 1;

    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 adjacent-order plan builds");
    ASSERT_EQ(plan.checkedLordChaosAdjacentIndex,
              DM1_ENDGAME_F0224_ADJACENT_WEST,
              "F0224 checks west before east");
    ASSERT_EQ(plan.lordChaosMapX, 9, "F0224 west Lord Chaos x");
    ASSERT_EQ(plan.lordChaosMapY, 10, "F0224 west Lord Chaos y");
    ASSERT_EQ(plan.otherFluxcageCount, 2,
              "F0224 uses first adjacent Lord Chaos fluxcage count");
    ASSERT_EQ(plan.scheduledDangerReaction, 1,
              "F0224 first adjacent Lord Chaos schedules C29");

    memset(input.lordChaosAdjacent, 0, sizeof(input.lordChaosAdjacent));
    memset(input.fluxcagesAroundAdjacentLordChaos, 0,
           sizeof(input.fluxcagesAroundAdjacentLordChaos));
    input.lordChaosAdjacent[DM1_ENDGAME_F0224_ADJACENT_EAST] = 1;
    input.fluxcagesAroundAdjacentLordChaos
        [DM1_ENDGAME_F0224_ADJACENT_EAST]
        [DM1_ENDGAME_F0224_ADJACENT_NORTH] = 1;
    ASSERT_EQ(DM1_Endgame_F0224_BuildFluxcageActionPlanPc34Compat(
                  &input, &plan), 1, "F0224 east-only plan builds");
    ASSERT_EQ(plan.checkedLordChaosAdjacentIndex,
              DM1_ENDGAME_F0224_ADJACENT_EAST,
              "F0224 reaches east after north/west miss");
    ASSERT_EQ(plan.lordChaosMapX, 11, "F0224 east Lord Chaos x");
    ASSERT_EQ(plan.lordChaosMapY, 10, "F0224 east Lord Chaos y");
    ASSERT_EQ(plan.otherFluxcageCount, 1,
              "F0224 records non-triggering fluxcage count");
    ASSERT_EQ(plan.scheduledDangerReaction, 0,
              "F0224 count other than two schedules no C29");
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

static void test_fuse_mutation_plan(void)
{
    DM1EndgameFuseMutationPlan plan;
    int i;
    int fireballs = 0;
    int harms = 0;
    int buzzes = 0;
    int cycles = 0;
    printf("  F0446 mutation/replay plan...\n");
    ASSERT_EQ(DM1_Endgame_BuildFuseMutationPlan(&plan), 1, "plan builds");
    ASSERT_EQ(plan.stepCount, 43, "F0445 cadence count");
    ASSERT_EQ(plan.totalF0445Updates, 43, "total F0445 updates");
    ASSERT_EQ(plan.steps[0].replayType, DM1_ENDGAME_F0445_EVENT_SETUP,
              "first step is setup");
    ASSERT_EQ(plan.steps[plan.stepCount - 1].deleteOtherGroups, 1,
              "last step deletes other groups");
    for (i = 0; i < plan.stepCount; ++i) {
        fireballs += plan.steps[i].spawnFireball;
        harms += plan.steps[i].spawnHarmNonMaterial;
        buzzes += plan.steps[i].requestBuzz;
        cycles += plan.steps[i].replayType == DM1_ENDGAME_F0445_EVENT_CHAOS_ORDER_SWITCH;
    }
    ASSERT_EQ(fireballs, 7, "six barrage fireballs plus final fireball");
    ASSERT_EQ(harms, 7, "six barrage harms plus final harm");
    ASSERT_EQ(buzzes, 13, "Lord Order plus twelve cycle buzzes");
    ASSERT_EQ(cycles, 24, "source cycle has twenty-four F0445 updates");
    ASSERT_EQ(plan.finalDelayTicks, 600, "F0446 final delay");
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
    test_f0222_raw_lord_chaos_thing();
    test_f0224_fluxcage_plan_source_event_fields();
    test_f0224_fluxcage_plan_noop_gates();
    test_f0224_fluxcage_plan_adjacent_order();
    test_fuse_action_no_lord_chaos();
    test_fuse_action_chaos_escapes();
    test_fuse_action_chaos_trapped();
    test_fuse_action_out_of_bounds();
    test_cycle_creature_type();
    test_fuse_mutation_plan();
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
