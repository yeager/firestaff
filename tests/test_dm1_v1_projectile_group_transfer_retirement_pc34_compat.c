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
    int partyLanding = argc == 2 && strcmp(argv[1], "party-landing") == 0;
    int peer = argc == 2 && strcmp(argv[1], "peer") == 0;
    int fluxcage = argc == 2 && strcmp(argv[1], "fluxcage") == 0;
    int burstLethal = argc == 2 && strcmp(argv[1], "burst-lethal") == 0;
    int burst = burstLethal || (argc == 2 && strcmp(argv[1], "burst") == 0);
    int fakeOpen = argc == 2 && strcmp(argv[1], "fake-open") == 0;
    int fakeImaginary = argc == 2 && strcmp(argv[1], "fake-imaginary") == 0;
    int fakeClosed = argc == 2 && strcmp(argv[1], "fake-closed") == 0;
    int stairsEntry = argc == 2 && strcmp(argv[1], "stairs-entry") == 0;
    int stairsPair = argc == 2 && strcmp(argv[1], "stairs-pair") == 0;
    int halfActive = argc == 2 && strcmp(argv[1], "half-active") == 0;
    int halfLethal = halfActive || (argc == 2 && strcmp(argv[1], "half-lethal") == 0);
    int halfSquare = halfLethal || (argc == 2 && strcmp(argv[1], "half-square") == 0);
    int sameSquare = grace || emptyCell || partyFirst || halfSquare || (argc == 2 && strcmp(argv[1], "same-square") == 0);
    struct GameWorld_Compat world;
    struct DungeonDatState_Compat dungeon = {0};
    struct DungeonThings_Compat things = {0};
    struct DungeonMapDesc_Compat map = {0};
    struct DungeonMapTiles_Compat tile = {0};
    struct DungeonGroup_Compat group = {0};
    struct DungeonWeapon_Compat weapon = {0};
    struct DungeonProjectile_Compat c14[2] = {{0}};
    struct DungeonExplosion_Compat c15 = {0};
    unsigned char rawC15[4] = {0};
    struct ProjectileCreateInput_Compat create = {0};
    struct TimelineEvent_Compat event;
    struct TickResult_Compat result;
    DM1_C14PoolReservationPc34 reservation;
    unsigned char squares[9], rawGroup[16] = {0}, rawWeapon[4] = {0}, rawC14[16] = {0};
    unsigned short columns[3] = {0,0,1}, sft[3] = {THING_TYPE_GROUP << 10, THING_NONE, THING_NONE};
    const unsigned short weaponThing = THING_TYPE_WEAPON << 10;
    int slot = -1;
    CHECK(F0881_WORLD_InitDefault_Compat(&world, 1));
    memset(squares, DUNGEON_ELEMENT_CORRIDOR << 5, sizeof(squares));
    squares[sameSquare ? 4 : 3] |= DUNGEON_SQUARE_MASK_THING_LIST;
    if (fakeOpen || fakeImaginary || fakeClosed) {
        squares[3] = (DUNGEON_ELEMENT_FAKEWALL << 5) |
            DUNGEON_SQUARE_MASK_THING_LIST | (fakeOpen ? 4 : fakeImaginary ? 1 : 0);
    }
    dungeon.loaded = dungeon.tilesLoaded = 1;
    if (stairsEntry || stairsPair) {
        squares[3] = (DUNGEON_ELEMENT_STAIRS << 5) | DUNGEON_SQUARE_MASK_THING_LIST;
        if (stairsPair) squares[4] = DUNGEON_ELEMENT_STAIRS << 5;
    }
    dungeon.header.mapCount = 1; dungeon.header.squareFirstThingCount = 3;
    dungeon.maps = &map; dungeon.tiles = &tile;
    dungeon.dungeonColumnCount = 3;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    map.width = map.height = 3;
    tile.squareData = squares; tile.squareCount = 9;
    group.next = group.slot = THING_ENDOFLIST;
    group.creatureType = 0; group.cells = emptyCell ? 1 : 255; group.count = 0;
    group.health[0] = 1000;
    if (halfSquare) {
        group.creatureType = 15;
        group.count = 1;
        group.cells = 0 | (2 << 2);
        group.health[1] = halfLethal ? 1 : 1000;
        if (halfActive) {
            group.health[0] = 1;
            group.health[1] = 1000;
        }
        rawGroup[4] = 15;
        word(rawGroup + 8, group.health[1]);
        rawGroup[14] = 1 << 5;
    }
    word(rawGroup, group.next); word(rawGroup + 2, group.slot);
    word(rawGroup + 6, group.health[0]);
    rawGroup[5] = group.cells;
    weapon.next = THING_ENDOFLIST; weapon.type = 8; /* Original sharp dagger. */
    word(rawWeapon, weapon.next); rawWeapon[2] = 8;
    c14[0].next = c14[1].next = THING_NONE;
    word(rawC14, THING_NONE); word(rawC14 + 8, THING_NONE);
    things.loaded = 1; things.squareFirstThings = sft; things.squareFirstThingCount = 3;
    things.groups = &group; things.groupCount = 1;
    things.weapons = &weapon; things.weaponCount = 1;
    things.projectiles = c14; things.projectileCount = peer ? 2 : 1;
    things.thingCounts[THING_TYPE_GROUP] = things.thingCounts[THING_TYPE_WEAPON] =
        things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_GROUP] = rawGroup;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawC14;
    if (peer) things.thingCounts[THING_TYPE_PROJECTILE] = 2;
    if (fluxcage) {
        c15.next = THING_ENDOFLIST;
        c15.type = 50;
        word(rawC15, THING_ENDOFLIST);
        word(rawC15 + 2, 50);
        things.explosions = &c15;
        things.explosionCount = things.thingCounts[THING_TYPE_EXPLOSION] = 1;
        things.rawThingData[THING_TYPE_EXPLOSION] = rawC15;
        sft[0] = THING_TYPE_EXPLOSION << 10;
    }
    world.dungeon = &dungeon; world.things = &things; world.ownsDungeon = 0;
    world.gameTick = 40; world.partyMapIndex = world.party.mapIndex = 0;
    world.newPartyMapIndex = -1; world.party.mapX = 2; world.party.mapY = 2;
    if (burst) {
        struct ExplosionCreateInput_Compat blast = {0};
        struct RngState_Compat expectedRng = world.masterRng;
        int attack, spread, expectedHp[2], i;
        group.creatureType = rawGroup[4] = 15;
        group.count = 1; rawGroup[14] = 1 << 5;
        group.cells = rawGroup[5] = 8;
        group.health[1] = 1000; word(rawGroup + 8, 1000);
        if (burstLethal) {
            group.health[0] = 1; word(rawGroup + 6, 1);
            world.creatureAICount = world.pc34ActiveGroupSourceCount = 1;
            world.creatureAI[0].reserved0 = 0;
            world.creatureAI[0].creatureType = 15;
            world.creatureAI[0].groupCells = 8;
            world.creatureAI[0].groupDirection = world.pc34ActiveGroupDirections[0] = 8;
            world.creatureAI[0].aspect[1] = 2;
        }
        /* F0213 attack 80, original C15 worm fire resistance 9; F0191
         * draws independently in descending slot order. */
        attack = 42 + F0732_COMBAT_RngRandom_Compat(&expectedRng, 41);
        attack -= F0732_COMBAT_RngRandom_Compat(&expectedRng, 19);
        spread = (attack >> 3) + 1;
        for (i = 1; i >= 0; --i)
            expectedHp[i] = 1000 - (attack - spread +
                F0732_COMBAT_RngRandom_Compat(&expectedRng, spread * 2));
        c15.next = THING_NONE;
        word(rawC15, THING_NONE);
        things.explosions = &c15;
        things.explosionCount = things.thingCounts[THING_TYPE_EXPLOSION] = 1;
        things.rawThingData[THING_TYPE_EXPLOSION] = rawC15;
        blast.explosionType = C000_EXPLOSION_FIREBALL;
        blast.attack = 80;
        blast.mapX = 1; blast.mapY = 0;
        blast.centered = 1; blast.cell = 255; blast.currentTick = 40;
        blast.creatorProjectileSlot = -1;
        CHECK(F0887_ORCH_CreateSourceExplosion_Compat(&world, &blast, 0));
        if (burstLethal) {
            CHECK(group.count == 0 && group.health[0] == expectedHp[1]);
            CHECK(group.cells == 10 && world.creatureAI[0].groupCells == 10);
            CHECK((world.pc34ActiveGroupDirections[0] & 3) == 2);
            CHECK(world.creatureAI[0].aspect[0] == 2);
            CHECK(readword(rawGroup + 6) == group.health[0]);
            puts("ok: source explosion compacts the active survivor");
            return 0;
        }
        CHECK(group.health[0] < 1000 && group.health[0] > 0);
        CHECK(readword(rawGroup + 6) == group.health[0]);
        CHECK(group.health[0] == expectedHp[0] && group.health[1] == expectedHp[1]);
        CHECK(readword(rawGroup + 8) == group.health[1]);
        CHECK(world.masterRng.seed == expectedRng.seed);
        puts("ok: source explosion group damage updates raw and decoded HP");
        return 0;
    }
    world.creatureAICount = 1;
    world.creatureAI[0].groupMapIndex = 0;
    world.creatureAI[0].groupMapX = 1;
    world.creatureAI[0].groupMapY = sameSquare ? 1 : 0;
    world.creatureAI[0].creatureType = 0;
    if (halfSquare) world.creatureAICount = 0; /* Source GROUP needs no AI row. */
    if (halfActive) {
        world.creatureAICount = 1;
        world.creatureAI[0].reserved0 = 0;
        world.creatureAI[0].creatureType = group.creatureType;
        world.creatureAI[0].groupCells = group.cells;
        world.creatureAI[0].groupDirection = 8; /* slot zero north, slot one south */
        world.pc34ActiveGroupSourceCount = 1;
        world.pc34ActiveGroupDirections[0] = 8;
        world.creatureAI[0].aspect[0] = 1;
        world.creatureAI[0].aspect[1] = 2;
    }
    if (partyFirst || partyLanding) {
        world.party.mapX = 1;
        world.party.mapY = partyLanding ? 0 : 1;
        world.party.championCount = 1;
        world.party.champions[0].present = 1;
        world.party.champions[0].cell = partyLanding ? 3 : 0;
        world.party.champions[0].hp.current = world.party.champions[0].hp.maximum = 1000;
    }
    create.category = PROJECTILE_CATEGORY_KINETIC;
    create.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    create.ownerKind = PROJECTILE_OWNER_CHAMPION;
    create.mapIndex = 0; create.mapX = create.mapY = 1;
    create.cell = halfSquare && !halfActive ? 1 : 0; create.direction = 0;
    create.kineticEnergy = 40; create.attack = 30; create.stepEnergy = 1;
    create.currentTick = 40; create.firstMoveGraceFlag = grace;
    create.attackTypeCode = COMBAT_ATTACK_NORMAL;
    CHECK(F0810_PROJECTILE_Create_Compat(&create, &world.projectiles, &slot, &event));
    CHECK(slot == 0);
    world.projectiles.entries[slot].reserved1 = weaponThing;
    CHECK(dm1_v1_c14_pool_reserve_pc34(&things, &reservation));
    CHECK(dm1_v1_c14_pool_initialize_and_link_pc34(&reservation, &dungeon,
              weaponThing, 40, 30, 0, create.cell, 0, 1, 1));
    CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &event));
    if (peer) {
        int peerSlot = -1;
        struct TimelineEvent_Compat peerEvent;
        create.category = PROJECTILE_CATEGORY_MAGICAL;
        create.subtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
        create.currentTick += 10; /* Leave the second real C14 pending. */
        CHECK(F0810_PROJECTILE_Create_Compat(&create, &world.projectiles, &peerSlot, &peerEvent));
        CHECK(peerSlot == 1);
        CHECK(dm1_v1_c14_pool_reserve_pc34(&things, &reservation));
        CHECK(dm1_v1_c14_pool_initialize_and_link_pc34(&reservation, &dungeon,
                  0xff83, 40, 30, 1, create.cell, 0, 1, 1));
        CHECK(F0721_TIMELINE_Schedule_Compat(&world.timeline, &peerEvent));
    }
    world.gameTick = event.fireAtTick;
    memset(&result, 0, sizeof(result));
    CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) >= 1);
    if (fluxcage) {
        CHECK(world.projectiles.entries[0].reserved3 != 0);
        CHECK(world.projectiles.entries[0].mapY == 0);
        CHECK(c14[0].next != THING_NONE);
        CHECK(c15.type == 50 && c15.next != THING_NONE);
        puts("ok: source projectile passes through Fluxcage");
        return 0;
    }
    if (stairsEntry || stairsPair) {
        /* PROJEXPL.C F0219:723 blocks only stairs -> stairs. */
        CHECK((world.projectiles.entries[0].reserved3 != 0) == !stairsPair);
        if (stairsEntry) {
            CHECK(world.projectiles.entries[0].mapY == 0);
            CHECK(world.projectiles.entries[0].mapIndex == 0);
        } else {
            CHECK(c14[0].next == THING_NONE && readword(rawC14) == THING_NONE);
        }
        CHECK(group.health[0] == 1000);
        puts("ok: source projectile stair boundary matches F0219");
        return 0;
    }
    if (fakeOpen || fakeImaginary || fakeClosed) {
        CHECK((world.projectiles.entries[0].reserved3 != 0) == !fakeClosed);
        if (!fakeClosed) CHECK(world.projectiles.entries[0].mapY == 0);
        CHECK(group.health[0] == 1000);
        puts("ok: fake wall flags control source projectile passage");
        return 0;
    }
    if (peer) {
        CHECK(world.projectiles.entries[0].reserved3 != 0);
        CHECK(world.projectiles.entries[0].mapY == 0);
        CHECK(world.projectiles.entries[1].reserved3 != 0);
        CHECK(c14[0].next != THING_NONE && c14[1].next != THING_NONE);
        puts("ok: co-located source projectiles do not collide with each other");
        return 0;
    }
    if (!sameSquare) {
        int i, found = 0;
        CHECK(group.health[0] == 1000 && group.slot == THING_ENDOFLIST);
        CHECK(world.projectiles.entries[slot].reserved3 != 0);
        CHECK(world.projectiles.entries[slot].mapX == 1);
        CHECK(world.projectiles.entries[slot].mapY == 0);
        CHECK(world.projectiles.entries[slot].cell == 3);
        if (partyLanding) CHECK(world.party.champions[0].hp.current == 1000);
        /* F0219 checks the landed cell on its next event, independently
         * of active AI admission. */
        world.creatureAICount = 0;
        for (i = 0; i < world.timeline.count; ++i) {
            if (world.timeline.events[i].kind == TIMELINE_EVENT_PROJECTILE_MOVE) {
                world.gameTick = world.timeline.events[i].fireAtTick;
                found = 1;
                break;
            }
        }
        CHECK(found);
        memset(&result, 0, sizeof(result));
        CHECK(F0887_ORCH_DispatchTimelineEvents_Compat(&world, &result) >= 1);
    }
    if (halfSquare) {
        CHECK(group.health[0] == 1000);
        if (halfLethal) {
            /* GROUP.C F0190:892-904 shifts only the surviving fields,
             * masks bits 6-7 and does not recenter a lone survivor. */
            CHECK(group.cells == (halfActive ? 10 : 8));
            CHECK(rawGroup[5] == group.cells);
            if (halfActive) {
                CHECK(world.creatureAI[0].groupCells == group.cells);
                CHECK((world.pc34ActiveGroupDirections[0] & 3) == 2);
                CHECK((world.creatureAI[0].groupDirection & 3) == 2);
                CHECK(world.creatureAI[0].aspect[0] == 2);
            }
            CHECK(group.count == 0);
            CHECK(((rawGroup[14] >> 5) & 3) == 0);
            CHECK(readword(rawGroup + 6) == 1000);
            CHECK(c14[0].next == THING_NONE && readword(rawC14) == THING_NONE);
            puts("ok: lethal half-square hit preserves the other creature");
            return 0;
        }
        CHECK(group.health[1] > 0 && group.health[1] < 1000);
        CHECK(group.slot == THING_ENDOFLIST);
        CHECK(c14[0].next == THING_NONE && readword(rawC14) == THING_NONE);
        CHECK((group.next & 0x3fff) == weaponThing);
        puts("ok: half-square footprint hits the second worm without an active AI row");
        return 0;
    }
    if (partyFirst || partyLanding) {
        CHECK(world.party.champions[0].hp.current < 1000);
        CHECK(group.health[0] == 1000 && group.slot == THING_ENDOFLIST);
        CHECK(c14[0].next == THING_NONE && readword(rawC14) == THING_NONE);
        CHECK((group.next & 0x3fff) == weaponThing);
        CHECK(weapon.next == THING_ENDOFLIST);
        puts("ok: source-cell champion impact precedes the co-located group");
        return 0;
    }
    if (grace || emptyCell) {
        CHECK(group.health[0] == 1000 && group.slot == THING_ENDOFLIST);
        CHECK(world.projectiles.entries[slot].reserved3 != 0);
        CHECK(c14[0].next != THING_NONE && readword(rawC14) != THING_NONE);
        CHECK(world.projectiles.entries[slot].firstMoveGraceFlag == 0);
        puts("ok: grace or empty source cell does not create a false group hit");
        return 0;
    }
    CHECK(group.health[0] > 0 && group.health[0] < 1000);
    CHECK((group.slot & 0x3fff) == weaponThing);
    CHECK(readword(rawGroup + 2) == group.slot);
    CHECK(weapon.next == THING_ENDOFLIST && readword(rawWeapon) == THING_ENDOFLIST);
    CHECK(c14[0].next == THING_NONE && readword(rawC14) == THING_NONE);
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
