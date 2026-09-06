#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); return 1; } \
} while (0)

static int test_f0186_fresh_drop_pits(void)
{
    int variant;
    /* MOVESENS.C F0267:538 admits only open, non-imaginary pits for
     * non-levitating things. A fresh F0186 object has no source-square
     * removal; an unrelated weapon already on the pit must stay there. */
    for (variant = 0; variant < 3; ++variant) {
        struct GameWorld_Compat world = {0};
        struct DungeonDatState_Compat dungeon = {0};
        struct DungeonThings_Compat things = {0};
        struct DungeonMapDesc_Compat maps[2] = {{0}};
        struct DungeonMapTiles_Compat tiles[2] = {{0}};
        struct DungeonWeapon_Compat weapon = {0};
        struct DungeonJunk_Compat junk = {0};
        unsigned char squares[2] = {
            (DUNGEON_ELEMENT_PIT << 5) | DUNGEON_SQUARE_MASK_THING_LIST,
            DUNGEON_ELEMENT_CORRIDOR << 5};
        unsigned char rawWeapon[4] = {0xfe, 0xff, 1, 0};
        unsigned char rawJunk[4] = {0xff, 0xff, 0, 0};
        unsigned short columns[2] = {0, 1};
        unsigned short heads[2] = {THING_TYPE_WEAPON << 10, THING_NONE};
        unsigned short expectedDrop;
        struct RngState_Compat oracle;
        int i, sound = -1, falls = variant == 0;
        if (variant != 1) squares[0] |= 0x08;
        if (variant == 2) squares[0] |= 0x01;
        for (i = 0; i < 2; ++i) {
            maps[i].width = maps[i].height = 1;
            maps[i].level = (unsigned char)i;
            tiles[i].squareData = &squares[i];
            tiles[i].squareCount = 1;
        }
        dungeon.loaded = dungeon.tilesLoaded = 1;
        dungeon.header.mapCount = 2;
        dungeon.header.squareFirstThingCount = 2;
        dungeon.maps = maps; dungeon.tiles = tiles;
        dungeon.columnsCumulativeSquareFirstThingCount = columns;
        dungeon.dungeonColumnCount = 2;
        weapon.next = THING_ENDOFLIST; weapon.type = 1;
        junk.next = THING_NONE;
        things.loaded = 1;
        things.squareFirstThings = heads; things.squareFirstThingCount = 2;
        things.weapons = &weapon; things.weaponCount = 1;
        things.junks = &junk; things.junkCount = 1;
        things.thingCounts[THING_TYPE_WEAPON] = things.thingCounts[THING_TYPE_JUNK] = 1;
        things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
        things.rawThingData[THING_TYPE_JUNK] = rawJunk;
        world.dungeon = &dungeon; world.things = &things;
        world.partyMapIndex = 0; world.party.mapX = world.party.mapY = 5;
        world.masterRng.seed = 8;
        oracle = world.masterRng;
        expectedDrop = (unsigned short)((THING_TYPE_JUNK << 10) |
            (F0732_COMBAT_RngRandom_Compat(&oracle, 4) << 14));
        CHECK(F0186_GROUP_MaterializeFixedDropsOnWorld_Compat(
            &world, 15, 255, 0, 0, 0, &sound) == 1,
            "one real-format free C10 record materializes a worm round");
        CHECK(heads[0] == (THING_TYPE_WEAPON << 10),
            "fresh drop does not unlink the unrelated source-square owner");
        CHECK(junk.type == 34 && junk.next == THING_ENDOFLIST &&
            rawJunk[0] == 0xfe && rawJunk[1] == 0xff && rawJunk[2] == 34,
            "fresh worm round raw and decoded ownership agree");
        CHECK(weapon.next == (falls ? THING_ENDOFLIST : expectedDrop),
            "closed or imaginary pit retains the drop on its original level");
        CHECK((unsigned short)(rawWeapon[0] | (rawWeapon[1] << 8)) == weapon.next,
            "original weapon raw Next agrees after destination publication");
        CHECK(heads[1] == (falls ? expectedDrop : THING_NONE) &&
            !!(squares[1] & DUNGEON_SQUARE_MASK_THING_LIST) == falls,
            "only an open real pit publishes the original cell on the lower map");
        CHECK(columns[0] == 0 && columns[1] == 1,
            "compact column prefixes retain their actual square owners");
    }
    return 0;
}

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
    unsigned char raw_teleporter[6];
    struct F0267ThingMoveRequestPc34Compat request;
    struct F0267ThingMoveResultPc34Compat result;
    unsigned short weapon = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short projectile = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short teleporter = (unsigned short)(THING_TYPE_TELEPORTER << 10);
    int i;

    if (test_f0186_fresh_drop_pits()) return 1;

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
    memset(raw_teleporter, 0, sizeof(raw_teleporter));

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
    raw_teleporter[0] = 0xfe; raw_teleporter[1] = 0xff; /* next = THING_ENDOFLIST */
    raw_teleporter[2] = 0x21; raw_teleporter[3] = 0x40; /* fields: mapX=1,mapY=1,scope=2 */
    raw_teleporter[4] = 0x00; raw_teleporter[5] = 0x01; /* dest: targetMapIndex=1 */

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
    things.rawThingData[THING_TYPE_TELEPORTER] = raw_teleporter;
    things.thingCounts[THING_TYPE_TELEPORTER] = 1;
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
    raw_teleporter[2] = 0x22; raw_teleporter[3] = 0x44; /* mapX=2,mapY=1,rot=1,scope=2 */
    raw_teleporter[4] = 0x00; raw_teleporter[5] = 0x03; /* targetMapIndex=3 */
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
    raw_teleporter[2] = 0x22; raw_teleporter[3] = 0x20; /* mapX=2,mapY=1,rot=0,scope=1 */
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
