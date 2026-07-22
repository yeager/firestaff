#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

static int expect(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

static void authenticate_c04(struct DungeonThings_Compat* things,
                             const struct DungeonGroup_Compat* group,
                             unsigned char* raw)
{
    unsigned short bits = (unsigned short)((group->behavior & 0x0f) |
        ((group->count & 0x03u) << 5) |
        ((group->direction & 0x03u) << 8) |
        ((group->doNotDiscard & 0x01u) << 10));

    raw[0] = (unsigned char)(group->next & 0xffu);
    raw[1] = (unsigned char)(group->next >> 8);
    raw[2] = (unsigned char)(group->slot & 0xffu);
    raw[3] = (unsigned char)(group->slot >> 8);
    raw[4] = group->creatureType;
    raw[5] = group->cells;
    raw[6] = (unsigned char)(group->health[0] & 0xffu);
    raw[7] = (unsigned char)(group->health[0] >> 8);
    raw[14] = (unsigned char)(bits & 0xffu);
    raw[15] = (unsigned char)(bits >> 8);
    things->rawThingData[THING_TYPE_GROUP] = raw;
}

static int build_world(struct GameWorld_Compat* world)
{
    struct DungeonDatState_Compat* dungeon;
    struct DungeonThings_Compat* things;
    int i;

    memset(world, 0, sizeof(*world));
    if (!F0881_WORLD_InitDefault_Compat(world, 0xF0207u)) return 0;
    dungeon = calloc(1, sizeof(*dungeon));
    things = calloc(1, sizeof(*things));
    if (!dungeon || !things) return 0;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    dungeon->header.mapCount = 1;
    dungeon->maps = calloc(1, sizeof(*dungeon->maps));
    dungeon->tiles = calloc(1, sizeof(*dungeon->tiles));
    things->groups = calloc(1, sizeof(*things->groups));
    things->squareFirstThings = calloc(1, sizeof(*things->squareFirstThings));
    if (!dungeon->maps || !dungeon->tiles || !things->groups ||
        !things->squareFirstThings) return 0;
    dungeon->maps[0].width = 3;
    dungeon->maps[0].height = 3;
    dungeon->maps[0].creatureTypeCount = 1;
    dungeon->maps[0].allowedCreatureTypes[0] = 0;
    dungeon->tiles[0].squareCount = 9;
    dungeon->tiles[0].squareData = calloc(9, 1);
    if (!dungeon->tiles[0].squareData) return 0;
    for (i = 0; i < 9; ++i)
        dungeon->tiles[0].squareData[i] = DUNGEON_ELEMENT_CORRIDOR << 5;
    dungeon->tiles[0].squareData[4] |= DUNGEON_SQUARE_MASK_THING_LIST;
    things->loaded = 1;
    things->groupCount = 1;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->squareFirstThingCount = 1;
    things->squareFirstThings[0] = THING_TYPE_GROUP << 10;
    things->groups[0].next = THING_ENDOFLIST;
    things->groups[0].creatureType = 0;
    things->groups[0].count = 0;
    things->groups[0].cells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    things->groups[0].behavior = DM1_BEHAVIOR_WANDER;
    things->groups[0].health[0] = 100;
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
    world->creatureAICount = 1;
    world->creatureAI[0].reserved0 = 0;
    world->creatureAI[0].stateKind = AI_STATE_WANDER;
    world->creatureAI[0].creatureType = 0;
    world->creatureAI[0].groupMapIndex = 0;
    world->creatureAI[0].groupMapX = 1;
    world->creatureAI[0].groupMapY = 1;
    world->creatureAI[0].groupCells = RUNTIME_GROUP_CELLS_SINGLE_CENTERED;
    return 1;
}

int main(void)
{
    unsigned int seed;

    for (seed = 1; seed <= 64; ++seed) {
        struct GameWorld_Compat world;
        struct TickInput_Compat input;
        struct TickResult_Compat result;
        struct TimelineEvent_Compat event;
        unsigned char* raw;
        unsigned short rawBits;
        int direction;

        if (!build_world(&world)) return 1;
        raw = calloc(16, 1);
        if (!raw) return 1;
        authenticate_c04(world.things, &world.things->groups[0], raw);
        world.creatureAI[0].groupDirection = 0;
        world.pc34ActiveGroupSourceCount = 1;
        world.pc34ActiveGroupDirections[0] = 0;
        F0730_COMBAT_RngInit_Compat(&world.masterRng, seed);
        memset(&input, 0, sizeof(input));
        memset(&result, 0, sizeof(result));
        memset(&event, 0, sizeof(event));
        event.kind = TIMELINE_EVENT_CREATURE_REACTION;
        event.fireAtTick = world.gameTick;
        event.mapIndex = 0;
        event.mapX = 1;
        event.mapY = 1;
        event.aux0 = 0;
        event.aux2 = DM1_EVENT_REACTION_HIT_BY_PROJECTILE;
        if (!F0721_TIMELINE_Schedule_Compat(&world.timeline, &event) ||
            F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) != ORCH_OK) {
            F0883_WORLD_Free_Compat(&world);
            return 1;
        }
        direction = world.things->groups[0].direction & 3;
        rawBits = (unsigned short)(raw[14] | ((unsigned short)raw[15] << 8));
        if (direction != 0) {
            int ok = expect((world.creatureAI[0].groupDirection & 3) == direction,
                            "C30 F0209 direction reaches C38 state") &&
                     expect((world.pc34ActiveGroupDirections[0] & 3) == direction,
                            "C30 persists packed F0206 receipt") &&
                     expect(((rawBits >> 8) & 3u) == (unsigned int)direction,
                            "C30 writes the PC34 C04 direction bits");
            F0883_WORLD_Free_Compat(&world);
            return ok ? 0 : 1;
        }
        F0883_WORLD_Free_Compat(&world);
    }
    return expect(0, "C30 selects a projectile-impact search direction");
}
