/* ReDMCSB DUNGEON.C F0164 -> F0163 raw object-list transaction. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failed;

enum { TEST_THING_TYPE_TEXT = 2 };

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
    CSB_V1_F0163F0164ObjectMoveReceiptPc34 receipt;
    unsigned char raw[128];
    unsigned char before[128];
    unsigned short weapon = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short text = (unsigned short)(TEST_THING_TYPE_TEXT << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 2;
    dungeon.level_heights[0] = 1;
    dungeon.level_offsets[0] = 80;
    dungeon.square_bytes = 1;
    dungeon.square_first_thing_base = 90;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[TEST_THING_TYPE_TEXT] = 100;
    dungeon.thing_type_counts[TEST_THING_TYPE_TEXT] = 1;
    dungeon.thing_data_bases[THING_TYPE_WEAPON] = 108;
    dungeon.thing_type_counts[THING_TYPE_WEAPON] = 1;
    raw[80] = 0x10u;
    raw[81] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 90, weapon);
    put_le16(raw, 92, text);
    put_le16(raw, 100, THING_ENDOFLIST);
    put_le16(raw, 108, THING_ENDOFLIST);
    put_le16(raw, 110, 27u); /* WEAPON.Type arrow. */

    CHECK(csb_v1_runtime_f0163_f0164_object_move_receipt_pc34(
              &dungeon, weapon, 0, 0, 0, 0, 1, 0, &receipt) == 1,
          "F0164/F0163 admits a loaded raw PC34 object transaction");
    CHECK(receipt.valid && receipt.object_info.valid &&
              receipt.object_info.thing == weapon &&
              receipt.source_first_before == weapon &&
              receipt.source_previous_thing == THING_NONE &&
              receipt.source_next_thing == THING_ENDOFLIST &&
              receipt.destination_first_before == text &&
              receipt.destination_tail_thing == text &&
              receipt.source_record_fnv1a_before != 0u &&
              receipt.source_record_fnv1a_after != 0u,
          "receipt retains exact F0141 object and raw source/destination chain identity");
    CHECK(raw[90] == 0xfe && raw[91] == 0xff &&
              (raw[100] | ((unsigned short)raw[101] << 8)) == weapon &&
              raw[108] == 0xfe && raw[109] == 0xff,
          "F0164 unlinks the source and F0163 appends the object at target tail");

    memcpy(before, raw, sizeof(raw));
    CHECK(csb_v1_runtime_f0163_f0164_object_move_receipt_pc34(
              &dungeon, weapon, 0, 0, 0, 0, 1, 0, &receipt) == 0 &&
              !receipt.valid && memcmp(before, raw, sizeof(raw)) == 0,
          "absent source membership fails closed without a partial raw mutation");

    printf("csb F0163/F0164 object move receipt: %s\\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
