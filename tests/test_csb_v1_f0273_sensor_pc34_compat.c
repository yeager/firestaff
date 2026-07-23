#include "csb_v1_f0273_sensor_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\\n", message); } \
    else { ++failed; printf("  FAIL: %s\\n", message); } \
} while (0)

enum { kWeaponOffset = 80, kObjectType = 27 };

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static unsigned short thing(int index, int cell)
{
    return (unsigned short)((5u << 10) | (unsigned int)index |
                            ((unsigned int)cell << 14));
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[128])
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, (unsigned char)(1u << 5), 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[5] = kWeaponOffset;
    dungeon->thing_type_counts[5] = 2;
    raw[1] |= 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, thing(0, 2));
    put_le16(raw, kWeaponOffset, thing(1, 1));
    put_le16(raw, kWeaponOffset + 2, kObjectType);
    put_le16(raw, kWeaponOffset + 4, 0xfffeu);
    put_le16(raw, kWeaponOffset + 6, kObjectType);
    csb_v1_runtime_init(profile, NULL);
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0273ReceiptPc34 receipt;
    unsigned char raw[128];

    make_fixture(&profile, &dungeon, raw);
    CHECK(csb_v1_f0273_get_object_of_type_in_cell_pc34(
              &profile, 0, 1, 1, kObjectType, &receipt) == 1 &&
              receipt.valid && receipt.matched_thing == thing(1, 1),
          "F0273 returns the first same-type object in the requested cell");
    CHECK(csb_v1_f0273_get_object_of_type_in_cell_pc34(
              &profile, 0, 1, -1, kObjectType, &receipt) == 1 &&
              receipt.matched_thing == thing(0, 2) && receipt.matched_cell == 2,
          "F0273 CELL_ANY preserves raw chain order");
    CHECK(csb_v1_f0273_get_object_of_type_in_cell_pc34(
              &profile, 0, 1, 3, kObjectType, &receipt) == 1 &&
              receipt.matched_thing == 0xffffu,
          "F0273 reports a source-bound no-match without a fallback");
    put_le16(raw, kWeaponOffset, thing(3, 0));
    CHECK(csb_v1_f0273_get_object_of_type_in_cell_pc34(
              &profile, 0, 1, -1, kObjectType + 1, &receipt) == 0,
          "stale raw Thing links fail closed");
    printf("PASSED: %d\\nFAILED: %d\\n", passed, failed);
    return failed ? 1 : 0;
}
