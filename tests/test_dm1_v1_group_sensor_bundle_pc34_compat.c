#include "dm1_v1_group_state_bundle_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, label) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\\n", label); return 1; } \
} while (0)

static void put_u16(unsigned char *p, unsigned int value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
}

static unsigned int make_thing(int type, int index, int cell)
{
    return ((unsigned int)type << 10) | (unsigned int)index |
           ((unsigned int)cell << 14);
}

static void make_world(struct GameWorld_Compat *world,
                       struct DungeonDatState_Compat *dungeon,
                       struct DungeonMapDesc_Compat *maps,
                       struct DungeonMapTiles_Compat *tiles,
                       unsigned char *squares,
                       struct DungeonThings_Compat *things,
                       struct DungeonGroup_Compat *groups,
                       unsigned short *firstThings,
                       unsigned char *groupRaw,
                       unsigned char *weaponRaw)
{
    memset(world, 0, sizeof(*world));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(maps, 0, sizeof(*maps));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, 0, 9u);
    memset(things, 0, sizeof(*things));
    memset(groups, 0, sizeof(*groups));
    memset(firstThings, 0xff, sizeof(unsigned short));
    memset(groupRaw, 0, 16u);
    memset(weaponRaw, 0, 8u);

    dungeon->header.mapCount = 1;
    dungeon->tilesLoaded = 1;
    dungeon->maps = maps;
    dungeon->tiles = tiles;
    maps[0].width = 3;
    maps[0].height = 3;
    tiles[0].squareData = squares;
    tiles[0].squareCount = 9;
    /* Column-major (x=1,y=1) has a real SFT owner. */
    squares[4] = DUNGEON_SQUARE_MASK_THING_LIST;

    groups[0].next = THING_ENDOFLIST;
    things->loaded = 1;
    things->groups = groups;
    things->groupCount = 1;
    things->rawThingData[THING_TYPE_GROUP] = groupRaw;
    things->thingCounts[THING_TYPE_GROUP] = 1;
    things->rawThingData[THING_TYPE_WEAPON] = weaponRaw;
    things->thingCounts[THING_TYPE_WEAPON] = 2;
    things->squareFirstThings = firstThings;
    things->squareFirstThingCount = 1;

    world->dungeon = dungeon;
    world->things = things;
    world->creatureAICount = 1;
    world->creatureAI[0].reserved0 = 0;
    world->creatureAI[0].aspect[0] = 0x80;
    world->creatureAI[0].aspect[1] = 0x81;
    world->creatureAI[0].aspect[2] = 0x7f;
    world->creatureAI[0].aspect[3] = 0xff;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    unsigned char squares[9];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    unsigned short firstThings[1];
    unsigned char groupRaw[16];
    unsigned char weaponRaw[8];
    DM1_V1_GroupSensorReceiptPc34Compat receipt;

    make_world(&world, &dungeon, maps, tiles, squares, &things, groups,
               firstThings, groupRaw, weaponRaw);
    world.timeline.count = 3;
    memset(world.timeline.events, 0, sizeof(world.timeline.events));
    world.timeline.events[0].kind = TIMELINE_EVENT_CREATURE_REACTION;
    world.timeline.events[0].mapIndex = 0;
    world.timeline.events[0].mapX = 1;
    world.timeline.events[0].mapY = 1;
    world.timeline.events[0].aux2 = DM1_EVENT_REACTION_PARTY_IS_ADJACENT;
    world.timeline.events[1] = world.timeline.events[0];
    world.timeline.events[1].aux2 = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1;
    world.timeline.events[2] = world.timeline.events[0];
    world.timeline.events[2].mapY = 2;

    CHECK(dm1_v1_group_stop_attacking_f0182_pc34(
              &world, 0, 0, 1, 1, &receipt),
          "F0182 accepts active C04 ownership and source square");
    CHECK(receipt.valid && receipt.removedReactionCount == 1 &&
              world.timeline.count == 2 && world.creatureAI[0].aspect[0] == 0 &&
              world.creatureAI[0].aspect[1] == 1 && world.creatureAI[0].aspect[3] == 0x7f,
          "F0182 atomically clears attack bits and exact C29-C41 events");
    world.creatureAI[0].reserved0 = 4;
    CHECK(!dm1_v1_group_stop_attacking_f0182_pc34(&world, 0, 0, 1, 1, &receipt) &&
              world.timeline.count == 2 && world.creatureAI[0].aspect[1] == 1,
          "bad C04 owner fails closed without another mutation");
    world.creatureAI[0].reserved0 = 0;

    CHECK(dm1_v1_group_square_distance_f0226_pc34(
              &world, 0, 0, 2, 2, 0, &receipt) && receipt.squareDistance == 4,
          "F0226 computes square distance only on loaded original coordinates");
    CHECK(!dm1_v1_group_square_distance_f0226_pc34(
               &world, 0, -1, 0, 2, 0, &receipt),
          "F0226 rejects synthetic coordinates");

    firstThings[0] = (unsigned short)make_thing(THING_TYPE_WEAPON, 0, 2);
    put_u16(weaponRaw, make_thing(THING_TYPE_WEAPON, 1, 1));
    put_u16(weaponRaw + 2, 27);
    put_u16(weaponRaw + 4, THING_ENDOFLIST);
    put_u16(weaponRaw + 6, 27);
    CHECK(dm1_v1_sensor_get_object_of_type_f0273_pc34(
              &world, 0, 1, 1, 1, 27, &receipt) && receipt.valid &&
              receipt.matchedThing == make_thing(THING_TYPE_WEAPON, 1, 1),
          "F0273 traverses raw SFT ownership and matches type/cell");
    CHECK(dm1_v1_sensor_get_object_of_type_f0273_pc34(
              &world, 0, 1, 1, -1, 27, &receipt) &&
              receipt.matchedThing == make_thing(THING_TYPE_WEAPON, 0, 2),
          "F0273 preserves raw chain order for CELL_ANY");
    put_u16(weaponRaw, make_thing(THING_TYPE_WEAPON, 7, 0));
    put_u16(weaponRaw + 2, 28);
    CHECK(!dm1_v1_sensor_get_object_of_type_f0273_pc34(
              &world, 0, 1, 1, -1, 27, &receipt),
          "F0273 rejects malformed raw Thing links");

    puts("PASS: DM1 F0182/F0226/F0273 source-owned group and sensor bundle");
    return 0;
}
