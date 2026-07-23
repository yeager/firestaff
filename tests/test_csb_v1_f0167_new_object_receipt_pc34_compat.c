/* ReDMCSB DUNGEON.C F0167 -> F0166 raw object materialization receipt. */
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

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
    CSB_V1_F0167NewObjectReceiptPc34 receipt;
    unsigned char raw[96];
    unsigned short sensor = (unsigned short)((1u << 14) |
        (THING_TYPE_SENSOR << 10));
    unsigned short weapon = (unsigned short)(THING_TYPE_WEAPON << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_bytes = 1;
    dungeon.thing_data_bases[THING_TYPE_SENSOR] = 8;
    dungeon.thing_type_counts[THING_TYPE_SENSOR] = 1;
    dungeon.thing_data_bases[THING_TYPE_WEAPON] = 24;
    dungeon.thing_type_counts[THING_TYPE_WEAPON] = 2;
    put_le16(raw, 8, THING_ENDOFLIST);
    put_le16(raw, 10, (unsigned short)((51u << 7) |
        DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ));
    put_le16(raw, 24, THING_NONE);
    put_le16(raw, 28, THING_NONE);

    CHECK(csb_v1_runtime_f0167_new_object_receipt_pc34(
              &dungeon, sensor, &receipt) == 1,
          "F0167 admits a loaded C03 new-object launcher sensor");
    CHECK(receipt.valid && receipt.source_sensor_thing == sensor &&
              receipt.source_sensor_type ==
                  DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ &&
              receipt.source_icon_index == 51 &&
              receipt.allocated_thing == weapon &&
              receipt.allocated_thing_type == THING_TYPE_WEAPON &&
              receipt.allocated_item_type == 27 && receipt.object_info.valid &&
              receipt.object_info.subtype == 27 &&
              receipt.source_sensor_record_fnv1a != 0u,
          "F0167 preserves C03 source and F0166/F0141 allocation identity");
    CHECK((raw[26] & 0x7fu) == 27u && raw[24] == 0xfe && raw[25] == 0xff,
          "F0167 writes only the real allocated C05 record");

    put_le16(raw, 10, (unsigned short)((1u << 7) |
        DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_NEW_OBJ));
    CHECK(csb_v1_runtime_f0167_new_object_receipt_pc34(
              &dungeon, sensor, &receipt) == 0 && !receipt.valid,
          "F0167 rejects an unsupported source icon fail-closed");
    put_le16(raw, 10, (unsigned short)(51u << 7));
    CHECK(csb_v1_runtime_f0167_new_object_receipt_pc34(
              &dungeon, sensor, &receipt) == 0 && !receipt.valid,
          "F0167 rejects a non-generator C03 sensor fail-closed");

    printf("csb F0167 new-object receipt: %s\\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
