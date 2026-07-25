#include "dm1_v1_projectile_impact_count_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void add_live_projectile(struct ProjectileList_Compat *projectiles,
                                int slot,
                                int mapIndex,
                                int mapX,
                                int mapY,
                                int cell)
{
    struct ProjectileInstance_Compat *projectile = &projectiles->entries[slot];

    projectile->slotIndex = slot;
    projectile->mapIndex = mapIndex;
    projectile->mapX = mapX;
    projectile->mapY = mapY;
    projectile->cell = cell;
}

static int test_authenticated_c14_chain_rejects_drift(void)
{
    struct DungeonDatState_Compat dungeon = {0};
    struct DungeonMapDesc_Compat map = {0};
    struct DungeonMapTiles_Compat tiles = {0};
    struct DungeonThings_Compat things = {0};
    struct DungeonProjectile_Compat source = {0};
    struct ProjectileList_Compat live = {0};
    unsigned char squares[1] = { (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST) };
    unsigned short sft[1] = { (unsigned short)(THING_TYPE_PROJECTILE << 10) };
    unsigned short columns[1] = { 0 };
    unsigned char raw[8] = { 0xfe, 0xff, 0x34, 0x20, 9, 7, 2, 0 };
    int count = -1;

    dungeon.tilesLoaded = 1; dungeon.header.mapCount = 1;
    dungeon.maps = &map; dungeon.tiles = &tiles;
    dungeon.columnsCumulativeSquareFirstThingCount = columns;
    dungeon.dungeonColumnCount = 1;
    map.width = map.height = 1; tiles.squareData = squares; tiles.squareCount = 1;
    things.loaded = 1; things.projectiles = &source; things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = raw;
    things.squareFirstThings = sft; things.squareFirstThingCount = 1;
    source.next = THING_ENDOFLIST; source.slot = 0x2034;
    source.kineticEnergy = 9; source.attack = 7; source.eventIndex = 2;
    live.entries[0].slotIndex = 0; live.entries[0].reserved3 = 1;
    live.entries[0].cell = 0;
    if (dm1_v1_f0218_count_authenticated_c14_at_cell_pc34(
            &dungeon, &things, &live, 0, 0, 0, 0, &count) != 1 || count != 1) {
        fprintf(stderr, "FAIL: F0218 authentic C14 chain was not admitted\n");
        return 1;
    }
    raw[5] ^= 1u;
    if (dm1_v1_f0218_count_authenticated_c14_at_cell_pc34(
            &dungeon, &things, &live, 0, 0, 0, 0, &count) != 0 || count != 0) {
        fprintf(stderr, "FAIL: F0218 accepted drifted C14 bytes\n");
        return 1;
    }
    return 0;
}

int main(void)
{
    struct ProjectileList_Compat projectiles;
    struct ProjectileList_Compat before;
    (void)before;

    memset(&projectiles, 0, sizeof(projectiles));
    for (int slot = 0; slot < PROJECTILE_LIST_CAPACITY; ++slot) {
        projectiles.entries[slot].slotIndex = -1;
    }

    add_live_projectile(&projectiles, 1, 2, 10, 11, 3);
    add_live_projectile(&projectiles, 17, 2, 10, 11, 3);
    add_live_projectile(&projectiles, 23, 2, 10, 11, 2);
    add_live_projectile(&projectiles, 31, 2, 10, 12, 3);
    add_live_projectile(&projectiles, 44, 3, 10, 11, 3);
    projectiles.count = 5;
    before = projectiles;

    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 2, 10, 11, 3) == 2);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 2, 10, 11, 2) == 1);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 3, 10, 11, 3) == 1);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(
               &projectiles, 2, 10, 11, 4) == 0);
    assert(F0218_PROJECTILE_GetImpactCount_Compat(NULL, 2, 10, 11, 3) == 0);
    assert(memcmp(&projectiles, &before, sizeof(projectiles)) == 0);
    if (test_authenticated_c14_chain_rejects_drift() != 0) return 1;

    puts("ok: DM1 F0218 live projectile impact inventory");
    return 0;
}
