#include "dm2_v1_dungeon_room_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void test_set_tile_attribute_02(void)
{
    uint8_t attrs[4] = {0x00u, 0x04u, 0x02u, 0x80u};
    DM2_V1_DungeonRoomReceipt receipt;

    expect_true(dm2_v1_SET_TILE_ATTRIBUTE_02(attrs, 4u, 1u,
                                             &receipt) == 1,
                "SET_TILE_ATTRIBUTE_02 sets bit on selected tile");
    expect_true(attrs[1] == 0x06u && receipt.valid && receipt.mutated &&
                    receipt.attribute_before == 0x04u &&
                    receipt.attribute_after == 0x06u &&
                    strcmp(receipt.symbol, "SET_TILE_ATTRIBUTE_02") == 0,
                "tile attribute receipt records mutation");

    expect_true(dm2_v1_SET_TILE_ATTRIBUTE_02(attrs, 4u, 2u,
                                             &receipt) == 1,
                "SET_TILE_ATTRIBUTE_02 accepts already-set bit");
    expect_true(attrs[2] == 0x02u && !receipt.mutated,
                "already-set tile attribute is stable");

    expect_true(dm2_v1_SET_TILE_ATTRIBUTE_02(attrs, 4u, 9u,
                                             &receipt) == 0,
                "SET_TILE_ATTRIBUTE_02 blocks out-of-range tile");
    expect_true(receipt.blocked && !receipt.valid,
                "out-of-range tile attribute write is fail-closed");
}

static void test_summarize_stone_room(void)
{
    DM2_V1_StoneRoomTile tiles[5] = {
        {0u, DM2_V1_TILE_ATTRIBUTE_02, 1u, 0u, 0u},
        {1u, 0u, 0u, 1u, 0u},
        {1u, DM2_V1_TILE_ATTRIBUTE_02, 0u, 0u, 1u},
        {0u, 0u, 1u, 0u, 1u},
        {2u, 0u, 0u, 0u, 0u}
    };
    DM2_V1_StoneRoomSummary summary;
    DM2_V1_DungeonRoomReceipt receipt;

    expect_true(dm2_v1_SUMMARIZE_STONE_ROOM(tiles, 5u, &summary,
                                            &receipt) == 1,
                "SUMMARIZE_STONE_ROOM summarizes bounded tile facts");
    expect_true(summary.valid && summary.tile_count == 5u &&
                    summary.wall_count == 2u && summary.floor_count == 3u &&
                    summary.door_count == 1u && summary.alcove_count == 2u &&
                    summary.attribute02_count == 2u &&
                    summary.ornament_count == 2u &&
                    summary.summary_hash != 0u,
                "stone room summary counts walls, floor and ornaments");
    expect_true(receipt.valid && receipt.blocked &&
                    strcmp(receipt.symbol, "SUMMARIZE_STONE_ROOM") == 0,
                "stone room receipt blocks unproven decoration stage");

    expect_true(dm2_v1_SUMMARIZE_STONE_ROOM(tiles, 0u, &summary,
                                            &receipt) == 0,
                "SUMMARIZE_STONE_ROOM blocks empty room");
    expect_true(receipt.blocked && !receipt.valid,
                "empty stone room summary is fail-closed");
}

int main(void)
{
    test_set_tile_attribute_02();
    test_summarize_stone_room();
    expect_true(strstr(dm2_v1_dungeon_room_helpers_source_evidence(),
                       "SET_TILE_ATTRIBUTE_02:3050") != 0,
                "source evidence includes tile attribute symbol");
    expect_true(strstr(dm2_v1_dungeon_room_helpers_source_evidence(),
                       "SUMMARIZE_STONE_ROOM:9680") != 0,
                "source evidence includes stone room symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 dungeon room helpers: ok");
    return 0;
}
