#include "dm1_v1_c15_layout_pc34_compat.h"

#include <string.h>

#define REQUIRE(condition) do { if (!(condition)) return 1; } while (0)

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonExplosion_Compat explosions[2];
    unsigned char square[1] = { (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST };
    unsigned short square_first_things[1];
    unsigned short column_sft_base[1] = { 0 };
    unsigned char raw[8] = {
        0xfe, 0xff, 0x92, 0x31,
        0xff, 0xff, 0x47, 0xa5
    };
    unsigned char original_raw[sizeof(raw)];
    struct DungeonExplosion_Compat original_explosions[2];
    DM1_C15PoolReservationPc34 reservation;
    DM1_C15C25PublicationReceiptPc34 receipt;
    int has_fluxcage = 0;

    memset(&things, 0, sizeof(things));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    memset(explosions, 0, sizeof(explosions));
    explosions[0].next = THING_ENDOFLIST;
    explosions[0].type = 0x12;
    explosions[0].centered = 1;
    explosions[0].attack = 0x31;
    explosions[1].next = THING_NONE;
    explosions[1].type = 0x47;
    explosions[1].centered = 1;
    explosions[1].attack = 0xa5;
    memcpy(original_raw, raw, sizeof(raw));
    memcpy(original_explosions, explosions, sizeof(explosions));

    things.loaded = 1;
    things.explosions = explosions;
    things.explosionCount = 2;
    things.squareFirstThings = square_first_things;
    things.squareFirstThingCount = 1;
    things.rawThingData[THING_TYPE_EXPLOSION] = raw;
    things.thingCounts[THING_TYPE_EXPLOSION] = 2;

    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.columnsCumulativeSquareFirstThingCount = column_sft_base;
    dungeon.dungeonColumnCount = 1;
    map.width = 1;
    map.height = 1;
    tiles.squareData = square;
    tiles.squareCount = 1;
    square_first_things[0] = (unsigned short)(THING_TYPE_EXPLOSION << 10);

    REQUIRE(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) == 1);
    REQUIRE(reservation.active == 1);
    REQUIRE(reservation.things == &things);
    REQUIRE(reservation.thing == (unsigned short)((THING_TYPE_EXPLOSION << 10) | 1));
    REQUIRE(raw[4] == 0xfe && raw[5] == 0xff);
    REQUIRE(raw[6] == 0 && raw[7] == 0);
    REQUIRE(explosions[1].next == THING_ENDOFLIST);
    REQUIRE(explosions[1].type == 0 && explosions[1].attack == 0);
    REQUIRE(dm1_v1_c15_pool_rollback_pc34(&reservation) == 1);
    REQUIRE(reservation.active == 0);
    REQUIRE(memcmp(raw, original_raw, sizeof(raw)) == 0);
    REQUIRE(memcmp(explosions, original_explosions, sizeof(explosions)) == 0);
    REQUIRE(dm1_v1_c15_pool_rollback_pc34(&reservation) == 0);

    REQUIRE(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) == 1);
    REQUIRE(dm1_v1_c15_c25_publish_pc34(
        &reservation, &dungeon, 0x35, 0x78, 1, 2, 0, 0, 0,
        0x00123456u, 0x4d, &receipt) == 1);
    REQUIRE(reservation.thing ==
        (unsigned short)((2u << 14) | (THING_TYPE_EXPLOSION << 10) | 1));
    REQUIRE(raw[4] == 0xfe && raw[5] == 0xff);
    REQUIRE(raw[6] == 0xb5 && raw[7] == 0x78);
    REQUIRE(explosions[1].next == THING_ENDOFLIST);
    REQUIRE(explosions[1].type == 0x35 && explosions[1].centered == 1 &&
        explosions[1].attack == 0x78);
    REQUIRE(square_first_things[0] == (unsigned short)(THING_TYPE_EXPLOSION << 10));
    REQUIRE(raw[0] == (unsigned char)(reservation.thing & 0xffu) &&
        raw[1] == (unsigned char)(reservation.thing >> 8));
    REQUIRE(receipt.active == 1 && receipt.mapTime == 0x00123456u &&
        receipt.mapX == 0 && receipt.mapY == 0 && receipt.priority == 0x4d &&
        receipt.slot == reservation.thing);
    REQUIRE(dm1_v1_c15_c25_receipt_is_live_pc34(&receipt, &dungeon, &things) == 1);
    explosions[0].type = 50;
    raw[2] = (unsigned char)(50 | 0x80u);
    REQUIRE(dm1_v1_f0221_fluxcage_on_square_pc34(
        &dungeon, &things, 0, 0, 0, &has_fluxcage) == 1);
    REQUIRE(has_fluxcage == 1);
    raw[3] ^= 1u;
    REQUIRE(dm1_v1_f0221_fluxcage_on_square_pc34(
        &dungeon, &things, 0, 0, 0, &has_fluxcage) == 0);
    raw[3] ^= 1u;
    raw[2] = original_raw[2];
    explosions[0].type = original_explosions[0].type;
    raw[7] ^= 1u;
    REQUIRE(dm1_v1_c15_c25_receipt_is_live_pc34(&receipt, &dungeon, &things) == 0);
    raw[7] ^= 1u;
    receipt.mapX = 1;
    REQUIRE(dm1_v1_c15_c25_receipt_is_live_pc34(&receipt, &dungeon, &things) == 0);
    receipt.mapX = 0;
    REQUIRE(dm1_v1_c15_pool_rollback_pc34(&reservation) == 1);
    REQUIRE(square_first_things[0] == (unsigned short)(THING_TYPE_EXPLOSION << 10));
    REQUIRE(memcmp(raw, original_raw, sizeof(raw)) == 0);
    REQUIRE(memcmp(explosions, original_explosions, sizeof(explosions)) == 0);

    REQUIRE(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) == 1);
    memset(&receipt, 0xa5, sizeof(receipt));
    REQUIRE(dm1_v1_c15_c25_publish_pc34(
        &reservation, &dungeon, 0x35, 0x78, 1, 2, 0, 1, 0,
        0x00123456u, 0x4d, &receipt) == 0);
    REQUIRE(reservation.active == 0);
    REQUIRE(receipt.active == 0);
    REQUIRE(square_first_things[0] == (unsigned short)(THING_TYPE_EXPLOSION << 10));
    REQUIRE(memcmp(raw, original_raw, sizeof(raw)) == 0);
    REQUIRE(memcmp(explosions, original_explosions, sizeof(explosions)) == 0);

    raw[4] = 0xfe;
    raw[5] = 0xff;
    things.explosions = NULL;
    REQUIRE(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) == 0);
    REQUIRE(raw[4] == 0xfe && raw[5] == 0xff);
    things.explosions = explosions;
    things.explosionCount = 1;
    REQUIRE(dm1_v1_c15_pool_reserve_pc34(&things, &reservation) == 0);
    REQUIRE(raw[4] == 0xfe && raw[5] == 0xff);
    return 0;
}
