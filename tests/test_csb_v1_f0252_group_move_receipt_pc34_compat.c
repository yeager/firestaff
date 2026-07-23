/* ReDMCSB TIMELINE.C F0252 raw C60/C61 group-retry admission. */
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
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0252GroupMoveReceiptPc34 receipt;
    struct DM1_DispatchRecord_V1 record;
    unsigned char raw[128];
    unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&record, 0, sizeof(record));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 64;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 80;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0x30u; /* C01 corridor with a C04 chain. */
    raw[1] = 0xa8u; /* Open C08 target, no Thing list required. */
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, group);
    put_le16(raw, 80, THING_ENDOFLIST);
    put_le16(raw, 82, THING_ENDOFLIST);
    raw[84] = 3u;
    put_le16(raw, 94, 0u);
    profile.dungeon_handle = &dungeon;

    record.eventType = DM1_EVENT_MOVE_GROUP_AUDIBLE;
    record.mapIndex = 0;
    record.mapX = 1;
    record.mapY = 0;
    record.cell = group & 0xffu;
    record.effect = group >> 8;
    CHECK(csb_v1_runtime_f0252_group_move_receipt_pc34(
              &profile, &record, &receipt) == 1,
          "F0252 admits C61 only from the linked raw C04 owner");
    CHECK(receipt.valid && receipt.group_thing == group &&
              receipt.source_map_x == 0 && receipt.target_map_x == 1 &&
              receipt.target_square_type == 5 && receipt.audible == 1 &&
              receipt.group_record_fnv1a != 0u,
          "F0252 receipt preserves C04 identity and C08 destination state");

    put_le16(raw, 64, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0252_group_move_receipt_pc34(
              &profile, &record, &receipt) == 0,
          "a stale C60/C61 Thing owner fails closed before movement");

    printf("csb F0252 group-move receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
