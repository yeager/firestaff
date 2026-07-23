/* ReDMCSB GROUP1.C F0189 raw C04/ActiveGroup delete admission. */
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
    CSB_V1_F0189GroupDeleteReceiptPc34 receipt;
    unsigned char raw[128];
    unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 72;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 80;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 72, group);
    put_le16(raw, 80, THING_ENDOFLIST);
    put_le16(raw, 82, THING_ENDOFLIST);
    raw[84] = 3u;

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.active_group_state_count = 1u;
    profile.active_group_state[0].valid = 1;
    profile.active_group_state[0].group_thing = group;
    profile.active_group_state[0].map_index = 0;
    profile.active_group_state[0].map_x = 0;
    profile.active_group_state[0].map_y = 0;

    CHECK(csb_v1_runtime_f0189_group_delete_receipt_pc34(
              &profile, group, 0, 0, 0, &receipt) == 1,
          "F0189 admits a linked current-map C04 with its ActiveGroup owner");
    CHECK(receipt.valid && receipt.group_thing == group &&
              receipt.group_record_offset == 80 &&
              receipt.group_next == THING_ENDOFLIST &&
              receipt.group_slot == THING_ENDOFLIST &&
              receipt.active_group_slot == 0 && receipt.group_record_fnv1a != 0u,
          "F0189 receipt preserves raw C04 and active-owner identity");

    profile.active_group_state[0].map_x = 1;
    CHECK(csb_v1_runtime_f0189_group_delete_receipt_pc34(
              &profile, group, 0, 0, 0, &receipt) == 0,
          "mismatched ActiveGroup ownership fails closed before deletion");

    memset(profile.active_group_state, 0, sizeof(profile.active_group_state));
    profile.active_group_state_count = 0u;
    CHECK(csb_v1_runtime_f0189_group_delete_receipt_pc34(
              &profile, group, 0, 0, 0, &receipt) == 1 &&
              receipt.active_group_slot == -1,
          "a raw C04 remains admissible when no optional mirror has been materialized");

    printf("csb F0189 group-delete receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
