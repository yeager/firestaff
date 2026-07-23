/* ReDMCSB MOVE.C F0265 raw C04 C60/C61 creation admission. */
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
    CSB_V1_F0265GroupRetryReceiptPc34 receipt;
    unsigned char raw[128];
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
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 80;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0x30u; /* C01 source corridor with C04. */
    raw[1] = 0x48u; /* Open C09 target pit. */
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, group);
    put_le16(raw, 80, THING_ENDOFLIST);
    put_le16(raw, 82, THING_ENDOFLIST);
    raw[84] = 3u;
    put_le16(raw, 94, 0u);
    profile.dungeon_handle = &dungeon;

    CHECK(csb_v1_runtime_f0265_group_retry_receipt_pc34(
              &profile, group, 0, 1, 0, 1, &receipt) == 1,
          "F0265 admits C61 creation only from a linked raw C04");
    CHECK(receipt.valid && receipt.source_map_x == 0 &&
              receipt.target_map_x == 1 && receipt.target_square_type == 2 &&
              receipt.audible == 1 && receipt.group_record_fnv1a != 0u,
          "F0265 receipt retains C04 identity and loaded C09 target state");

    put_le16(raw, 64, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0265_group_retry_receipt_pc34(
              &profile, group, 0, 1, 0, 0, &receipt) == 0,
          "an unlinked C04 cannot synthesize a C60 retry");

    printf("csb F0265 group-retry receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
