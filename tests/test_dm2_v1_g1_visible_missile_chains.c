#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static void wr16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8);
}

static int build_fixture(DM2_V1_DungeonData *dungeon,
                         DM2_V1_RecordPoolSet *pools,
                         uint8_t raw[16], uint8_t records[24])
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(pools, 0, sizeof(*pools));
    memset(raw, 0, 16);
    memset(records, 0, 24);

    /* SKProject G1 layout: two byte tiles, one column-start word per x,
     * then one first-thing word for each tile with flag 0x10. */
    raw[0] = 0x10;
    raw[1] = 0x10;
    wr16(raw + 2, 0);
    wr16(raw + 4, 1);
    wr16(raw + 6, 0x3800u); /* DB14 subtype 0, index 0 */
    wr16(raw + 8, 0x7802u); /* DB14 subtype 1, index 2 */
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 2;
    dungeon->level_heights[0] = 1;
    dungeon->square_bytes = 1;
    dungeon->raw_map_data_base = 0;
    dungeon->level_offsets[0] = 0;
    dungeon->column_index_base = 2;
    dungeon->square_first_thing_base = 6;
    dungeon->square_first_thing_count = 2;
    dungeon->record_graph_complete = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 16;

    /* c_record.cpp:28-31 DB14 uses eight bytes. Its w0 links preserve the
     * source chain order; the final entry uses OBJECT_END_MARKER. */
    pools->valid = 1;
    pools->record_graph_complete = 1;
    pools->pools[14].record_size = 8;
    pools->pools[14].record_count = 3;
    pools->pools[14].bytes = records;
    wr16(records, 0xb801u);   /* subtype 0 root -> subtype 2, index 1 */
    wr16(records + 2, 0x1410u);
    records[4] = 17;
    records[5] = 19;
    wr16(records + 6, 0x2211u);
    wr16(records + 8, 0xfffeu);
    wr16(records + 10, 0x1420u);
    records[12] = 23;
    records[13] = 29;
    wr16(records + 14, 0x3322u);
    wr16(records + 16, 0xfffeu);
    wr16(records + 18, 0x1430u);
    records[20] = 31;
    records[21] = 37;
    wr16(records + 22, 0x4433u);
    return 1;
}

int main(void)
{
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_G1VisibleMapCell cells[2] = {{1, 0}, {0, 0}};
    DM2_V1_G1MissileChainSnapshot snapshots[4];
    uint8_t raw[16];
    uint8_t records[24];
    size_t count = 99;
    int ok = build_fixture(&dungeon, &pools, raw, records);

    memset(snapshots, 0xa5, sizeof(snapshots));
    ok &= dm2_v1_g1_snapshot_visible_missile_chains(
        &dungeon, &pools, 0, cells, 2, 0, snapshots, 4, &count);
    ok &= count == 3;
    ok &= snapshots[0].object_id == 0x7802u && snapshots[0].map == 0 &&
        snapshots[0].x == 1 && snapshots[0].y == 0 &&
        snapshots[0].chain_ordinal == 1 &&
        snapshots[0].view_position_5x5 == 8 &&
        snapshots[0].object_direction_subtype == 1 &&
        snapshots[0].missile_object == 0x1430u &&
        snapshots[0].energy_remaining == 31 &&
        snapshots[0].energy_remaining2 == 37 &&
        snapshots[0].timer_index == 0x4433u && snapshots[0].raw_record_hash != 0;
    ok &= snapshots[1].object_id == 0x3800u && snapshots[1].x == 0 &&
        snapshots[1].chain_ordinal == 1 &&
        snapshots[1].view_position_5x5 == 6 &&
        snapshots[2].object_id == 0xb801u && snapshots[2].x == 0 &&
        snapshots[2].chain_ordinal == 2 &&
        snapshots[2].view_position_5x5 == 18;

    /* Each invalid graph/capacity case is atomic: no partial missile list. */
    wr16(records + 8, 0x3800u); /* cycle from second missile to the root */
    memset(snapshots, 0xa5, sizeof(snapshots));
    count = 99;
    ok &= !dm2_v1_g1_snapshot_visible_missile_chains(
        &dungeon, &pools, 0, cells, 2, 0, snapshots, 4, &count);
    ok &= count == 0 && snapshots[0].object_id == 0;

    build_fixture(&dungeon, &pools, raw, records);
    count = 99;
    ok &= !dm2_v1_g1_snapshot_visible_missile_chains(
        &dungeon, &pools, 0, cells, 2, 0, snapshots, 2, &count);
    ok &= count == 0;

    build_fixture(&dungeon, &pools, raw, records);
    cells[1] = cells[0]; /* repeated visible cell is an invalid schedule */
    count = 99;
    ok &= !dm2_v1_g1_snapshot_visible_missile_chains(
        &dungeon, &pools, 0, cells, 2, 0, snapshots, 4, &count);
    ok &= count == 0;

    if (!ok) {
        fprintf(stderr, "visible DB14 chain snapshot test failed\n");
        return 1;
    }
    return 0;
}
