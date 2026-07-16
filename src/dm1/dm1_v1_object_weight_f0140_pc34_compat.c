#include "dm1_v1_object_weight_f0140_pc34_compat.h"

static int is_end(uint16_t thing)
{
    return thing == DM1_V1_F0140_THING_END_OF_LIST_PC34 ||
           thing == DM1_V1_F0140_THING_NONE_PC34;
}

static const DM1_V1_F0140_ObjectRecordPc34* find_record(
    const DM1_V1_F0140_ObjectWorldPc34* world,
    uint16_t thing)
{
    size_t i;

    if (!world || !world->records || is_end(thing)) {
        return 0;
    }
    for (i = 0; i < world->recordCount; ++i) {
        if (world->records[i].thing == thing) {
            return &world->records[i];
        }
    }
    return 0;
}

static int table_weight(const uint8_t* weights,
                        size_t count,
                        uint8_t objectType,
                        int* outWeight)
{
    if (!weights || objectType >= count || !outWeight) {
        return -1;
    }
    *outWeight = (int)weights[objectType];
    return 0;
}

static int object_weight_recursive(
    const DM1_V1_F0140_ObjectWorldPc34* world,
    uint16_t thing,
    size_t depth,
    int* outWeight)
{
    const DM1_V1_F0140_ObjectRecordPc34* record;
    int weight;

    if (!world || !outWeight || depth > world->recordCount) {
        return -1;
    }
    record = find_record(world, thing);
    if (!record) {
        return -1;
    }

    switch (record->thingType) {
    case DM1_V1_F0140_THING_TYPE_WEAPON_PC34:
        return table_weight(world->weaponWeights, world->weaponWeightCount,
                            record->objectType, outWeight);

    case DM1_V1_F0140_THING_TYPE_ARMOUR_PC34:
        return table_weight(world->armourWeights, world->armourWeightCount,
                            record->objectType, outWeight);

    case DM1_V1_F0140_THING_TYPE_SCROLL_PC34:
        *outWeight = DM1_V1_F0140_SCROLL_WEIGHT_PC34;
        return 0;

    case DM1_V1_F0140_THING_TYPE_POTION_PC34:
        *outWeight =
            record->objectType == DM1_V1_F0140_EMPTY_FLASK_POTION_TYPE_PC34
                ? DM1_V1_F0140_EMPTY_FLASK_WEIGHT_PC34
                : DM1_V1_F0140_POTION_WEIGHT_PC34;
        return 0;

    case DM1_V1_F0140_THING_TYPE_CONTAINER_PC34:
        weight = DM1_V1_F0140_CONTAINER_BASE_WEIGHT_PC34;
        thing = record->containerSlotHead;
        while (!is_end(thing)) {
            int childWeight = 0;
            const DM1_V1_F0140_ObjectRecordPc34* child;

            if (object_weight_recursive(world, thing, depth + 1,
                                        &childWeight) != 0) {
                return -1;
            }
            child = find_record(world, thing);
            if (!child) {
                return -1;
            }
            weight += childWeight;
            thing = child->nextThing;
            if (++depth > world->recordCount) {
                return -1;
            }
        }
        *outWeight = weight;
        return 0;

    case DM1_V1_F0140_THING_TYPE_JUNK_PC34:
        if (table_weight(world->junkWeights, world->junkWeightCount,
                         record->objectType, &weight) != 0) {
            return -1;
        }
        if (record->objectType == DM1_V1_F0140_JUNK_WATERSKIN_TYPE_PC34) {
            weight += (int)record->chargeCount << 1;
        }
        *outWeight = weight;
        return 0;

    default:
        return -1;
    }
}

const char* DM1_V1_F0140_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1082-1133 F0140_DUNGEON_GetObjectWeight\n"
        "DUNGEON.C:1103-1130 selects weapon, armour, junk, scroll, potion, "
        "and container weights\n"
        "DUNGEON.C:1114-1120 gives containers weight 50 plus linked Slot "
        "contents via F0159 next-Thing traversal";
}

int F0140_DUNGEON_GetObjectWeight(
    const DM1_V1_F0140_ObjectWorldPc34* world,
    uint16_t thing,
    int* outWeight)
{
    if (outWeight) {
        *outWeight = 0;
    }
    if (!world || !outWeight) {
        return -1;
    }
    return object_weight_recursive(world, thing, 0, outWeight);
}

int DM1_V1_Dungeon_GetObjectWeightF0140Pc34Compat(
    const DM1_V1_F0140_ObjectWorldPc34* world,
    uint16_t thing,
    int* outWeight)
{
    return F0140_DUNGEON_GetObjectWeight(world, thing, outWeight);
}
