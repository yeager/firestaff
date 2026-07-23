/* ReDMCSB DUNGEON.C F0143/F0144 raw runtime receipts, without graphics. */
#include "csb_v1_runtime_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"

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
    CSB_V1_F0143ArmourDefenseReceiptPc34 armour;
    CSB_V1_F0144CreatureAttributesReceiptPc34 creature;
    const struct CreatureBehaviorProfile_Compat *profile;
    unsigned char raw[64];
    unsigned short armour_thing = (unsigned short)(THING_TYPE_ARMOUR << 10);
    unsigned short group_thing = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.thing_data_bases[THING_TYPE_ARMOUR] = 8;
    dungeon.thing_type_counts[THING_TYPE_ARMOUR] = 1;
    dungeon.thing_data_bases[THING_TYPE_GROUP] = 24;
    dungeon.thing_type_counts[THING_TYPE_GROUP] = 1;
    put_le16(raw, 8, THING_ENDOFLIST);
    put_le16(raw, 10, 34u); /* G0239 row 34: defense 70, sharp bits 7. */
    put_le16(raw, 24, THING_ENDOFLIST);
    raw[28] = 1u; /* Raw C04 GROUP.Type -> CreatureInfo row 1. */
    put_le16(raw, 38, 0u);

    CHECK(csb_v1_runtime_f0143_armour_defense_receipt_pc34(
              &dungeon, armour_thing, 1, &armour) == 1,
          "F0143 admits raw PC34 ARMOUR through F0141");
    CHECK(armour.valid && armour.object_info.valid && armour.armour_type == 34 &&
              armour.object_info.object_info_index == 103 &&
              armour.base_defense == 70 && armour.sharp_defense_bits == 7 &&
              armour.defense == 96 && armour.armour_info_fnv1a != 0u,
          "F0143 retains G0239 defense and sharp-defense arithmetic");
    CHECK(csb_v1_runtime_f0143_armour_defense_receipt_pc34(
              &dungeon, group_thing, 0, &armour) == 0 && !armour.valid,
          "F0143 rejects non-armour Things fail-closed");

    profile = CREATURE_GetProfile_Compat(1);
    CHECK(profile != NULL, "test obtains source CreatureInfo row");
    CHECK(csb_v1_runtime_f0144_creature_attributes_receipt_pc34(
              &dungeon, group_thing, &creature) == 1,
          "F0144 admits raw PC34 C04 GROUP.Type");
    CHECK(creature.valid && creature.group_thing == group_thing &&
              creature.record_offset == 24 && creature.record_size == 16 &&
              creature.creature_type == 1 &&
              creature.base_defense == profile->baseDefense &&
              creature.attributes == profile->attributes &&
              creature.creature_info_fnv1a != 0u,
          "F0144 retains the matched immutable CreatureInfo attributes");
    raw[28] = 27u;
    CHECK(csb_v1_runtime_f0144_creature_attributes_receipt_pc34(
              &dungeon, group_thing, &creature) == 0 && !creature.valid,
          "F0144 rejects an out-of-range raw GROUP.Type fail-closed");

    printf("csb F0143/F0144 runtime receipts: %s\\n", failed ? "FAIL" : "PASS");
    return failed ? 1 : 0;
}
