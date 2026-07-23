/* ReDMCSB GROUP1.C F0185 raw C006 generator admission. */
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
    CSB_V1_F0185GeneratedGroupReceiptPc34 receipt;
    unsigned char raw[128];
    unsigned short sensor = (unsigned short)(THING_TYPE_SENSOR << 10);

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
    dungeon.thing_data_bases[THING_TYPE_SENSOR] = 80;
    dungeon.thing_type_counts[THING_TYPE_SENSOR] = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 100;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 72, sensor);
    put_le16(raw, 80, THING_ENDOFLIST);
    put_le16(raw, 82, (unsigned short)(6u | (3u << 7)));
    put_le16(raw, 84, 0x0244u);
    put_le16(raw, 86, 0x0130u);
    put_le16(raw, 100, THING_NONE);

    CHECK(csb_v1_runtime_f0185_generated_group_receipt_pc34(
              &dungeon, sensor, 0, 0, 0, &receipt) == 1,
          "F0185 admits a linked raw C006 and one unused raw C04 slot");
    CHECK(receipt.valid && receipt.source_sensor_thing == sensor &&
              receipt.source_sensor_record_offset == 80 &&
              receipt.creature_type == 3 && receipt.flags_word == 0x0244u &&
              receipt.local_word == 0x0130u &&
              receipt.allocated_group_thing == (THING_TYPE_GROUP << 10) &&
              receipt.allocated_group_record_offset == 100 &&
              receipt.source_sensor_record_fnv1a != 0u &&
              receipt.allocated_group_record_fnv1a != 0u,
          "receipt preserves exact C006 fields and C04 allocation identity");

    put_le16(raw, 72, THING_ENDOFLIST);
    CHECK(csb_v1_runtime_f0185_generated_group_receipt_pc34(
              &dungeon, sensor, 0, 0, 0, &receipt) == 0,
          "a detached C006 fails closed before F0185 materialization");

    printf("csb F0185 generated-group receipt: %s\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
