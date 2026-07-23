/* ReDMCSB DUNGEON.C F0158 -> G0238 raw weapon runtime receipt. */
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
    CSB_V1_F0158WeaponInfoReceiptPc34 receipt;
    unsigned char raw[48];
    unsigned short bow = (unsigned short)(THING_TYPE_WEAPON << 10);
    unsigned short armour = (unsigned short)(THING_TYPE_ARMOUR << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.thing_data_bases[THING_TYPE_WEAPON] = 8;
    dungeon.thing_type_counts[THING_TYPE_WEAPON] = 1;
    dungeon.thing_data_bases[THING_TYPE_ARMOUR] = 16;
    dungeon.thing_type_counts[THING_TYPE_ARMOUR] = 1;
    put_le16(raw, 8, THING_ENDOFLIST);
    put_le16(raw, 10, 25u); /* G0238 BOW row. */
    put_le16(raw, 16, THING_ENDOFLIST);
    put_le16(raw, 18, 0u);

    CHECK(csb_v1_runtime_f0158_weapon_info_receipt_pc34(
              &dungeon, bow, &receipt) == 1,
          "F0158 admits a loaded raw PC34 weapon through F0141");
    CHECK(receipt.valid && receipt.object_info.valid && receipt.weapon_type == 25 &&
              receipt.object_info.object_info_index == 48 && receipt.weight == 10 &&
              receipt.weapon_class == 20 && receipt.strength == 1 &&
              receipt.kinetic_energy == 50 && receipt.shoot_attack == 50 &&
              receipt.weapon_info_fnv1a != 0u,
          "F0158 locks G0238 BOW fields to its raw Weapon.Type");
    CHECK(csb_v1_runtime_f0158_weapon_info_receipt_pc34(
              &dungeon, armour, &receipt) == 0 && !receipt.valid,
          "F0158 rejects a non-weapon Thing fail-closed");
    put_le16(raw, 10, 46u);
    CHECK(csb_v1_runtime_f0158_weapon_info_receipt_pc34(
              &dungeon, bow, &receipt) == 0 && !receipt.valid,
          "F0158 rejects an out-of-range raw Weapon.Type fail-closed");

    printf("csb F0158 weapon-info receipt: %s\\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
