#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); return 1; } \
} while (0)

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct DungeonMapDesc_Compat maps[4];
    struct DungeonMapTiles_Compat tiles[4];
    unsigned char square_data[4][9];
    unsigned short first_things[36];
    struct DungeonWeapon_Compat weapons[1];
    struct DungeonProjectile_Compat projectiles[1];
    struct DungeonTeleporter_Compat teleporters[1];
    unsigned char raw_weapon[4];
    unsigned char raw_projectile[8];
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat result;
    unsigned short weapon = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short projectile = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short teleporter = (unsigned short)(THING_TYPE_TELEPORTER << 10);
    int i;

    memset(&world, 0, sizeof(world));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(square_data, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(square_data));
    for (i = 0; i < 36; ++i) first_things[i] = THING_ENDOFLIST;
    memset(weapons, 0, sizeof(weapons));
    memset(projectiles, 0, sizeof(projectiles));
    memset(teleporters, 0, sizeof(teleporters));
    memset(raw_weapon, 0, sizeof(raw_weapon));
    memset(raw_projectile, 0, sizeof(raw_projectile));

    for (i = 0; i < 4; ++i) {
        maps[i].width = 3;
        maps[i].height = 3;
        maps[i].level = (unsigned char)i;
        tiles[i].squareData = square_data[i];
        tiles[i].squareCount = 9;
    }
    /* destination -> open teleporter -> open pit -> descending stairs */
    square_data[0][1 * 3 + 0] = (DUNGEON_ELEMENT_TELEPORTER << 5) | 0x18;
    square_data[1][1 * 3 + 1] = (DUNGEON_ELEMENT_PIT << 5) | 0x08;
    square_data[2][1 * 3 + 1] = (DUNGEON_ELEMENT_STAIRS << 5) | 0x10;
    /* DUNGEON.DAT stores FirstThing entries only for squares carrying the
     * thing-list flag; this fixture must use the same packed ordering. */
    square_data[0][0 * 3 + 0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    square_data[0][1 * 3 + 0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    square_data[3][2 * 3 + 1] |= DUNGEON_SQUARE_MASK_THING_LIST;
    first_things[0] = weapon;
    first_things[1] = teleporter;
    weapons[0].next = THING_ENDOFLIST;
    weapons[0].type = 1;
    teleporters[0].next = THING_ENDOFLIST;
    teleporters[0].scope = 2;
    teleporters[0].targetMapIndex = 1;
    teleporters[0].targetMapX = 1;
    teleporters[0].targetMapY = 1;
    raw_weapon[0] = 0xfe;
    raw_weapon[1] = 0xff;
    raw_weapon[2] = 1;

    dungeon.header.mapCount = 4;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.tilesLoaded = 1;
    things.squareFirstThings = first_things;
    things.squareFirstThingCount = 36;
    things.weapons = weapons;
    things.weaponCount = 1;
    things.projectiles = projectiles;
    things.projectileCount = 1;
    things.teleporters = teleporters;
    things.teleporterCount = 1;
    things.rawThingData[THING_TYPE_WEAPON] = raw_weapon;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = raw_projectile;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.loaded = 1;
    world.dungeon = &dungeon;
    world.things = &things;

    memset(&request, 0, sizeof(request));
    request.thing = weapon;
    request.sourceMapIndex = 0;
    request.sourceMapX = 0;
    request.sourceMapY = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    request.destinationMapY = 0;
    memset(&result, 0, sizeof(result));
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "F0267 moves the loaded ordinary Thing through the special-square chain");
    if (result.teleporterChainCount != 1 || result.pitChainCount != 1 ||
        result.stairsChainCount != 1) {
        fprintf(stderr, "chain counts: teleporter=%d pit=%d stairs=%d\n",
                result.teleporterChainCount, result.pitChainCount,
                result.stairsChainCount);
        return 1;
    }
    CHECK(result.finalMapIndex == 3 && result.finalMapX == 2 && result.finalMapY == 1,
          "F0267 lands after the source F0154/F0155 stairs exit transform");
    CHECK(first_things[0] == THING_ENDOFLIST && result.sourceUnlinked &&
          result.destinationLinked,
          "F0267 unlinks the source and links the final destination only");
    CHECK(raw_weapon[0] == 0xfe && raw_weapon[1] == 0xff,
          "F0267 persists the terminal raw Generic.Next word after relink");

    /* C14 is levitating, but F0267 still applies its relative teleporter
     * cell rotation before it appends to the target's live list. */
    teleporters[0].targetMapIndex = 3;
    teleporters[0].targetMapX = 2;
    teleporters[0].targetMapY = 1;
    teleporters[0].rotation = 1;
    teleporters[0].absoluteRotation = 0;
    teleporters[0].scope = 2;
    projectile |= (unsigned short)(0u << 14);
    first_things[0] = projectile;
    projectiles[0].next = THING_ENDOFLIST;
    raw_projectile[0] = 0xfe;
    raw_projectile[1] = 0xff;
    memset(&request, 0, sizeof(request));
    request.thing = projectile;
    request.sourceMapIndex = 0;
    request.sourceMapX = 0;
    request.sourceMapY = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    request.destinationMapY = 0;
    memset(&result, 0, sizeof(result));
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "F0267 moves a loaded C14 projectile through an object-scope teleporter");
    CHECK(result.levitates && result.teleporterChainCount == 1 &&
          result.pitChainCount == 0 && result.stairsChainCount == 0,
          "C14 rotates through teleporters but remains above pits and stairs");
    if (!(result.finalThing == (unsigned short)(projectile | (1u << 14)) &&
          weapons[0].next == result.finalThing &&
          raw_weapon[0] == (unsigned char)(result.finalThing & 0xffu) &&
          raw_weapon[1] == (unsigned char)(result.finalThing >> 8))) {
        fprintf(stderr, "C14 link: final=%04x next=%04x raw=%02x%02x\n",
                result.finalThing, weapons[0].next, raw_weapon[1], raw_weapon[0]);
        return 1;
    }

    /* A creature-only teleporter is not a non-group route. F0267 leaves
     * the ordinary object on that square instead of following its target. */
    teleporters[0].scope = 1;
    teleporters[0].rotation = 0;
    first_things[0] = weapon;
    weapons[0].next = THING_ENDOFLIST;
    raw_weapon[0] = 0xfe;
    raw_weapon[1] = 0xff;
    memset(&request, 0, sizeof(request));
    request.thing = weapon;
    request.sourceMapIndex = 0;
    request.sourceMapX = 0;
    request.sourceMapY = 0;
    request.destinationMapIndex = 0;
    request.destinationMapX = 1;
    request.destinationMapY = 0;
    memset(&result, 0, sizeof(result));
    CHECK(F0267_MOVE_MoveThingOnLoadedChain_Compat(&world, &request, &result),
          "F0267 moves an ordinary Thing onto a creature-only teleporter square");
    CHECK(result.teleporterChainCount == 0 && result.finalMapIndex == 0 &&
          result.finalMapX == 1 && result.finalMapY == 0,
          "creature-only teleporters do not transport objects or C14 projectiles");
    puts("ok: F0267 object/C14 chains apply scope, rotation, levitation, and raw links");
    return 0;
}
