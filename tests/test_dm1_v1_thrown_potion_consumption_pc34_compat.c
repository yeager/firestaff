#include "memory_tick_orchestrator_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned short make_thing(int type, int index)
{
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static unsigned char square_for_test(int elementType, int lowBits)
{
    return (unsigned char)(((elementType & 0x07) << 5) | (lowBits & 0x1f));
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[1];
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[1];
    struct DungeonMapTiles_Compat tiles[1];
    struct ProjectileCreateInput_Compat create;
    struct TimelineEvent_Compat firstMove;
    (void)firstMove;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    unsigned char squareData[12];
    unsigned char rawPotionData[4];
    unsigned short squareFirstThings[1];
    int projectileSlot = -1;
    (void)projectileSlot;
    int i;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(&potions, 0, sizeof(potions));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&maps, 0, sizeof(maps));
    memset(&tiles, 0, sizeof(tiles));
    memset(&create, 0, sizeof(create));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    memset(rawPotionData, 0, sizeof(rawPotionData));
    for (i = 0; i < 12; ++i) {
        squareData[i] = square_for_test(DUNGEON_ELEMENT_CORRIDOR, 0);
    }
    squareData[(1 * 3) + 1] = square_for_test(
        DUNGEON_ELEMENT_CORRIDOR, DUNGEON_SQUARE_MASK_THING_LIST);
    squareData[(2 * 3) + 1] = square_for_test(DUNGEON_ELEMENT_WALL, 0);
    squareFirstThings[0] = THING_ENDOFLIST;

    world.things = &things;
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.mapIndex = 0;
    world.party.mapX = 0;
    world.party.mapY = 0;
    world.partyMapIndex = 0;
    world.gameTick = 101;
    world.timeline.nowTick = 101;

    things.potions = potions;
    things.potionCount = 1;
    things.squareFirstThings = squareFirstThings;
    things.squareFirstThingCount = 1;
    things.rawThingData[THING_TYPE_POTION] = rawPotionData;
    things.thingCounts[THING_TYPE_POTION] = 1;
    potions[0].type = 3; /* ReDMCSB C03_POTION_VEN_POTION. */
    potions[0].power = 77;
    potions[0].next = THING_ENDOFLIST;
    rawPotionData[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawPotionData[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawPotionData[2] = potions[0].power;
    rawPotionData[3] = potions[0].type;

    dungeon.header.mapCount = 1;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    maps[0].width = 4;
    maps[0].height = 3;
    tiles[0].squareData = squareData;
    tiles[0].squareCount = 12;
    world.dungeon = &dungeon;

    create.category = PROJECTILE_CATEGORY_KINETIC;
    create.subtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
    create.ownerKind = PROJECTILE_OWNER_CHAMPION;
    create.ownerIndex = 0;
    create.mapIndex = 0;
    create.mapX = 1;
    create.mapY = 1;
    create.cell = 2;
    create.direction = 1;
    create.kineticEnergy = 80;
    create.attack = 40;
    create.stepEnergy = 10;
    create.currentTick = 100;
    create.firstMoveGraceFlag = 1;
    create.associatedThing = make_thing(THING_TYPE_POTION, 0);
    create.potionPower = potions[0].power;
    assert(F0810_PROJECTILE_Create_Compat(
        &create, &world.projectiles, &projectileSlot, &firstMove) == 1);
    assert(F0721_TIMELINE_Schedule_Compat(&world.timeline, &firstMove) == 1);

    assert(F0884_ORCH_AdvanceOneTick_Compat(&world, &input, &result) == ORCH_OK);
    assert(world.projectiles.count == 0);
    assert(world.explosions.count == 1);
    assert(squareFirstThings[0] == THING_ENDOFLIST);
    assert(potions[0].next == THING_NONE);
    assert(rawPotionData[0] == (unsigned char)(THING_NONE & 0xffu));
    assert(rawPotionData[1] == (unsigned char)(THING_NONE >> 8));

    puts("ok: DM1 F0328/F0811 thrown potion consumption");
    return 0;
}
