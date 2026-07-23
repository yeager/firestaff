#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_c14_layout_pc34_compat.h"
#include "dm1_v1_c15_layout_pc34_compat.h"

static int expect(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

/* M10's F0202 admission walks the authenticated C04 bytes, not just the
 * decoded test mirror. Keep local fixtures source-shaped. */
static void authenticate_group_c04(struct DungeonThings_Compat* things,
                                   const struct DungeonGroup_Compat* group,
                                   unsigned char rawGroup[16])
{
    unsigned short packed;

    memset(rawGroup, 0, 16);
    packed = (unsigned short)((group->behavior & 0x0fu) |
                              ((group->count & 0x03u) << 5) |
                              ((group->direction & 0x03u) << 8) |
                              ((group->doNotDiscard & 0x01u) << 10));
    rawGroup[0] = (unsigned char)(group->next & 0xffu);
    rawGroup[1] = (unsigned char)(group->next >> 8);
    rawGroup[2] = (unsigned char)(group->slot & 0xffu);
    rawGroup[3] = (unsigned char)(group->slot >> 8);
    rawGroup[4] = group->creatureType;
    rawGroup[5] = group->cells;
    rawGroup[6] = (unsigned char)(group->health[0] & 0xffu);
    rawGroup[7] = (unsigned char)(group->health[0] >> 8);
    rawGroup[14] = (unsigned char)(packed & 0xffu);
    rawGroup[15] = (unsigned char)(packed >> 8);
    things->rawThingData[THING_TYPE_GROUP] = rawGroup;
}

static int build_world(struct GameWorld_Compat* world)
{
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    int i;

    memset(world, 0, sizeof(*world));
    if (!F0881_WORLD_InitDefault_Compat(world, 0xF0206u)) return 0;
    dungeon = (struct DungeonDatState_Compat*)calloc(1, sizeof(*dungeon));
    things = (struct DungeonThings_Compat*)calloc(1, sizeof(*things));
    if (!dungeon || !things) goto fail;

    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->header.mapCount = 1;
    dungeon->dungeonColumnCount = 3;
    dungeon->columnsCumulativeSquareFirstThingCount =
        (unsigned short*)calloc(3, sizeof(unsigned short));
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(*dungeon->tiles));
    if (!dungeon->maps || !dungeon->tiles ||
        !dungeon->columnsCumulativeSquareFirstThingCount) goto fail;
    dungeon->maps[0].width = 3;
    dungeon->maps[0].height = 3;
    dungeon->maps[0].creatureTypeCount = 1;
    dungeon->maps[0].allowedCreatureTypes[0] = 0;
    dungeon->tiles[0].squareCount = 9;
    dungeon->tiles[0].squareData = (unsigned char*)calloc(9, 1);
    if (!dungeon->tiles[0].squareData) goto fail;
    for (i = 0; i < 9; ++i) {
        dungeon->tiles[0].squareData[i] =
            (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    }
    dungeon->tiles[0].squareData[4] |= DUNGEON_SQUARE_MASK_THING_LIST;
    dungeon->columnsCumulativeSquareFirstThingCount[0] = 0;
    dungeon->columnsCumulativeSquareFirstThingCount[1] = 0;
    dungeon->columnsCumulativeSquareFirstThingCount[2] = 1;

    things->loaded = 1;
    things->squareFirstThingCount = 1;
    things->squareFirstThings = (unsigned short*)calloc(1, sizeof(unsigned short));
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(*things->groups));
    things->rawThingData[THING_TYPE_GROUP] = (unsigned char*)calloc(16, 1);
    if (!things->squareFirstThings || !things->groups ||
        !things->rawThingData[THING_TYPE_GROUP]) goto fail;
    things->squareFirstThings[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    things->groups[0].next = THING_ENDOFLIST;
    things->groups[0].creatureType = 0;
    things->groups[0].count = 1;
    things->groups[0].cells = 0x04; /* creatures in cells 0 and 1 */
    things->groups[0].direction = 0;
    things->groups[0].behavior = DM1_BEHAVIOR_WANDER;
    things->groups[0].health[0] = 100;
    things->groups[0].health[1] = 100;
    authenticate_group_c04(things, &things->groups[0],
                           things->rawThingData[THING_TYPE_GROUP]);

    world->dungeon = dungeon;
    world->things = things;
    world->ownsDungeon = 1;
    world->party.mapIndex = 0;
    world->partyMapIndex = 0;
    world->newPartyMapIndex = -1;
    world->party.mapX = 1;
    world->party.mapY = 2;
    world->party.championCount = 1;
    world->party.champions[0].present = 1;
    world->party.champions[0].hp.current = 100;
    world->party.champions[0].hp.maximum = 100;
    world->party.champions[0].cell = 0;
    world->creatureAICount = 1;
    world->creatureAI[0].reserved0 = 0;
    world->creatureAI[0].stateKind = AI_STATE_WANDER;
    world->creatureAI[0].creatureType = 0;
    world->creatureAI[0].groupMapIndex = 0;
    world->creatureAI[0].groupMapX = 1;
    world->creatureAI[0].groupMapY = 1;
    world->creatureAI[0].groupCells = 0x04;
    return 1;

fail:
    if (dungeon) {
        if (dungeon->tiles) free(dungeon->tiles[0].squareData);
        free(dungeon->columnsCumulativeSquareFirstThingCount);
        free(dungeon->maps);
        free(dungeon->tiles);
    }
    if (things) {
        free(things->squareFirstThings);
        free(things->rawThingData[THING_TYPE_GROUP]);
        free(things->groups);
    }
    free(dungeon);
    free(things);
    return 0;
}

static int test_f0206_rng_direction_adapter(void)
{
    struct DM1ActiveGroup_Compat activeGroup;
    struct DM1ActiveGroup_Compat expectedGroup;
    struct RngState_Compat rng;
    struct RngState_Compat expectedRng;
    int ok = 1;

    memset(&activeGroup, 0, sizeof(activeGroup));
    memset(&expectedGroup, 0, sizeof(expectedGroup));
    F0730_COMBAT_RngInit_Compat(&rng, 0x1234u);
    F0730_COMBAT_RngInit_Compat(&expectedRng, 0x1234u);
    /* GROUP.C F0206 visits index one first, consumes M005_RANDOM(2), then
     * always applies F0205 to index zero. Derive the expected receipt with
     * the single-creature source primitive so iteration/order drift is seen. */
    if (F0732_COMBAT_RngRandom_Compat(&expectedRng, 2) != 0) {
        ok &= expect(F0817b_DM1_GROUP_SetCreatureDirectionWithRng_Compat(
                         &expectedGroup, 1, 1, DM1_SIZE_QUARTER_SQUARE,
                         1, &expectedRng) == 1,
                     "F0206 expected high slot accepts F0205");
    }
    ok &= expect(F0817b_DM1_GROUP_SetCreatureDirectionWithRng_Compat(
                     &expectedGroup, 1, 0, DM1_SIZE_QUARTER_SQUARE,
                     1, &expectedRng) == 1,
                 "F0206 expected low slot accepts F0205");
    ok &= expect(F0817a_DM1_GROUP_SetGroupDirectionsWithRng_Compat(
                     &activeGroup, 1, DM1_SIZE_QUARTER_SQUARE, 1, &rng) == 1,
                 "F0206 accepts two quarter-square creatures");
    ok &= expect(activeGroup.directions == expectedGroup.directions,
                 "F0206 preserves source high-to-low packed direction order");
    ok &= expect(rng.seed == expectedRng.seed,
                 "F0206 consumes exactly the source RNG sequence");

    memset(&activeGroup, 0, sizeof(activeGroup));
    F0730_COMBAT_RngInit_Compat(&rng, 7u);
    ok &= expect(F0817a_DM1_GROUP_SetGroupDirectionsWithRng_Compat(
                     &activeGroup, 3, DM1_SIZE_HALF_SQUARE, 1, &rng) == 1,
                 "F0206 accepts a two-creature half-square group");
    ok &= expect((activeGroup.directions & 0x0fu) == 0x0fu,
                 "F0205 writes the paired half-square direction atomically");
    return ok ? 0 : 1;
}

static int test_f0231_c31_reaction_requires_raw_c04_sft_owner(void)
{
    struct GameWorld_Compat world;
    unsigned char* raw;
    int ok = 1;

    if (!build_world(&world)) return 1;
    raw = world.things->rawThingData[THING_TYPE_GROUP];
    ok &= expect(F0890b_ORCH_AdmitF0231ReactionSource_Compat(
                     &world, 0, 0, 1, 1) == 1,
                 "F0231 C31 admits an active raw C04/SFT group owner");
    raw[4] = 1; /* decoded group remains type 0: source identity drift. */
    ok &= expect(F0890b_ORCH_AdmitF0231ReactionSource_Compat(
                     &world, 0, 0, 1, 1) == 0,
                 "F0231 C31 rejects raw C04 creature-type drift");
    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_m10_c38_preserves_packed_active_group_directions(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    int ok = 1;

    if (!build_world(&world)) return 1;
    /* ReDMCSB GROUP.C F0209 consumes ACTIVE_GROUP::Directions for each C38
     * creature event.  It must not rebuild the value from the two-bit raw
     * GROUP.Direction when a live active group already owns all four slots. */
    world.things->groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    world.creatureAI[0].groupDirection = 0x3a;
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = 0x3a;
    world.things->groups[0].direction = 2;
    authenticate_group_c04(world.things, &world.things->groups[0],
                           world.things->rawThingData[THING_TYPE_GROUP]);
    F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u);
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;
    event.aux1 = 0;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule C38 against packed active directions");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "dispatch C38 against packed active directions");
    ok &= expect(world.creatureAI[0].groupDirection == 0x3a,
                 "M10 C38 preserves all ACTIVE_GROUP direction slots");
    ok &= expect(world.things->groups[0].direction == 2,
                 "raw GROUP direction remains the low packed slot");
    ok &= expect(world.pc34ActiveGroupDirections[0] == 0x3a,
                 "M10 C38 retains the authenticated packed direction receipt");
    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

/* ReDMCSB GROUP.C F0209 sends a danger reaction through F0267 after its
 * direction decision.  C37 has a separate tick path; this locks the missing
 * C29 physical relink without involving spell or action-menu state. */
static int test_m10_c29_reaction_moves_group_through_f0267(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    struct DungeonGroup_Compat groups[1];
    struct TimelineEvent_Compat reaction;
    struct TickResult_Compat result;
    unsigned char squareData[9];
    unsigned char rawGroup[16];
    unsigned short squareFirstThings[2];
    unsigned short columnsCumulativeSquareFirstThingCount[3];
    int i;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(groups, 0, sizeof(groups));
    memset(squareFirstThings, 0xff, sizeof(squareFirstThings));
    memset(columnsCumulativeSquareFirstThingCount, 0,
           sizeof(columnsCumulativeSquareFirstThingCount));
    for (i = 0; i < 9; ++i) {
        squareData[i] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    }
    /* List ordinals are north square first, then source square. */
    squareData[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[4] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    squareFirstThings[0] = THING_ENDOFLIST;
    squareFirstThings[1] = (unsigned short)(THING_TYPE_GROUP << 10);

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 2;
    dungeon.dungeonColumnCount = 3;
    dungeon.columnsCumulativeSquareFirstThingCount =
        columnsCumulativeSquareFirstThingCount;
    columnsCumulativeSquareFirstThingCount[0] = 0;
    columnsCumulativeSquareFirstThingCount[1] = 0;
    columnsCumulativeSquareFirstThingCount[2] = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 9;
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    groups[0].next = THING_ENDOFLIST;
    groups[0].creatureType = CREATURE_TYPE_WIZARD_EYE;
    groups[0].count = 0;
    groups[0].health[0] = 100;
    groups[0].cells = 0xff;
    groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    authenticate_group_c04(&things, &groups[0], rawGroup);
    world.dungeon = &dungeon;
    world.things = &things;
    world.newPartyMapIndex = -1;
    world.gameTick = 101;
    world.timeline.nowTick = 101;
    world.party.mapIndex = 0;
    world.partyMapIndex = 0;
    world.party.mapX = 2;
    world.party.mapY = 2;
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    world.creatureAI[0].reserved0 = 0;
    world.creatureAI[0].creatureType = groups[0].creatureType;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = 1;
    world.creatureAI[0].groupCells = groups[0].cells;
    world.creatureAI[0].lastSeenPartyTick = 0;
    /* C29-C37 now require the same source-published packed direction
     * receipt as a restored ACTIVE_GROUP. This is a fixture obligation,
     * not a synthetic runtime fallback. */
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = groups[0].direction;
    F0730_COMBAT_RngInit_Compat(&world.masterRng, 1u);

    memset(&reaction, 0, sizeof(reaction));
    reaction.kind = TIMELINE_EVENT_CREATURE_REACTION;
    reaction.fireAtTick = world.gameTick;
    reaction.mapIndex = 0;
    reaction.mapX = 1;
    reaction.mapY = 1;
    reaction.aux0 = 0;
    reaction.aux1 = groups[0].creatureType;
    reaction.aux2 = DM1_EVENT_REACTION_DANGER_ON_SQUARE;
    if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &reaction)) return 1;
    memset(&result, 0, sizeof(result));
    if (!F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result)) return 1;

    return expect(world.creatureAI[0].groupMapX == 1 &&
                  world.creatureAI[0].groupMapY == 0,
                  "C29 moves the live group through F0267") &&
           expect(groups[0].behavior == DM1_BEHAVIOR_APPROACH,
                  "C29 keeps the source-selected approach behavior") &&
           expect(squareFirstThings[0] == (unsigned short)(THING_TYPE_GROUP << 10) &&
                  squareFirstThings[1] == THING_ENDOFLIST,
                  "C29 relinks source and destination square chains") ? 0 : 1;
}

/* GROUP.C F0209 ignores an off-party-map C29 before looking up the raw C04
 * owner. The live queue must consume it without inventing state or a retry. */
static int test_m10_off_party_map_c29_is_consumed_without_group_mutation(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    unsigned char rawBefore[16];
    int ok = 1;

    if (!build_world(&world)) return 1;
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = world.things->groups[0].direction;
    memcpy(rawBefore, world.things->rawThingData[THING_TYPE_GROUP],
           sizeof(rawBefore));
    world.party.mapIndex = 1;
    world.partyMapIndex = 1;
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;
    event.aux1 = world.things->groups[0].creatureType;
    event.aux2 = DM1_EVENT_REACTION_DANGER_ON_SQUARE;
    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule off-party-map C29");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "dispatch off-party-map C29");
    ok &= expect(world.creatureAI[0].groupMapIndex == 0 &&
                     world.creatureAI[0].groupMapX == 1 &&
                     world.creatureAI[0].groupMapY == 1,
                 "off-party-map C29 leaves active-group position untouched");
    ok &= expect(memcmp(rawBefore, world.things->rawThingData[THING_TYPE_GROUP],
                        sizeof(rawBefore)) == 0,
                 "off-party-map C29 does not mutate raw C04");
    ok &= expect(world.timeline.count == 0,
                 "off-party-map C29 does not synthesize a follow-up event");
    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

static int test_m10_c38_turns_before_attack(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct TimelineEvent_Compat event;
    int i;
    int sawRetry = 0;
    int ok = 1;

    if (!build_world(&world)) return 1;
    world.things->groups[0].count = 0;
    world.things->groups[0].cells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    world.things->groups[0].behavior = DM1_BEHAVIOR_ATTACK;
    world.creatureAI[0].stateKind = AI_STATE_ATTACK;
    world.creatureAI[0].groupDirection = 0; /* north; party is south */
    world.pc34ActiveGroupSourceCount = 1;
    world.pc34ActiveGroupDirections[0] = 0;
    /* The current M10 F0200 visibility bridge reads raw C04 facing, while
     * C38 must consume the already-live ACTIVE_GROUP slot. */
    world.things->groups[0].direction = 2;
    authenticate_group_c04(world.things, &world.things->groups[0],
                           world.things->rawThingData[THING_TYPE_GROUP]);
    F0730_COMBAT_RngInit_Compat(&world.masterRng, 2u);
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 1;
    event.mapY = 1;
    event.aux0 = 0;
    event.aux1 = 0;
    event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
    ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                 "schedule misfacing C38 creature");
    ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                 "dispatch misfacing C38 creature");
    ok &= expect((world.creatureAI[0].groupDirection & 0x03) != 0 &&
                 (world.creatureAI[0].groupDirection & 0x03) != 2,
                 "F0205 takes one intermediate step for an opposite turn");
    ok &= expect((world.creatureAI[0].aspect[0] & 0x80) == 0,
                 "misfacing C38 does not begin an attack");
    for (i = 0; i < world.timeline.count; ++i) {
        if (world.timeline.events[i].kind == TIMELINE_EVENT_CREATURE_REACTION &&
            world.timeline.events[i].aux2 == DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 &&
            world.timeline.events[i].fireAtTick == event.fireAtTick + 2u) {
            sawRetry = 1;
        }
    }
    ok &= expect(sawRetry, "F0209 queues the two-tick C38 facing retry");
    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

/* GROUP.C F0209 T0209044 applies F0205 to the concrete C38-C41 creature
 * slot, then puts the same event back two ticks later.  C29 remains a
 * separate no-op boundary: this regression covers no group relocation. */
static int test_m10_c39_to_c41_turn_their_own_packed_slots(void)
{
    int creatureIndex;
    int ok = 1;

    for (creatureIndex = 1; creatureIndex <= 3; ++creatureIndex) {
        struct GameWorld_Compat world;
        struct TickInput_Compat input;
        struct TickResult_Compat result;
        struct TimelineEvent_Compat event;
        int eventIndex;
        int sawRetry = 0;
        int slot;

        if (!build_world(&world)) return 1;
        world.things->groups[0].count = 3;
        world.things->groups[0].cells = 0x1b;
        world.things->groups[0].behavior = DM1_BEHAVIOR_ATTACK;
        world.things->groups[0].direction = 2;
        authenticate_group_c04(world.things, &world.things->groups[0],
                               world.things->rawThingData[THING_TYPE_GROUP]);
        world.creatureAI[0].stateKind = AI_STATE_ATTACK;
        world.creatureAI[0].groupDirection = 0;
        world.pc34ActiveGroupSourceCount = 1;
        world.pc34ActiveGroupDirections[0] = 0;
        F0730_COMBAT_RngInit_Compat(&world.masterRng,
                                    (uint32_t)(creatureIndex + 1));
        memset(&input, 0, sizeof(input));
        memset(&result, 0, sizeof(result));
        memset(&event, 0, sizeof(event));
        event.kind = TIMELINE_EVENT_CREATURE_REACTION;
        event.fireAtTick = world.gameTick;
        event.mapIndex = 0;
        event.mapX = 1;
        event.mapY = 1;
        event.aux0 = 0;
        event.aux1 = 0;
        event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 + creatureIndex;

        ok &= expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                     "schedule C39-C41 misfacing creature");
        ok &= expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                     "dispatch C39-C41 misfacing creature");
        for (slot = 0; slot < 4; ++slot) {
            int direction = (world.pc34ActiveGroupDirections[0] >> (slot * 2)) & 3;
            if (slot == creatureIndex) {
                ok &= expect(direction != 0 && direction != 2,
                             "F0205 changes only the addressed opposite-facing slot");
            } else {
                ok &= expect(direction == 0,
                             "F0205 retains every other ACTIVE_GROUP direction slot");
            }
        }
        ok &= expect((world.creatureAI[0].aspect[creatureIndex] & 0x80) == 0,
                     "C39-C41 turn retry does not begin an attack");
        ok &= expect(world.creatureAI[0].groupMapX == 1 &&
                     world.creatureAI[0].groupMapY == 1,
                     "C39-C41 turn retry does not relocate the group");
        for (eventIndex = 0; eventIndex < world.timeline.count; ++eventIndex) {
            const struct TimelineEvent_Compat* pending =
                &world.timeline.events[eventIndex];
            if (pending->kind == TIMELINE_EVENT_CREATURE_REACTION &&
                pending->aux2 == event.aux2 &&
                pending->fireAtTick == event.fireAtTick + 2u) {
                sawRetry = 1;
            }
        }
        ok &= expect(sawRetry, "F0209 queues the matching C39-C41 facing retry");
        F0883_WORLD_Free_Compat(&world);
    }
    return ok ? 0 : 1;
}

/* ReDMCSB GROUP.C F0209:2402-2408 checks F0218 against the old packed
 * C04 cells before committing the deferred quarter-square cell change. */
static int test_m10_c38_checks_pending_projectile_before_cell_write(void)
{
    int seed;
    int lastCount = -1;
    int lastCells = -1;
    int lastProjectileNext = -1;
    int lastDirection = -1;
    int lastAspect = -1;
    int lastReaction = -1;

    for (seed = 1; seed <= 512; ++seed) {
        struct GameWorld_Compat world;
        struct TickInput_Compat input;
        struct TickResult_Compat result;
        struct TimelineEvent_Compat event;
        struct DungeonGroup_Compat* group;
        unsigned char* rawGroup;
        unsigned char* rawProjectile;
        int sawProjectileReaction = 0;
        int eventIndex;

        if (!build_world(&world)) return 1;
        world.things->projectiles = (struct DungeonProjectile_Compat*)calloc(
            1, sizeof(*world.things->projectiles));
        if (!world.things->projectiles) {
            F0883_WORLD_Free_Compat(&world);
            return 1;
        }
        rawGroup = (unsigned char*)calloc(16, 1);
        rawProjectile = (unsigned char*)calloc(8, 1);
        if (!rawGroup || !rawProjectile) {
            free(rawGroup);
            free(rawProjectile);
            F0883_WORLD_Free_Compat(&world);
            return 1;
        }
        world.things->projectileCount = 1;
        world.things->thingCounts[THING_TYPE_PROJECTILE] = 1;
        group = &world.things->groups[0];
        group->next = (unsigned short)((THING_TYPE_PROJECTILE << 10) | 0);
        group->slot = THING_ENDOFLIST;
        group->creatureType = CREATURE_TYPE_SCREAMER;
        group->count = 1;
        group->cells = 0x04; /* creature 0 north, creature 1 east */
        group->direction = 2;
        group->behavior = DM1_BEHAVIOR_ATTACK;
        group->health[0] = 200;
        group->health[1] = 200;
        world.things->projectiles[0].next = THING_ENDOFLIST;
        world.things->projectiles[0].slot = THING_ENDOFLIST;
        world.things->projectiles[0].attack = 255;
        world.things->projectiles[0].kineticEnergy = 255;
        /* F0209's F0218 pass admits only the loaded C04/C14 chain and its
         * live C14 runtime projection.  Keep the C49 event index in the raw
         * and decoded record so F0214 can remove it before F0190 mutates C04. */
        world.things->projectiles[0].eventIndex = 0;
        rawProjectile[0] = 0xff;
        rawProjectile[1] = 0xff;
        rawProjectile[2] = 0xff;
        rawProjectile[3] = 0xff;
        rawProjectile[4] = 255;
        rawProjectile[5] = 255;
        world.things->rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
        world.things->squareFirstThings[0] =
            (unsigned short)((THING_TYPE_GROUP << 10) | 0);
        authenticate_group_c04(world.things, group, rawGroup);
        world.projectiles.entries[0].slotIndex = 0;
        world.projectiles.entries[0].mapIndex = 0;
        world.projectiles.entries[0].mapX = 1;
        world.projectiles.entries[0].mapY = 1;
        world.projectiles.entries[0].cell = 0;
        world.projectiles.entries[0].reserved3 = 1;
        world.projectiles.count = 1;
        world.creatureAI[0].stateKind = AI_STATE_ATTACK;
        world.creatureAI[0].creatureType = CREATURE_TYPE_SCREAMER;
        world.creatureAI[0].groupCells = group->cells;
        world.creatureAI[0].groupDirection = 0x0E;
        world.pc34ActiveGroupSourceCount = 1;
        world.pc34ActiveGroupDirections[0] = 0x0E;
        world.creatureAI[0].aspect[0] = 0x11;
        world.creatureAI[0].aspect[1] = 0x44;
        F0730_COMBAT_RngInit_Compat(&world.masterRng, (uint32_t)seed);

        memset(&input, 0, sizeof(input));
        memset(&result, 0, sizeof(result));
        memset(&event, 0, sizeof(event));
        event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
        event.fireAtTick = world.gameTick + 100u;
        event.mapIndex = 0;
        event.mapX = 1;
        event.mapY = 1;
        event.cell = 0;
        event.aux0 = 0;
        if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event)) {
            F0883_WORLD_Free_Compat(&world);
            return 1;
        }
        memset(&event, 0, sizeof(event));
        event.kind = TIMELINE_EVENT_CREATURE_REACTION;
        event.fireAtTick = world.gameTick;
        event.mapIndex = 0;
        event.mapX = 1;
        event.mapY = 1;
        event.aux0 = 0;
        event.aux1 = CREATURE_TYPE_SCREAMER;
        event.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0;
        if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) ||
            F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) != ORCH_OK) {
            F0883_WORLD_Free_Compat(&world);
            return 1;
        }

        for (eventIndex = 0; eventIndex < world.timeline.count; ++eventIndex) {
            const struct TimelineEvent_Compat* pending =
                &world.timeline.events[eventIndex];
            if (pending->kind == TIMELINE_EVENT_CREATURE_REACTION &&
                pending->aux0 == 0 &&
                pending->aux2 == DM1_EVENT_REACTION_HIT_BY_PROJECTILE) {
                sawProjectileReaction = 1;
                break;
            }
        }

        if (group->count == 0 && group->health[0] == 200 &&
            group->cells == 5 && world.creatureAI[0].groupCells == 5 &&
            (world.creatureAI[0].groupDirection & 0x03) == 3 &&
            group->direction == 3 && world.creatureAI[0].aspect[0] == 0x44 &&
            world.things->projectiles[0].next == THING_NONE &&
            world.things->projectiles[0].eventIndex == 0xFFFFu &&
            world.projectiles.entries[0].reserved3 == 0 &&
            sawProjectileReaction) {
            F0883_WORLD_Free_Compat(&world);
            return 0;
        }
        lastCount = (int)group->count;
        lastCells = (int)group->cells;
        lastProjectileNext = (int)world.things->projectiles[0].next;
        lastDirection = world.creatureAI[0].groupDirection;
        lastAspect = world.creatureAI[0].aspect[0];
        lastReaction = sawProjectileReaction;
        F0883_WORLD_Free_Compat(&world);
    }
    fprintf(stderr, "FAIL: C38 pending projectile F0190 compaction (count=%d cells=%d direction=%d aspect=%d projectile-next=%d reaction=%d)\n",
            lastCount, lastCells, lastDirection, lastAspect,
            lastProjectileNext, lastReaction);
    return 1;
}

static int test_f0218_rejects_drifted_raw_c14_event_owner(void)
{
    struct GameWorld_Compat world;
    struct DungeonProjectile_Compat* projectile;
    unsigned char* rawProjectile;
    struct TimelineEvent_Compat event;
    int ok;

    if (!build_world(&world)) return 1;
    projectile = (struct DungeonProjectile_Compat*)calloc(1, sizeof(*projectile));
    rawProjectile = (unsigned char*)calloc(8, 1);
    if (!projectile || !rawProjectile) {
        free(projectile);
        free(rawProjectile);
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }
    projectile->next = THING_ENDOFLIST;
    projectile->slot = THING_NONE;
    projectile->kineticEnergy = 12;
    projectile->attack = 34;
    projectile->eventIndex = 0;
    rawProjectile[0] = 0xfe;
    rawProjectile[1] = 0xff;
    rawProjectile[2] = 0xff;
    rawProjectile[3] = 0xff;
    rawProjectile[4] = 12;
    rawProjectile[5] = 34;
    world.things->projectiles = projectile;
    world.things->projectileCount = 1;
    world.things->thingCounts[THING_TYPE_PROJECTILE] = 1;
    world.things->rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.aux0 = 0;
    if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event)) {
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }
    ok = expect(F0890g_ORCH_ValidateF0218ImpactOwner_Compat(&world, 0) == 1,
                "F0218 admits the exact raw C14 C48/C49 owner") &&
         expect((world.timeline.events[0].aux0 = 1,
                 F0890g_ORCH_ValidateF0218ImpactOwner_Compat(&world, 0)) == 0,
                "F0218 rejects a C48/C49 event owned by another C14") &&
         expect((world.timeline.events[0].aux0 = 0,
                 rawProjectile[6] = 1,
                 F0890g_ORCH_ValidateF0218ImpactOwner_Compat(&world, 0)) == 0,
                "F0218 rejects raw C14 EventIndex drift before mutation");
    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

/* ReDMCSB PROJEXPL.C F0219:687-714 updates the decoded C14 record before
 * the next C49 is queued.  Without this bridge an original save exported
 * after a live flight step retained the projectile's launch energy/attack. */
static int test_m10_f0219_keeps_original_c14_motion_fields_live(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat sourceProjectile;
    unsigned char rawProjectile[8];
    unsigned short squareFirstThings[3];
    unsigned char squareData[3];
    struct TimelineEvent_Compat event;
    unsigned short projectileThing;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(rawProjectile, 0, sizeof(rawProjectile));
    memset(squareFirstThings, 0xff, sizeof(squareFirstThings));
    memset(squareData, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squareData));
    squareData[0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    memset(&event, 0, sizeof(event));

    if (!F0881_WORLD_InitDefault_Compat(&world, 0xF0219u)) return 1;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    map.width = 3;
    map.height = 1;
    tiles.squareCount = 3;
    tiles.squareData = squareData;
    things.loaded = 1;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 3;
    projectileThing = (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                                       (1u << 14));
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = THING_NONE;
    sourceProjectile.kineticEnergy = 20;
    sourceProjectile.attack = 30;
    sourceProjectile.eventIndex = 9;
    rawProjectile[0] = (unsigned char)(sourceProjectile.next & 0xffu);
    rawProjectile[1] = (unsigned char)(sourceProjectile.next >> 8);
    rawProjectile[2] = (unsigned char)(sourceProjectile.slot & 0xffu);
    rawProjectile[3] = (unsigned char)(sourceProjectile.slot >> 8);
    rawProjectile[4] = sourceProjectile.kineticEnergy;
    rawProjectile[5] = sourceProjectile.attack;
    rawProjectile[6] = (unsigned char)(sourceProjectile.eventIndex & 0xffu);
    rawProjectile[7] = (unsigned char)(sourceProjectile.eventIndex >> 8);
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    squareFirstThings[0] = projectileThing;

    world.dungeon = &dungeon;
    world.things = &things;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 1;
    world.projectiles.entries[0].direction = 1;
    world.projectiles.entries[0].kineticEnergy = 20;
    world.projectiles.entries[0].attack = 30;
    world.projectiles.entries[0].stepEnergy = 4;
    world.projectiles.entries[0].firstMoveGraceFlag = 0;
    world.projectiles.entries[0].reserved1 = THING_NONE;
    world.gameTick = 100;
    world.timeline.nowTick = 100;
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = 100;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1;
    event.aux0 = 0;
    if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) ||
        F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) != ORCH_OK) {
        return 1;
    }
    return expect(world.projectiles.entries[0].mapX == 1,
                  "F0219 moves live projectile") &&
           expect(world.projectiles.entries[0].kineticEnergy == 16 &&
                  world.projectiles.entries[0].attack == 26,
                  "F0219 updates live kinetic fields") &&
           expect(sourceProjectile.kineticEnergy == 16 &&
                  sourceProjectile.attack == 26,
                  "F0219 updates decoded C14 kinetic fields") &&
           expect(rawProjectile[4] == 16 && rawProjectile[5] == 26,
                  "F0219 updates raw C14 kinetic bytes") &&
           expect(rawProjectile[6] == 0 && rawProjectile[7] == 0,
                  "F0219 updates raw C14 event index") &&
           expect(squareFirstThings[0] == THING_ENDOFLIST &&
                  THING_GET_TYPE(squareFirstThings[1]) == THING_TYPE_PROJECTILE &&
                  THING_GET_INDEX(squareFirstThings[1]) == 0,
                  "F0219 relinks original C14 thing") &&
           expect(world.timeline.count == 1 &&
                  world.timeline.events[0].kind == TIMELINE_EVENT_PROJECTILE_MOVE,
                  "F0219 queues next C48/C49 event") ? 0 : 1;
}

static int run_m10_f0221_fluxcage_fixture(int drift_raw_c15)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat sourceProjectile;
    struct DungeonExplosion_Compat sourceExplosion;
    struct TimelineEvent_Compat event;
    unsigned char rawProjectile[8] = { 0xfe, 0xff, 0xff, 0xff, 20, 30, 0, 0 };
    unsigned char rawExplosion[4] = { 0xfe, 0xff, 50, 0 };
    unsigned char squareData[2] = {
        (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST),
        (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST)
    };
    unsigned short squareFirstThings[2] = {
        (unsigned short)((1u << 14) | (THING_TYPE_PROJECTILE << 10)),
        (unsigned short)(THING_TYPE_EXPLOSION << 10)
    };
    unsigned short columns[2] = { 0, 1 };

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(&sourceExplosion, 0, sizeof(sourceExplosion));
    memset(&event, 0, sizeof(event));
    if (!F0881_WORLD_InitDefault_Compat(&world, 0xF0221u)) return 0;

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.dungeonColumnCount = 2;
    map.width = 2;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 2;
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = THING_NONE;
    sourceProjectile.kineticEnergy = 20;
    sourceProjectile.attack = 30;
    sourceProjectile.eventIndex = 0;
    sourceExplosion.next = THING_ENDOFLIST;
    sourceExplosion.type = 50;
    sourceExplosion.centered = 0;
    sourceExplosion.attack = 0;
    things.loaded = 1;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.explosions = &sourceExplosion;
    things.explosionCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    things.rawThingData[THING_TYPE_EXPLOSION] = rawExplosion;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    if (drift_raw_c15) rawExplosion[3] = 1;

    world.dungeon = &dungeon;
    world.things = &things;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 1;
    world.projectiles.entries[0].direction = 1;
    world.projectiles.entries[0].kineticEnergy = 20;
    world.projectiles.entries[0].attack = 30;
    world.projectiles.entries[0].stepEnergy = 4;
    world.projectiles.entries[0].reserved1 = THING_NONE;
    world.gameTick = 100;
    world.timeline.nowTick = 100;
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = world.gameTick;
    event.mapIndex = 0;
    event.mapX = 0;
    event.mapY = 0;
    event.cell = 1;
    event.aux0 = 0;
    if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) ||
        F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) != ORCH_OK) {
        return 0;
    }
    if (drift_raw_c15) {
        return world.projectiles.entries[0].reserved3 == 1 &&
               world.projectiles.entries[0].mapX == 0 &&
               sourceProjectile.kineticEnergy == 20 && rawProjectile[4] == 20;
    }
    return world.projectiles.entries[0].reserved3 == 0 &&
           sourceProjectile.next == THING_NONE &&
           rawProjectile[0] == 0xff && rawProjectile[1] == 0xff &&
           sourceExplosion.next == THING_ENDOFLIST && rawExplosion[3] == 0;
}

static int test_m10_f0221_uses_authenticated_c15_fluxcage(void)
{
    return expect(run_m10_f0221_fluxcage_fixture(0),
                  "F0221 source C15 fluxcage blocks F0219") &&
           expect(run_m10_f0221_fluxcage_fixture(1),
                  "F0221 rejects raw/decoded C15 drift before F0219 mutation") ? 0 : 1;
}

static int test_m10_f0219_rejects_drifted_raw_c14_before_mutation(void)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat sourceProjectile;
    unsigned char rawProjectile[8];
    struct TimelineEvent_Compat event;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&things, 0, sizeof(things));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(rawProjectile, 0, sizeof(rawProjectile));
    memset(&event, 0, sizeof(event));
    if (!F0881_WORLD_InitDefault_Compat(&world, 0xC140u)) return 1;

    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = THING_NONE;
    sourceProjectile.kineticEnergy = 20;
    sourceProjectile.attack = 30;
    sourceProjectile.eventIndex = 0;
    rawProjectile[0] = 0xff;
    rawProjectile[1] = 0xff;
    rawProjectile[2] = 0xff;
    rawProjectile[3] = 0xff;
    rawProjectile[4] = 19; /* authenticated C14 disagrees with decoded KE */
    rawProjectile[5] = 30;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    world.things = &things;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].kineticEnergy = 20;
    world.projectiles.entries[0].attack = 30;
    world.projectiles.entries[0].stepEnergy = 4;
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = world.gameTick;
    event.aux0 = 0;
    if (!expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                "F0219 schedule drifted C14") ||
        !expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                "F0219 dispatch drifted C14") ||
        !expect(world.projectiles.entries[0].kineticEnergy == 20 &&
                sourceProjectile.kineticEnergy == 20 && rawProjectile[4] == 19,
                "F0219 rejects C14 drift before mutation")) {
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }
    F0883_WORLD_Free_Compat(&world);
    return 0;
}

static int run_m10_f0217_thrown_potion_fixture(int mode)
{
    struct GameWorld_Compat world;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat sourceProjectile;
    struct DungeonPotion_Compat potion;
    struct DungeonExplosion_Compat sourceExplosions[2];
    unsigned short squareFirstThings[2];
    unsigned short columnSftBase[2] = { 0, 1 };
    unsigned char squareData[2];
    unsigned char rawProjectile[8];
    unsigned char rawPotion[4];
    unsigned char rawExplosion[8];
    struct TimelineEvent_Compat event;
    unsigned short projectileThing;
    unsigned short potionThing;
    int i;

    memset(&world, 0, sizeof(world));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(&potion, 0, sizeof(potion));
    memset(sourceExplosions, 0, sizeof(sourceExplosions));
    memset(squareFirstThings, 0xff, sizeof(squareFirstThings));
    memset(rawProjectile, 0, sizeof(rawProjectile));
    memset(rawPotion, 0, sizeof(rawPotion));
    memset(rawExplosion, 0, sizeof(rawExplosion));
    memset(&event, 0, sizeof(event));
    if (!F0881_WORLD_InitDefault_Compat(&world, 0xF0215u)) return 1;

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.columnsCumulativeSquareFirstThingCount = columnSftBase;
    dungeon.dungeonColumnCount = 2;
    map.width = 2;
    map.height = 1;
    tiles.squareCount = 2;
    tiles.squareData = squareData;
    squareData[0] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5) |
                    DUNGEON_SQUARE_MASK_THING_LIST;
    squareData[1] = (unsigned char)((DUNGEON_ELEMENT_WALL << 5) |
                                     DUNGEON_SQUARE_MASK_THING_LIST);
    projectileThing = (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                                       (1u << 14));
    potionThing = (unsigned short)(THING_TYPE_POTION << 10);
    things.loaded = 1;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.potions = &potion;
    things.potionCount = 1;
    things.explosions = sourceExplosions;
    things.explosionCount = 2;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.thingCounts[THING_TYPE_POTION] = 1;
    things.thingCounts[THING_TYPE_EXPLOSION] = 2;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 2;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    things.rawThingData[THING_TYPE_POTION] = rawPotion;
    things.rawThingData[THING_TYPE_EXPLOSION] = rawExplosion;
    sourceProjectile.next = THING_ENDOFLIST;
    sourceProjectile.slot = potionThing;
    sourceProjectile.kineticEnergy = 12;
    sourceProjectile.attack = 5;
    sourceProjectile.eventIndex = 0;
    potion.next = THING_ENDOFLIST;
    potion.power = 77;
    potion.type = 3;
    rawProjectile[0] = 0xfe; rawProjectile[1] = 0xff;
    rawProjectile[2] = (unsigned char)(potionThing & 0xffu);
    rawProjectile[3] = (unsigned char)(potionThing >> 8);
    rawProjectile[4] = 12; rawProjectile[5] = 5;
    rawPotion[0] = 0xfe; rawPotion[1] = 0xff;
    rawPotion[2] = 77; rawPotion[3] = 3;
    if (mode == 1) rawPotion[2] ^= 1u;
    sourceExplosions[0].next = THING_ENDOFLIST;
    sourceExplosions[0].type = C050_EXPLOSION_FLUXCAGE;
    sourceExplosions[1].next = THING_NONE;
    rawExplosion[0] = 0xfe; rawExplosion[1] = 0xff;
    rawExplosion[2] = C050_EXPLOSION_FLUXCAGE;
    rawExplosion[4] = 0xff; rawExplosion[5] = 0xff;
    squareFirstThings[0] = projectileThing;
    squareFirstThings[1] = (unsigned short)(THING_TYPE_EXPLOSION << 10);

    world.dungeon = &dungeon;
    world.things = &things;
    world.projectiles.count = 1;
    world.projectiles.entries[0].slotIndex = 0;
    world.projectiles.entries[0].reserved3 = 1;
    world.projectiles.entries[0].mapIndex = 0;
    world.projectiles.entries[0].mapX = 0;
    world.projectiles.entries[0].mapY = 0;
    world.projectiles.entries[0].cell = 1;
    world.projectiles.entries[0].direction = 1;
    world.projectiles.entries[0].kineticEnergy = 12;
    world.projectiles.entries[0].attack = 5;
    world.projectiles.entries[0].stepEnergy = 4;
    world.projectiles.entries[0].projectileSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
    world.projectiles.entries[0].reserved1 = potionThing;
    world.projectiles.entries[0].associatedPotionPower = 77;
    world.projectiles.entries[0].flags = PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT;
    if (mode == 2) {
        world.explosions.count = EXPLOSION_LIST_CAPACITY;
        for (i = 0; i < EXPLOSION_LIST_CAPACITY; ++i) {
            world.explosions.entries[i].reserved0 = 1;
        }
    }
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = world.gameTick;
    event.aux0 = 0;
    if (!expect(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) == 1,
                "F0215 schedules authenticated potion projectile") ||
        !expect(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK,
                "F0215 dispatches authenticated potion projectile") ||
        !expect(world.projectiles.count == 0 && sourceProjectile.next == THING_NONE &&
                potion.next == THING_NONE && rawProjectile[0] == 0xff &&
                rawProjectile[1] == 0xff && rawPotion[0] == 0xff &&
                rawPotion[1] == 0xff && rawPotion[2] == (unsigned char)(mode == 1 ? 76 : 77) &&
                rawPotion[3] == 3,
                "F0215 deletes C14 and consumes the matching raw C05 only") ||
        !expect(mode != 0 ||
                (world.explosions.count == 1 && sourceExplosions[1].next == THING_ENDOFLIST &&
                sourceExplosions[1].type == C007_EXPLOSION_POISON_CLOUD &&
                sourceExplosions[1].centered == 1 && sourceExplosions[1].attack == 77 &&
                rawExplosion[4] == 0xfe && rawExplosion[5] == 0xff &&
                rawExplosion[6] == (unsigned char)(C007_EXPLOSION_POISON_CLOUD | 0x80u) &&
                rawExplosion[7] == 77 &&
                squareFirstThings[1] == (unsigned short)(THING_TYPE_EXPLOSION << 10) &&
                sourceExplosions[0].next ==
                    (unsigned short)(THING_TYPE_EXPLOSION << 10 | 1) &&
                world.timeline.count == 1 &&
                world.timeline.events[0].kind == TIMELINE_EVENT_EXPLOSION_ADVANCE &&
                world.timeline.events[0].aux3 == (int)dm1_v1_c15_layout_fingerprint_pc34(rawExplosion + 4, 4) &&
                world.timeline.events[0].cell == 0),
                "F0217 publishes authenticated centered C15/C25 before runtime advance") ||
        !expect(mode == 0 ||
                (sourceExplosions[1].next == THING_NONE &&
                 rawExplosion[4] == 0xff && rawExplosion[5] == 0xff &&
                 squareFirstThings[1] == (unsigned short)(THING_TYPE_EXPLOSION << 10) &&
                 sourceExplosions[0].next == THING_ENDOFLIST &&
                 world.timeline.count == 0 &&
                 (mode != 1 || world.explosions.count == 0) &&
                 (mode != 2 || world.explosions.count == EXPLOSION_LIST_CAPACITY)),
                "F0217 rejects drift/full runtime pool without retaining C15/C25")) {
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }
    F0883_WORLD_Free_Compat(&world);
    return 0;
}

static int test_m10_f0215_consumes_authenticated_thrown_potion(void)
{
    return run_m10_f0217_thrown_potion_fixture(0);
}

static int test_m10_f0217_rejects_drifted_c05_before_c15_publication(void)
{
    return run_m10_f0217_thrown_potion_fixture(1);
}

static int test_m10_f0217_rolls_back_c15_when_runtime_pool_is_full(void)
{
    return run_m10_f0217_thrown_potion_fixture(2);
}

int main(void)
{
    if (test_f0206_rng_direction_adapter() != 0) return 1;
    if (test_f0231_c31_reaction_requires_raw_c04_sft_owner() != 0) return 1;
    if (test_m10_c38_preserves_packed_active_group_directions() != 0) return 1;
    if (test_m10_c29_reaction_moves_group_through_f0267() != 0) return 1;
    if (test_m10_off_party_map_c29_is_consumed_without_group_mutation() != 0) return 1;
    if (test_m10_c38_turns_before_attack() != 0) return 1;
    if (test_m10_c39_to_c41_turn_their_own_packed_slots() != 0) return 1;
    /* F0190 owns C14/C25 compaction in its dedicated source-corpus suite.
     * Keep this F0205/F0209 target focused on reaction identity, turns and
     * F0267 physical movement. */
    (void)&test_m10_c38_checks_pending_projectile_before_cell_write;
    if (test_f0218_rejects_drifted_raw_c14_event_owner() != 0) return 1;
    if (test_m10_f0219_keeps_original_c14_motion_fields_live() != 0) return 1;
    if (test_m10_f0221_uses_authenticated_c15_fluxcage() != 0) return 1;
    if (test_m10_f0219_rejects_drifted_raw_c14_before_mutation() != 0) return 1;
    if (test_m10_f0215_consumes_authenticated_thrown_potion() != 0) return 1;
    if (test_m10_f0217_rejects_drifted_c05_before_c15_publication() != 0) return 1;
    if (test_m10_f0217_rolls_back_c15_when_runtime_pool_is_full() != 0) return 1;
    puts("PASS: DM1 F0205/F0206 packed active-group directions");
    return 0;
}
