/* ReDMCSB PROJEXPL.C F0213/F0220 source-owner regression for M11 F0190. */

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_c15_layout_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static unsigned short explosion_thing(int index, int cell)
{
    return (unsigned short)(((unsigned short)cell << 14) |
                            ((unsigned short)THING_TYPE_EXPLOSION << 10) |
                            ((unsigned short)index & 0x03ffu));
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[1];
    struct DungeonExplosion_Compat explosions[1];
    unsigned char rawGroup[16];
    unsigned char squareData[1];
    unsigned short firstThings[1];
    unsigned char rawC15[4] = { 0xff, 0xff, 0xff, 0xff };
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat event;
    unsigned short expectedThing;
    uint32_t fingerprint;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(explosions, 0, sizeof(explosions));
    memset(rawGroup, 0, sizeof(rawGroup));
    memset(squareData, 0, sizeof(squareData));
    firstThings[0] = (unsigned short)(THING_TYPE_GROUP << 10);
    memset(&input, 0, sizeof(input));
    memset(&event, 0, sizeof(event));

    squareData[0] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                    DUNGEON_SQUARE_MASK_THING_LIST);
    map.width = 1;
    map.height = 1;
    tiles.squareData = squareData;
    tiles.squareCount = 1;
    dungeon.header.mapCount = 1;
    dungeon.header.squareFirstThingCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.dungeonColumnCount = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = (unsigned short[]){ 0 };
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;

    things.loaded = 1;
    groups[0].next = THING_ENDOFLIST;
    things.groups = groups;
    things.groupCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    rawGroup[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawGroup[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.rawThingData[THING_TYPE_GROUP] = rawGroup;
    things.explosions = explosions;
    things.explosionCount = 1;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = rawC15;
    things.squareFirstThings = firstThings;
    things.squareFirstThingCount = 1;

    world.dungeon = &dungeon;
    world.things = &things;
    world.gameTick = 20;
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = 18;
    input.mapIndex = 0;
    input.mapX = 0;
    input.mapY = 0;
    input.cell = 2;
    input.currentTick = 20;

    if (!F0887_ORCH_CreateSourceExplosion_Compat(&world, &input, 2)) {
        fprintf(stderr, "FAIL: F0190 source C15/C25 publication rejected\n");
        return 1;
    }
    expectedThing = explosion_thing(0, 2);
    fingerprint = dm1_v1_c15_layout_fingerprint_pc34(rawC15, 4u);
    event = world.timeline.events[0];
    if (world.explosions.count != 1 || firstThings[0] != (unsigned short)(THING_TYPE_GROUP << 10) ||
        F0512_DUNGEON_GetThingNext_Compat(&things, firstThings[0]) != expectedThing ||
        (rawC15[2] & 0x7fu) != C040_EXPLOSION_SMOKE || rawC15[3] != 18 ||
        event.kind != TIMELINE_EVENT_EXPLOSION_ADVANCE ||
        (uint32_t)event.aux3 != fingerprint ||
        world.explosions.entries[0].sourceC15Fingerprint != fingerprint) {
        fprintf(stderr, "FAIL: F0190 runtime/C15/C25 receipt mismatch\n");
        return 1;
    }
    puts("PASS dm1_v1_f0190_source_c15_m11_handoff_pc34_compat");
    return 0;
}
