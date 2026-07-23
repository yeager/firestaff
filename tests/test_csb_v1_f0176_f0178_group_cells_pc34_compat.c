/* ReDMCSB GROUP1.C F0176/F0178 raw C04 cell receipts. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(condition, message) do { \
    if (condition) printf("  PASS: %s\n", message); \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0176CreatureOrdinalReceiptPc34 ordinal;
    CSB_V1_F0178GroupCellsCompactReceiptPc34 compact;
    unsigned char raw[144];
    unsigned short sensor = (unsigned short)(THING_TYPE_SENSOR << 10);
    unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 80;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 90;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[THING_TYPE_SENSOR] = 100;
    dungeon.thing_type_counts[THING_TYPE_SENSOR] = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 108;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[80] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 90, sensor);
    put_le16(raw, 100, group);
    put_le16(raw, 108, THING_ENDOFLIST);
    raw[112] = 3u;
    raw[113] = 0x08u; /* C04 cells: creature 0 at 0, creature 1 at 2. */
    put_le16(raw, 122, 0x0020u); /* Two creatures, facing direction 0. */

    CHECK(csb_v1_runtime_f0176_creature_ordinal_receipt_pc34(
              &dungeon, group, 0, 0, 0, 2, &ordinal) == 1,
          "F0176 admits only the linked raw C04 on its PC34 square");
    CHECK(ordinal.valid && ordinal.creature_count == 2 &&
              ordinal.creature_ordinal == 2 && ordinal.group_cells == 0x08u &&
              ordinal.creature_attributes.valid &&
              ordinal.group_record_offset == 108 && ordinal.group_record_fnv1a != 0u,
          "F0176 retains C04, CreatureInfo, count, packed cells, and ordinal");

    CHECK(csb_v1_runtime_f0178_group_cells_compact_receipt_pc34(
              &dungeon, group, 0, 0, 0, 2, 0, &compact) == 1,
          "F0178 derives a partial-kill packed-cell rewrite from linked raw C04");
    CHECK(compact.valid && compact.original_group_cells == 0x08u &&
              compact.compacted_group_cells == 0x02u &&
              compact.removed_creature_index == 0 &&
              compact.group_record_fnv1a != 0u,
          "F0178 preserves the survivor cell and clears the retired ordinal");

    put_le16(raw, 100, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0176_creature_ordinal_receipt_pc34(
              &dungeon, group, 0, 0, 0, 2, &ordinal) == 0 && !ordinal.valid &&
              csb_v1_runtime_f0178_group_cells_compact_receipt_pc34(
                  &dungeon, group, 0, 0, 0, 2, 0, &compact) == 0 &&
              !compact.valid,
          "F0176/F0178 reject an unlinked C04 fail-closed");

    printf("csb F0176/F0178 group-cell receipts: %s\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
