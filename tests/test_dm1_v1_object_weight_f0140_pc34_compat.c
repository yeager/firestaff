#include "dm1_v1_object_weight_f0140_pc34_compat.h"

#include <assert.h>
#include <string.h>

static DM1_V1_F0140_ObjectRecordPc34 record(uint16_t thing,
                                            uint8_t thingType,
                                            uint8_t objectType)
{
    DM1_V1_F0140_ObjectRecordPc34 r;

    r.thing = thing;
    r.thingType = thingType;
    r.objectType = objectType;
    r.chargeCount = 0;
    r.nextThing = DM1_V1_F0140_THING_END_OF_LIST_PC34;
    r.containerSlotHead = DM1_V1_F0140_THING_END_OF_LIST_PC34;
    return r;
}

int main(void)
{
    const uint8_t weaponWeights[] = { 24, 11, 7, 3 };
    const uint8_t armourWeights[] = { 3, 4, 3, 6, 16 };
    const uint8_t junkWeights[] = { 1, 3, 2, 2, 4 };
    DM1_V1_F0140_ObjectRecordPc34 records[10];
    DM1_V1_F0140_ObjectWorldPc34 world;
    int weight = -99;
    (void)weight;

    records[0] = record(0x0105u, DM1_V1_F0140_THING_TYPE_WEAPON_PC34, 1);
    records[1] = record(0x0206u, DM1_V1_F0140_THING_TYPE_ARMOUR_PC34, 4);
    records[2] = record(0x0307u, DM1_V1_F0140_THING_TYPE_SCROLL_PC34, 17);
    records[3] = record(0x0408u, DM1_V1_F0140_THING_TYPE_POTION_PC34,
                        DM1_V1_F0140_EMPTY_FLASK_POTION_TYPE_PC34);
    records[4] = record(0x0508u, DM1_V1_F0140_THING_TYPE_POTION_PC34, 9);
    records[5] = record(0x060au, DM1_V1_F0140_THING_TYPE_JUNK_PC34,
                        DM1_V1_F0140_JUNK_WATERSKIN_TYPE_PC34);
    records[5].chargeCount = 4;
    records[6] = record(0x0709u, DM1_V1_F0140_THING_TYPE_CONTAINER_PC34, 0);
    records[7] = record(0x080au, DM1_V1_F0140_THING_TYPE_JUNK_PC34, 2);
    records[8] = record(0x0909u, DM1_V1_F0140_THING_TYPE_CONTAINER_PC34, 0);
    records[9] = record(0x0a07u, DM1_V1_F0140_THING_TYPE_SCROLL_PC34, 0);

    records[6].containerSlotHead = records[0].thing;
    records[0].nextThing = records[8].thing;
    records[8].containerSlotHead = records[7].thing;
    records[7].nextThing = records[9].thing;

    world.records = records;
    world.recordCount = sizeof(records) / sizeof(records[0]);
    world.weaponWeights = weaponWeights;
    world.weaponWeightCount = sizeof(weaponWeights) / sizeof(weaponWeights[0]);
    world.armourWeights = armourWeights;
    world.armourWeightCount = sizeof(armourWeights) / sizeof(armourWeights[0]);
    world.junkWeights = junkWeights;
    world.junkWeightCount = sizeof(junkWeights) / sizeof(junkWeights[0]);

    assert(strstr(DM1_V1_F0140_SourceEvidencePc34(),
                  "F0140_DUNGEON_GetObjectWeight") != 0);

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[0].thing,
                                         &weight) == 0);
    assert(weight == 11);

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[1].thing,
                                         &weight) == 0);
    assert(weight == 16);

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[2].thing,
                                         &weight) == 0);
    assert(weight == DM1_V1_F0140_SCROLL_WEIGHT_PC34);

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[3].thing,
                                         &weight) == 0);
    assert(weight == DM1_V1_F0140_EMPTY_FLASK_WEIGHT_PC34);

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[4].thing,
                                         &weight) == 0);
    assert(weight == DM1_V1_F0140_POTION_WEIGHT_PC34);

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[5].thing,
                                         &weight) == 0);
    assert(weight == 3 + (4 << 1));

    assert(F0140_DUNGEON_GetObjectWeight(&world, records[6].thing,
                                         &weight) == 0);
    assert(weight == 50 + 11 + 50 + 2 + 1);

    weight = 123;
    assert(DM1_V1_Dungeon_GetObjectWeightF0140Pc34Compat(
               &world, DM1_V1_F0140_THING_NONE_PC34, &weight) == -1);
    assert(weight == 0);

    records[9].nextThing = records[8].thing;
    assert(F0140_DUNGEON_GetObjectWeight(&world, records[8].thing,
                                         &weight) == -1);

    assert(F0140_DUNGEON_GetObjectWeight(0, records[0].thing, &weight) == -1);
    assert(F0140_DUNGEON_GetObjectWeight(&world, records[0].thing, 0) == -1);

    return 0;
}
