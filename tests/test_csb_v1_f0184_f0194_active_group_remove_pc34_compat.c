/* ReDMCSB GROUP1.C F0184/F0194 raw ActiveGroup writeback. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(condition, message) do { \
    if (condition) printf("  PASS: %s\n", message); \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static unsigned short get_le16(const unsigned char *bytes, int offset)
{
    return (unsigned short)(bytes[offset] | ((unsigned short)bytes[offset + 1] << 8));
}

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0184ActiveGroupRemoveReceiptPc34 receipt;
    unsigned char raw[120];
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
    raw[84] = 3u;
    raw[85] = 0x00u;
    put_le16(raw, 94, 0x0166u); /* behavior C6, raw direction 1. */

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.active_group_state_count = 1u;
    profile.active_group_state[0].valid = 1;
    profile.active_group_state[0].group_thing = group;
    profile.active_group_state[0].map_index = 0;
    profile.active_group_state[0].map_x = 0;
    profile.active_group_state[0].map_y = 0;
    profile.active_group_state[0].cells = 0x39u;
    profile.active_group_state[0].directions = 0x0055u;

    CHECK(csb_v1_runtime_f0184_active_group_remove_receipt_pc34(
              &profile, 0, &receipt) == 1,
          "F0184 admits an active slot only while its raw C04 remains linked");
    CHECK(receipt.valid && receipt.group_thing == group &&
              receipt.group_record_offset == 80 && receipt.group_cells == 0x39u &&
              receipt.group_direction == 1 && receipt.behavior_before == 6 &&
              receipt.behavior_after == 0 && receipt.group_record_fnv1a != 0u,
          "F0184 receipt carries exact C04 cells, normalized direction, and behavior");

    CHECK(csb_v1_runtime_f0194_remove_all_active_groups_pc34(&profile) == 1,
          "F0194 commits one authenticated F0184 writeback");
    CHECK(raw[85] == 0x39u && get_le16(raw, 94) == 0x0160u &&
              profile.active_group_state_count == 0u &&
              !profile.active_group_state[0].valid,
          "F0194 writes Cells/Direction/Behavior then clears the active pool");

    profile.active_group_state_count = 1u;
    profile.active_group_state[0].valid = 1;
    profile.active_group_state[0].group_thing = group;
    profile.active_group_state[0].map_index = 0;
    profile.active_group_state[0].map_x = 0;
    profile.active_group_state[0].map_y = 0;
    put_le16(raw, 72, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0194_remove_all_active_groups_pc34(&profile) < 0 &&
              profile.active_group_state_count == 1u &&
              profile.active_group_state[0].valid,
          "F0194 rejects an unlinked C04 before mutating raw or active state");

    printf("csb F0184/F0194 active-group removal: %s\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
