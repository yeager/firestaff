/* ReDMCSB MOVESENS.C F0267 -> GROUP1.C F0191 raw fall admission. */
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
    CSB_V1_F0191GroupFallReceiptPc34 receipt;
    unsigned char raw[128];
    unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

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
    put_le16(raw, 86, 40u);
    put_le16(raw, 88, 40u);
    put_le16(raw, 94, 0x0020u); /* Two C04 creatures. */

    CHECK(csb_v1_runtime_f0191_group_fall_receipt_pc34(
              &dungeon, group, 0, 0, 0, 20, &receipt) == 1,
          "F0191 admits the C04 only after it is linked at the destination square");
    CHECK(receipt.valid && receipt.group_thing == group &&
              receipt.group_record_offset == 80 && receipt.creature_count == 2 &&
              receipt.attack == 20 && receipt.random_window == 3 &&
              receipt.group_record_fnv1a != 0u,
          "F0191 receipt preserves raw C04 identity and source attack fanout");

    put_le16(raw, 72, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0191_group_fall_receipt_pc34(
              &dungeon, group, 0, 0, 0, 20, &receipt) == 0,
          "an unlinked destination C04 fails closed before fall damage");

    printf("csb F0191 group-fall receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
