/* ReDMCSB DUNGEON.C F0141 -> DUNGLOB.C G0209 raw ObjectInfo arithmetic. */
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

static void make_raw_pc34_fixture(CSB_V1_DungeonData *dungeon,
                                  unsigned char raw[64])
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 64u);
    dungeon->raw_data = raw;
    dungeon->raw_size = 64;
    dungeon->thing_data_bases[THING_TYPE_ARMOUR] = 8;
    dungeon->thing_type_counts[THING_TYPE_ARMOUR] = 1;
    dungeon->thing_data_bases[THING_TYPE_JUNK] = 16;
    dungeon->thing_type_counts[THING_TYPE_JUNK] = 1;
    put_le16(raw, 8, THING_ENDOFLIST);
    put_le16(raw, 10, 57u); /* Armour subtype 57 -> ObjectInfo row 126. */
    put_le16(raw, 16, THING_ENDOFLIST);
    put_le16(raw, 18, 52u); /* Junk subtype 52 -> ObjectInfo row 179. */
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0141G0209ObjectInfoReceiptPc34 receipt;
    unsigned char raw[64];
    unsigned short armour = (unsigned short)(THING_TYPE_ARMOUR << 10);
    unsigned short junk = (unsigned short)(THING_TYPE_JUNK << 10);

    make_raw_pc34_fixture(&dungeon, raw);
    CHECK(csb_v1_runtime_f0141_g0209_object_info_receipt_pc34(
              &dungeon, armour, &receipt) == 1,
          "F0141 admits a loaded PC34 armour Thing");
    CHECK(receipt.valid && receipt.thing == armour &&
              receipt.thing_type == THING_TYPE_ARMOUR &&
              receipt.thing_index == 0 && receipt.subtype == 57 &&
              receipt.object_info_index == 126 && receipt.record_offset == 8 &&
              receipt.record_size == 4 && receipt.record_fnv1a != 0u,
          "F0141 locks armour raw type/subtype to ObjectInfo arithmetic");
    CHECK(csb_v1_runtime_f0141_g0209_object_info_receipt_pc34(
              &dungeon, junk, &receipt) == 1 && receipt.subtype == 52 &&
              receipt.object_info_index == 179 && receipt.record_offset == 16,
          "F0141 locks junk raw type/subtype to ObjectInfo arithmetic");
    CHECK(csb_v1_runtime_f0141_g0209_object_info_receipt_pc34(
              NULL, armour, &receipt) == 0 && !receipt.valid,
          "F0141 rejects an absent PC34 DUNGEON.DAT admission source");
    dungeon.thing_type_counts[THING_TYPE_ARMOUR] = 0;
    CHECK(csb_v1_runtime_f0141_g0209_object_info_receipt_pc34(
              &dungeon, armour, &receipt) == 0 && !receipt.valid,
          "F0141 rejects a Thing outside the admitted PC34 record table");

    printf("csb F0141/G0209 raw object-info receipt: %s\\n",
           failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
