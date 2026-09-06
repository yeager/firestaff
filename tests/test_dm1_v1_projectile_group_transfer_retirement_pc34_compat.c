#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_c14_layout_pc34_compat.h"
#include <stdio.h>
#include <string.h>

/* Source-shaped RAM fixture for F0217 -> F0215 ownership, not game media.
 * The surviving C00 creature keeps a sharp dagger; its carrier C14 must
 * still be unlinked and freed. PROJEXPL.C:540-553,607-608,248-256. */
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; \
} } while (0)
static void word(unsigned char *p, unsigned short value)
{ p[0] = (unsigned char)value; p[1] = (unsigned char)(value >> 8); }
static unsigned short readword(const unsigned char *p)
{ return (unsigned short)(p[0] | ((unsigned short)p[1] << 8)); }

int main(int argc, char** argv)
{
    int grace = argc == 2 && strcmp(argv[1], "grace") == 0;
    int emptyCell = argc == 2 && strcmp(argv[1], "empty-cell") == 0;
    int partyFirst = argc == 2 && strcmp(argv[1], "party-first") == 0;
    int sameSquare = grace || emptyCell || partyFirst || (argc == 2 && strcmp(argv[1], "same-square") == 0);
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon = {0};
    struct DungeonThings_Compat things = {0};
    struct DungeonMapDesc_Compat map = {0};
    struct DungeonMapTiles_Compat tile = {0};
    struct DungeonGroup_Compat group = {0};
    struct DungeonWeapon_Compat weapon = {0};
    struct DungeonProjectile_Compat c14 = {0};
    struct ProjectileCreateInput_Compat create = {0};
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    DM1_C14PoolReservationPc34 reservation;
    unsigned char squares[9], rawGroup[16] = {0}, rawWeapon[4] = {0}, rawC14[8] = {0};
    unsigned short columns[3] = {0,0,1}, sft[3] = {THING_TYPE_GROUP << 10, THING_NONE, THING_NONE};
    const unsigned short weaponThing = THING_TYPE_WEAPON << 10;
    int slot = -1;
    CHECK(F0881_WORLD_InitDefault_Compat(&world, 1));
    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squares));
    squares[sameSquare ? 4 : 3] |= DUNGEON_SQUARE_MASK_THING_LIST;
    dungeon.loaded = dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1; dungeon.header.squareFirstThingCount = 3;
    dungeon.maps = &map; dungeon.tiles = &tile;
    dungeon.dungeonColumnCount = 3;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    map.width = map.height = 3;
    tile.squareData = squares; tile.squareCount = 9;
    group.next = group.slot = THING_ENDOFLIST;
    group.creatureType = 0; group.cells = emptyCell ? 1 : 255; group.count = 0;
    group.health[0] = 1000;
    word(rawGroup, group.next); word(rawGroup + 2, group.slot);
    word(rawGroup + 6, group.health[0]);
    rawGroup[5] = group.cells;
    weapon.next = THING_ENDOFLIST; weapon.type = 8; /* Original sharp dagger. */
    word(rawWeapon, weapon.next); rawWeapon[2] = 8;
    c14.next = THING_NONE; word(rawC14, THING_NONE);
    things.loaded = 1; things.squareFirstThings = sft; things.squareFirstThingCount = 3;
    things.groups = &group; things.groupCount = 1;
    things.weapons = &weapon; things.weaponCount = 1;
    things.projectiles = &c14; things.projectileCount = 1;
    things.thingCounts[THING_TYPE_GROUP] = things.thingCounts[THING_TYPE_WEAPON] =
        things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawGroup;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawC14;
    world.dungeon = &dungeon; world.things = &things; world.ownsDungeon = 0;
    world.gameTick = 40; world.partyMapIndex = world.party.mapIndex = 0;
    world.newPartyMapIndex = -1; world.party.mapX = 2; world.party.mapY = 2;
    world.creatureAICount = 1;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = sameSquare ? 1 : 0;
    world.creatureAI[0].creatureType = 0;
    if (partyFirst) {
        world.party.mapX = world.party.mapY = 1;
        world.party.championCount = 1;
        world.party.champions[0].present = 1;
        world.party.champions[0].cell = 0;
        world.party.champions[0].hp.current = world.party.champions[0].hp.maximum = 1000;
    }
    create.category = PROJECTILE_CATEGORY_KINETIC;
    create.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    create.ownerKind = PROJECTILE_OWNER_CHAMPION;
    create.mapIndex = 0; create.mapX = create.mapY = 1;
    create.cell = 0; create.direction = 0;
    create.kineticEnergy = 40; create.attack = 30; create.stepEnergy = 1;
    create.currentTick = 40; create.firstMoveGraceFlag = grace;
    create.attackTypeCode = COMBAT_ATTACK_NORMAL;
    CHECK(F0810_PROJECTILE_Create_Compat(&create, &world.projectiles, &slot, &event));
    CHECK(slot == 0);
    world.projectiles.entries[slot].reserved1 = weaponThing;
    CHECK(dm1_v1_c14_pool_reserve_pc34(&things, &reservation));
    CHECK(dm1_v1_c14_pool_initialize_and_link_pc34(&reservation, &dungeon,
              weaponThing, 40, 30, 0, 0, 0, 1, 1));
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event));
    world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) >= 1);
    if (partyFirst) {
        CHECK(world.party.champions[0].hp.current < 1000);
        CHECK(group.health[0] == 1000 && group.slot == THING_ENDOFLIST);
        CHECK(c14.next == THING_NONE && readword(rawC14) == THING_NONE);
        CHECK((group.next & 0x3fff) == weaponThing);
        CHECK(weapon.next == THING_ENDOFLIST);
        puts("ok: source-cell champion impact precedes the co-located group");
        return 0;
    }
    if (grace || emptyCell) {
        CHECK(group.health[0] == 1000 && group.slot == THING_ENDOFLIST);
        CHECK(world.projectiles.entries[slot].reserved3 != 0);
        CHECK(c14.next != THING_NONE && readword(rawC14) != THING_NONE);
        CHECK(world.projectiles.entries[slot].firstMoveGraceFlag == 0);
        puts("ok: grace or empty source cell does not create a false group hit");
        return 0;
    }
    CHECK(group.health[0] > 0 && group.health[0] < 1000);
    CHECK((group.slot & 0x3fff) == weaponThing);
    CHECK(readword(rawGroup + 2) == group.slot);
    CHECK(weapon.next == THING_ENDOFLIST && readword(rawWeapon) == THING_ENDOFLIST);
    CHECK(c14.next == THING_NONE && readword(rawC14) == THING_NONE);
    CHECK(world.projectiles.entries[slot].reserved3 == 0);
    CHECK(sft[0] == (THING_TYPE_GROUP << 10));
    CHECK(group.next == THING_ENDOFLIST && readword(rawGroup) == THING_ENDOFLIST);
    CHECK(sft[1] == THING_NONE);
    CHECK(sft[2] == THING_NONE);
    CHECK(((squares[4] & DUNGEON_SQUARE_MASK_THING_LIST) != 0) == sameSquare);
    CHECK(columns[0] == 0 && columns[1] == 0 && columns[2] == 1);
    puts("ok: retained sharp weapon has one group owner and retired C14 carrier");
    return 0;
}
