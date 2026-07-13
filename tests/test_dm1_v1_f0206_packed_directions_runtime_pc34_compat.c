#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

static int expect(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
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
    dungeon->maps = (struct DungeonMapDesc_Compat*)calloc(1, sizeof(*dungeon->maps));
    dungeon->tiles = (struct DungeonMapTiles_Compat*)calloc(1, sizeof(*dungeon->tiles));
    if (!dungeon->maps || !dungeon->tiles) goto fail;
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

    things->loaded = 1;
    things->squareFirstThingCount = 1;
    things->squareFirstThings = (unsigned short*)calloc(1, sizeof(unsigned short));
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->groups = (struct DungeonGroup_Compat*)calloc(1, sizeof(*things->groups));
    if (!things->squareFirstThings || !things->groups) goto fail;
    things->squareFirstThings[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    things->groups[0].next = THING_ENDOFLIST;
    things->groups[0].creatureType = 0;
    things->groups[0].count = 1;
    things->groups[0].cells = 0x04; /* creatures in cells 0 and 1 */
    things->groups[0].direction = 0;
    things->groups[0].behavior = DM1_BEHAVIOR_WANDER;
    things->groups[0].health[0] = 100;
    things->groups[0].health[1] = 100;

    world->dungeon = dungeon;
    world->things = things;
    world->ownsDungeon = 1;
    world->party.mapIndex = 0;
    world->partyMapIndex = 0;
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
        free(dungeon->maps);
        free(dungeon->tiles);
    }
    if (things) {
        free(things->squareFirstThings);
        free(things->groups);
    }
    free(dungeon);
    free(things);
    return 0;
}

static int test_f0206_rng_direction_adapter(void)
{
    struct DM1ActiveGroup_Compat activeGroup;
    struct RngState_Compat rng;
    int ok = 1;

    memset(&activeGroup, 0, sizeof(activeGroup));
    F0730_COMBAT_RngInit_Compat(&rng, 0x1234u);
    ok &= expect(F0817a_DM1_GROUP_SetGroupDirectionsWithRng_Compat(
                     &activeGroup, 1, DM1_SIZE_QUARTER_SQUARE, 1, &rng) == 1,
                 "F0206 accepts two quarter-square creatures");
    ok &= expect((activeGroup.directions & 0x03) == 1,
                 "F0206 always turns creature zero");
    ok &= expect((activeGroup.directions & ~0x03) != 0,
                 "F0206 may retain a distinct packed direction for creature one");
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
    world.things->groups[0].direction = 2;
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
    /* The current M10 F0200 visibility bridge reads raw C04 facing, while
     * C38 must consume the already-live ACTIVE_GROUP slot. */
    world.things->groups[0].direction = 2;
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

/* ReDMCSB GROUP.C F0209:2402-2408 checks F0218 against the old packed
 * C04 cells before committing the deferred quarter-square cell change. */
static int test_m10_c38_checks_pending_projectile_before_cell_write(void)
{
    int seed;
    int lastCount = -1;
    int lastCells = -1;
    int lastProjectileNext = -1;

    for (seed = 1; seed <= 512; ++seed) {
        struct GameWorld_Compat world;
        struct TickInput_Compat input;
        struct TickResult_Compat result;
        struct TimelineEvent_Compat event;
        struct DungeonGroup_Compat* group;

        if (!build_world(&world)) return 1;
        world.things->projectiles = (struct DungeonProjectile_Compat*)calloc(
            1, sizeof(*world.things->projectiles));
        if (!world.things->projectiles) {
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
        world.things->projectiles[0].eventIndex = 77;
        world.things->squareFirstThings[0] =
            (unsigned short)((THING_TYPE_GROUP << 10) | 0);
        world.creatureAI[0].stateKind = AI_STATE_ATTACK;
        world.creatureAI[0].creatureType = CREATURE_TYPE_SCREAMER;
        world.creatureAI[0].groupCells = group->cells;
        world.creatureAI[0].groupDirection = 0x0A; /* both face south */
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

        if (group->count == 0 && group->health[0] == 200 &&
            group->cells == 5 && world.creatureAI[0].groupCells == 5 &&
            world.things->projectiles[0].next == THING_NONE) {
            F0883_WORLD_Free_Compat(&world);
            return 0;
        }
        lastCount = (int)group->count;
        lastCells = (int)group->cells;
        lastProjectileNext = (int)world.things->projectiles[0].next;
        F0883_WORLD_Free_Compat(&world);
    }
    fprintf(stderr, "FAIL: C38 pending projectile cell-write ordering (count=%d cells=%d projectile-next=%d)\n",
            lastCount, lastCells, lastProjectileNext);
    return 1;
}

int main(void)
{
    if (test_f0206_rng_direction_adapter() != 0) return 1;
    if (test_m10_c38_preserves_packed_active_group_directions() != 0) return 1;
    if (test_m10_c38_turns_before_attack() != 0) return 1;
    if (test_m10_c38_checks_pending_projectile_before_cell_write() != 0) return 1;
    puts("PASS: DM1 F0205/F0206 packed active-group directions");
    return 0;
}
