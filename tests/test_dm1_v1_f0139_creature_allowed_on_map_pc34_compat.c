#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static unsigned short make_group_thing(unsigned int index)
{
    return (unsigned short)((THING_TYPE_GROUP << 10) | (index & 0x03ffu));
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat groups[2];
    unsigned char rawGroups[32];
    DM1_V1_F0139_CreatureAllowedOnMapReceipt_PC34 receipt;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(&things, 0, sizeof(things));
    memset(groups, 0, sizeof(groups));
    memset(rawGroups, 0, sizeof(rawGroups));

    dungeon.loaded = 1;
    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    maps[0].creatureTypeCount = 3;
    maps[0].allowedCreatureTypes[0] = 1;
    maps[0].allowedCreatureTypes[1] = 17;
    maps[0].allowedCreatureTypes[2] = 23;
    maps[1].creatureTypeCount = 1;
    maps[1].allowedCreatureTypes[0] = 4;

    things.loaded = 1;
    things.groups = groups;
    things.groupCount = 2;
    things.thingCounts[THING_TYPE_GROUP] = 2;
    things.rawThingData[THING_TYPE_GROUP] = rawGroups;
    groups[0].creatureType = 99;
    rawGroups[4] = 17;
    rawGroups[16 + 4] = 4;

    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, make_group_thing(0), 0, &receipt) == 1,
          "F0139 accepts a raw C04 type present in target map metadata");
    check(receipt.valid && receipt.groupIndex == 0 &&
              receipt.creatureType == 17 && receipt.matchedIndex == 1 &&
              receipt.allowedCreatureTypeCount == 3,
          "F0139 receipt records raw group type and map-list match");
    check(receipt.sourceSymbol &&
              strcmp(receipt.sourceSymbol,
                     "F0139_DUNGEON_IsCreatureAllowedOnMap") == 0,
          "F0139 receipt is source named");

    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, make_group_thing(0), 1, &receipt) == 0,
          "F0139 rejects when target map does not allow the raw type");
    check(receipt.valid && receipt.creatureType == 17 &&
              receipt.matchedIndex == -1,
          "F0139 rejection still reports the source raw type");

    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, make_group_thing(1), 1, &receipt) == 1,
          "F0139 reads each C04 record by Thing index");

    rawGroups[4] = 4;
    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, make_group_thing(0), 1, &receipt) == 1,
          "F0139 follows raw C04 bytes instead of decoded group mirrors");
    check(groups[0].creatureType == 99 && receipt.creatureType == 4,
          "F0139 does not borrow decoded creature type");

    things.rawThingData[THING_TYPE_GROUP] = NULL;
    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, make_group_thing(0), 1, &receipt) == 0,
          "F0139 fails closed without loaded raw C04 records");
    check(!receipt.valid,
          "F0139 missing-source receipt is not promoted as valid");
    things.rawThingData[THING_TYPE_GROUP] = rawGroups;

    maps[1].creatureTypeCount = 17;
    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, make_group_thing(0), 1, &receipt) == 0,
          "F0139 rejects malformed map creature-count metadata");

    check(dm1_v1_dungeon_is_creature_allowed_on_map_f0139_pc34(
              &things, &dungeon, 0x0000u, 0, &receipt) == 0,
          "F0139 rejects non-group Things before raw lookup");

    if (failures != 0) return 1;
    puts("PASS: DM1 F0139 creature allowed-on-map source lock");
    return 0;
}
