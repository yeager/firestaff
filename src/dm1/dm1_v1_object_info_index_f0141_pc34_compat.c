#include "dm1_v1_object_info_index_f0141_pc34_compat.h"

static const DM1_V1_F0141_ObjectRecordPc34* find_record(
    const DM1_V1_F0141_ObjectWorldPc34* world,
    uint16_t thing)
{
    size_t i;

    if (!world || !world->records ||
        thing == DM1_V1_F0141_THING_NONE_PC34 ||
        thing == DM1_V1_F0141_THING_END_OF_LIST_PC34) {
        return 0;
    }
    for (i = 0; i < world->recordCount; ++i) {
        if (world->records[i].thing == thing) {
            return &world->records[i];
        }
    }
    return 0;
}

const char* DM1_V1_F0141_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1136-1165 F0141_DUNGEON_GetObjectInfoIndex maps an "
        "object Thing to G0237_as_Graphic559_ObjectInfo\n"
        "DUNGEON.C:G0237 rows 0..179 are grouped as scroll, container, "
        "potion, weapon, armour, and junk object-info families\n"
        "DUNVIEW.C F0115/G0209 consumes this ObjectInfo index for native "
        "object material selection";
}

int DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
    int thingType,
    int objectType)
{
    int objectInfoIndex;

    if (objectType < 0) {
        return -1;
    }

    switch (thingType) {
    case DM1_V1_F0141_THING_TYPE_SCROLL_PC34:
        objectInfoIndex = 0;
        break;
    case DM1_V1_F0141_THING_TYPE_CONTAINER_PC34:
        if (objectType > 0) return -1;
        objectInfoIndex = 1 + objectType;
        break;
    case DM1_V1_F0141_THING_TYPE_POTION_PC34:
        if (objectType > 20) return -1;
        objectInfoIndex = 2 + objectType;
        break;
    case DM1_V1_F0141_THING_TYPE_WEAPON_PC34:
        if (objectType > 45) return -1;
        objectInfoIndex = 23 + objectType;
        break;
    case DM1_V1_F0141_THING_TYPE_ARMOUR_PC34:
        if (objectType > 57) return -1;
        objectInfoIndex = 69 + objectType;
        break;
    case DM1_V1_F0141_THING_TYPE_JUNK_PC34:
        if (objectType > 52) return -1;
        objectInfoIndex = 127 + objectType;
        break;
    default:
        return -1;
    }

    if (objectInfoIndex < 0 ||
        objectInfoIndex >= DM1_V1_F0141_OBJECT_INFO_COUNT_PC34) {
        return -1;
    }
    return objectInfoIndex;
}

int F0141_DUNGEON_GetObjectInfoIndex(
    const DM1_V1_F0141_ObjectWorldPc34* world,
    uint16_t thing)
{
    const DM1_V1_F0141_ObjectRecordPc34* record = find_record(world, thing);

    if (!record) {
        return -1;
    }
    return DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
        record->thingType,
        record->objectType);
}

int DM1_V1_Dungeon_GetObjectInfoIndexF0141Pc34Compat(
    const DM1_V1_F0141_ObjectWorldPc34* world,
    uint16_t thing)
{
    return F0141_DUNGEON_GetObjectInfoIndex(world, thing);
}
