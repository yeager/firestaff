/* ReDMCSB TIMELINE.C F0249 C04-first open-square admission. */
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

static unsigned short read_le16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0175GroupThingReceiptPc34 group_receipt;
    CSB_V1_F0249OpenSquareGroupReceiptPc34 receipt;
    struct DM1_Event_V1 event;
    unsigned char raw[160];
    unsigned short teleporter = (unsigned short)(THING_TYPE_TELEPORTER << 10);
    unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(&profile, 0, sizeof(profile));
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
    dungeon.thing_data_bases[THING_TYPE_TELEPORTER] = 80;
    dungeon.thing_type_counts[THING_TYPE_TELEPORTER] = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 96;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0xb8u; /* Open C08 teleporter square with a Thing list. */
    raw[1] = 0x20u; /* Open corridor destination. */
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, teleporter);
    put_le16(raw, 66, THING_NONE);
    put_le16(raw, 80, group);
    put_le16(raw, 82, 0x2001u); /* Creature scope, target (1, 0). */
    put_le16(raw, 84, 0u);
    put_le16(raw, 96, THING_ENDOFLIST);
    put_le16(raw, 98, THING_ENDOFLIST);
    raw[100] = 3u;
    put_le16(raw, 110, 0u);
    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = &dungeon;

    CHECK(csb_v1_runtime_f0175_group_thing_receipt_pc34(
              &dungeon, 0, 0, 0, &group_receipt) == 1 &&
              group_receipt.group_thing == group,
          "fixture exposes the C04 after the C05 through the raw Thing chain");
    CHECK(csb_v1_runtime_f0249_open_square_group_receipt_pc34(
              &profile, 5, 0, 0, 0, &receipt) == 1,
          "F0249 admits the linked C04 first on a real open C08 square");
    CHECK(receipt.valid && receipt.group_thing == group &&
              receipt.group_record_offset == 96 && receipt.square_type == 5 &&
              receipt.group_record_fnv1a != 0u,
          "F0249 receipt preserves raw C04 identity before movement");

    raw[0] = 0xb0u; /* Closed C08 with the same source Thing chain. */
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.type = DM1_EVENT_TELEPORTER;
    event.b_mapX = 0u;
    event.b_mapY = 0u;
    event.c_effect = DM1_EFFECT_SET;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &event) >= 0 &&
              csb_v1_runtime_tick_v1(&profile) == 1,
          "C08 SET dispatches the loaded F0249 source square");
    CHECK((raw[0] & 0x08u) != 0u && read_le16(raw + 80) == THING_ENDOFLIST &&
              read_le16(raw + 66) == group,
          "F0249 moves the admitted C04 before later open-square consumers");

    put_le16(raw, 80, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0249_open_square_group_receipt_pc34(
              &profile, 5, 0, 0, 0, &receipt) == 0,
          "an open C08 without a linked C04 fails closed");

    printf("csb F0249 open-square group receipt: %s\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
