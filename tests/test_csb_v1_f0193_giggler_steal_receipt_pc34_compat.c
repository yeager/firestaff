/* ReDMCSB GROUP1.C F0193 raw C04 Giggler-slot admission. */
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
    CSB_V1_F0193GigglerStealReceiptPc34 receipt;
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
    raw[84] = 2u; /* Giggler. */
    put_le16(raw, 94, 0x0006u); /* One creature, attacking. */

    profile.dungeon_handle = &dungeon;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.Champions[0].CurrentHealth = 100;

    CHECK(csb_v1_runtime_f0193_giggler_steal_receipt_pc34(
              &profile, group, 0, 0, 0, 0, 0, &receipt) == 1,
          "F0193 admits a linked, attacking Giggler C04 before Slot mutation");
    CHECK(receipt.valid && receipt.group_thing == group &&
              receipt.group_record_offset == 80 &&
              receipt.creature_index == 0 && receipt.champion_index == 0 &&
              receipt.group_slot_before == THING_ENDOFLIST &&
              receipt.group_record_fnv1a != 0u,
          "F0193 receipt retains raw C04 and live champion identity");

    raw[84] = 3u;
    CHECK(csb_v1_runtime_f0193_giggler_steal_receipt_pc34(
              &profile, group, 0, 0, 0, 0, 0, &receipt) == 0,
          "a non-Giggler C04 fails closed before champion slots are touched");

    printf("csb F0193 Giggler-steal receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
