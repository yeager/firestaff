#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
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
    CSB_V1_F0183ActiveGroupReceiptPc34 receipt;
    unsigned char raw[120];
    unsigned short group0 = (unsigned short)(4u << 10);
    unsigned short group1 = (unsigned short)((4u << 10) | 1u);
    unsigned short sensor = (unsigned short)(3u << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, (unsigned char)(1u << 5), sizeof(raw));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 2;
    dungeon.square_bytes = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[3] = 70;
    dungeon.thing_type_counts[3] = 1;
    dungeon.thing_data_bases[4] = 76;
    dungeon.thing_type_counts[4] = 2;

    raw[0] |= 0x10u;
    raw[2] |= 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 66, group0);
    put_le16(raw, 68, sensor);
    put_le16(raw, 70, group1);
    put_le16(raw, 76, 0xfffeu);
    raw[80] = 9u;
    raw[81] = 0xe4u;
    put_le16(raw, 90, 0x0220u);
    put_le16(raw, 92, 0xfffeu);
    raw[96] = 7u;
    raw[97] = 0xffu;
    put_le16(raw, 106, 0x0100u);

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.game_time = 200u;

    CHECK(csb_v1_runtime_f0183_active_group_receipt_pc34(
              &profile, group1, 0, 1, 0, &receipt) == 1,
          "F0183 admits a linked raw C04 on the current map before mutation");
    CHECK(receipt.valid && !receipt.already_active &&
              receipt.active_group_slot == 0 && receipt.creature_count == 1 &&
              receipt.group_cells == 0xffu && receipt.group_direction == 1 &&
              receipt.group_record_offset == 92 && receipt.group_record_fnv1a != 0u &&
              receipt.creature_attributes.valid,
          "F0183 receipt retains exact C04, CreatureInfo, cells, direction, and pool slot");
    CHECK(csb_v1_runtime_f0183_active_group_receipt_pc34(
              &profile, group1, 0, 0, 0, &receipt) == 0 && !receipt.valid,
          "F0183 rejects a C04 that is not linked to its claimed square");

    CHECK(csb_v1_runtime_f0195_group_add_all_active_groups(&profile) == 2,
          "F0195 activates each C04 group on the current map");
    CHECK(profile.active_group_state_count == 2u,
          "F0195 records both active groups");
    CHECK(profile.active_group_state[0].valid &&
              profile.active_group_state[0].group_thing == group0 &&
              profile.active_group_state[0].map_x == 0 &&
              profile.active_group_state[0].map_y == 0 &&
              profile.active_group_state[0].cells == 0xe4u &&
              profile.active_group_state[0].directions == 0x00aau &&
              profile.active_group_state[0].last_move_time == 73u,
          "F0195 initializes C04 state from its native record");
    CHECK(profile.active_group_state[1].valid &&
              profile.active_group_state[1].group_thing == group1 &&
              profile.active_group_state[1].map_x == 1 &&
              profile.active_group_state[1].map_y == 0 &&
              profile.active_group_state[1].cells == 0xffu &&
              profile.active_group_state[1].directions == 0x0055u,
          "F0195 follows a mixed C03/C04 thing chain");
    CHECK(csb_v1_runtime_f0195_group_add_all_active_groups(&profile) == 0 &&
              profile.active_group_state_count == 2u,
          "F0195 does not duplicate already active groups");

    return failures == 0 ? 0 : 1;
}
