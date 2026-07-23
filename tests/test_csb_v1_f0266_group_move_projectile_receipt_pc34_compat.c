/* ReDMCSB MOVE.C F0266 raw C04 move/projectile preflight. */
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
    CSB_V1_F0266GroupMoveProjectileReceiptPc34 receipt;
    CSB_V1_F0175GroupThingReceiptPc34 group_receipt;
    CSB_V1_F0176CreatureOrdinalReceiptPc34 ordinal;
    unsigned char raw[144];
    unsigned short projectile = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 64;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[THING_TYPE_PROJECTILE] = 80;
    dungeon.thing_type_counts[THING_TYPE_PROJECTILE] = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 88;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0x30u;
    raw[1] = 0x30u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, group);
    put_le16(raw, 66, THING_ENDOFLIST);
    put_le16(raw, 80, THING_ENDOFLIST);
    put_le16(raw, 88, projectile);
    raw[92] = 3u;
    raw[93] = 0x08u;
    put_le16(raw, 94, 40u);
    put_le16(raw, 96, 40u);
    put_le16(raw, 102, 0x0020u);

    CHECK(csb_v1_runtime_f0175_group_thing_receipt_pc34(
              &dungeon, 0, 0, 0, &group_receipt) == 1 &&
              group_receipt.group_thing == group,
          "fixture exposes the source C04 through its raw Thing chain");
    CHECK(csb_v1_runtime_f0176_creature_ordinal_receipt_pc34(
              &dungeon, group, 0, 0, 0, 0, &ordinal) == 1 &&
              ordinal.creature_ordinal == 1,
          "fixture exposes C04 creature ordinal at cell zero");

    CHECK(csb_v1_runtime_f0266_group_move_projectile_receipt_pc34(
              &dungeon, group, 0, 0, 0, 1, 0, &receipt) == 1,
          "F0266 admits an adjacent move only from a linked live raw C04");
    CHECK(receipt.valid && receipt.source_projectile_count == 1 &&
              receipt.destination_projectile_count == 0 &&
              receipt.live_creature_cell_mask == 0x05u &&
              receipt.intermediary_creature_cell_mask == 0x08u &&
              receipt.group_record_fnv1a != 0u,
          "F0266 preserves raw C14 count and the exact live/intermediary cells");

    put_le16(raw, 94, 0u);
    put_le16(raw, 96, 0u);
    CHECK(csb_v1_runtime_f0266_group_move_projectile_receipt_pc34(
              &dungeon, group, 0, 0, 0, 1, 0, &receipt) == 0 && !receipt.valid,
          "a dead C04 fails closed before projectile-impact preflight");

    put_le16(raw, 94, 40u);
    put_le16(raw, 96, 40u);
    CHECK(csb_v1_runtime_f0266_group_move_projectile_receipt_pc34(
              &dungeon, group, 0, 0, 0, 0, 0, &receipt) == 0,
          "a non-adjacent source/destination pair is never admitted");

    printf("csb F0266 group-move projectile receipt: %s\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
