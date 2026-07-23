/* ReDMCSB GROUP1.C F0175 raw C04 lookup and F0144 creature receipt. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(condition, message) do { \
    if (condition) printf("  PASS: %s\\n", message); \
    else { ++failed; printf("  FAIL: %s\\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0175GroupThingReceiptPc34 receipt;
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
    raw[112] = 3u; /* Raw C04 GROUP.Type, G0243 creature 3. */

    CHECK(csb_v1_runtime_f0175_group_thing_receipt_pc34(
              &dungeon, 0, 0, 0, &receipt) == 1,
          "F0175 scans past a leading C03 to the loaded raw C04");
    CHECK(receipt.valid && receipt.square_first_thing == sensor &&
              receipt.group_thing == group && receipt.group_record_offset == 108 &&
              receipt.group_record_size == 16 && receipt.group_record_fnv1a != 0u &&
              receipt.creature_attributes.valid &&
              receipt.creature_attributes.creature_type == 3,
          "F0175 receipt preserves chain identity and F0144 creature attributes");

    put_le16(raw, 100, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0175_group_thing_receipt_pc34(
              &dungeon, 0, 0, 0, &receipt) == 0 && !receipt.valid,
          "F0175 rejects a chain without an admitted C04 fail-closed");

    printf("csb F0175 group-thing receipt: %s\\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
