#include "dm1_v1_object_info_index_f0141_pc34_compat.h"

#include <assert.h>
#include <string.h>

static DM1_V1_F0141_ObjectRecordPc34 record(uint16_t thing,
                                            uint8_t thingType,
                                            uint8_t objectType)
{
    DM1_V1_F0141_ObjectRecordPc34 r;

    r.thing = thing;
    r.thingType = thingType;
    r.objectType = objectType;
    return r;
}

int main(void)
{
    DM1_V1_F0141_ObjectRecordPc34 records[6];
    DM1_V1_F0141_ObjectWorldPc34 world;

    records[0] = record(0x0107u, DM1_V1_F0141_THING_TYPE_SCROLL_PC34, 99);
    records[1] = record(0x0209u, DM1_V1_F0141_THING_TYPE_CONTAINER_PC34, 0);
    records[2] = record(0x0308u, DM1_V1_F0141_THING_TYPE_POTION_PC34, 20);
    records[3] = record(0x0405u, DM1_V1_F0141_THING_TYPE_WEAPON_PC34, 45);
    records[4] = record(0x0506u, DM1_V1_F0141_THING_TYPE_ARMOUR_PC34, 57);
    records[5] = record(0x060au, DM1_V1_F0141_THING_TYPE_JUNK_PC34, 52);

    world.records = records;
    world.recordCount = sizeof(records) / sizeof(records[0]);

    assert(strstr(DM1_V1_F0141_SourceEvidencePc34(),
                  "F0141_DUNGEON_GetObjectInfoIndex") != 0);

    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, records[0].thing) == 0);
    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, records[1].thing) == 1);
    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, records[2].thing) == 22);
    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, records[3].thing) == 68);
    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, records[4].thing) == 126);
    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, records[5].thing) == 179);

    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_POTION_PC34, 0) == 2);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_WEAPON_PC34, 0) == 23);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_ARMOUR_PC34, 0) == 69);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_JUNK_PC34, 0) == 127);

    assert(DM1_V1_Dungeon_GetObjectInfoIndexF0141Pc34Compat(
               &world, records[4].thing) == 126);

    assert(F0141_DUNGEON_GetObjectInfoIndex(
               &world, DM1_V1_F0141_THING_NONE_PC34) == -1);
    assert(F0141_DUNGEON_GetObjectInfoIndex(
               &world, DM1_V1_F0141_THING_END_OF_LIST_PC34) == -1);
    assert(F0141_DUNGEON_GetObjectInfoIndex(&world, 0x7777u) == -1);
    assert(F0141_DUNGEON_GetObjectInfoIndex(0, records[0].thing) == -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_CONTAINER_PC34, 1) == -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_POTION_PC34, 21) == -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_WEAPON_PC34, 46) == -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_ARMOUR_PC34, 58) == -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_JUNK_PC34, 53) == -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(4, 0) ==
           -1);
    assert(DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
               DM1_V1_F0141_THING_TYPE_SCROLL_PC34, -1) == -1);

    return 0;
}
