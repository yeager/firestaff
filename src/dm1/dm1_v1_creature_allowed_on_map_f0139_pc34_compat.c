#include "dm1_v1_creature_allowed_on_map_f0139_pc34_compat.h"

const char* DM1_V1_F0139_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1052 F0139_DUNGEON_IsCreatureAllowedOnMap(P0234_T_Thing, "
        "P0235_ui_MapIndex)\n"
        "DUNGEON.C:1057-1062 uses a MAP pointer, a byte pointer into the "
        "map allowed-creature list, a counter, and the GROUP creature type\n"
        "MOVESENS.C F0267 consumes this decision to remove groups that are "
        "not allowed on the destination map after pit/teleporter movement";
}

uint16_t DM1_V1_F0139_MakeThingPc34(unsigned int thingType,
                                    unsigned int thingIndex)
{
    return (uint16_t)(((thingType & 0x0fu) << 10) | (thingIndex & 0x03ffu));
}

unsigned int DM1_V1_F0139_ThingTypePc34(uint16_t thing)
{
    return (unsigned int)((thing & 0x3c00u) >> 10);
}

unsigned int DM1_V1_F0139_ThingIndexPc34(uint16_t thing)
{
    return (unsigned int)(thing & 0x03ffu);
}

int DM1_V1_Dungeon_IsCreatureTypeAllowedOnMapF0139Pc34Compat(
    const uint8_t* allowedCreatureTypes,
    size_t allowedCreatureTypeCount,
    uint8_t creatureType)
{
    size_t i;

    if (!allowedCreatureTypes ||
        allowedCreatureTypeCount >
            DM1_V1_F0139_MAP_ALLOWED_CREATURE_CAPACITY_PC34) {
        return 0;
    }
    for (i = 0; i < allowedCreatureTypeCount; ++i) {
        if (allowedCreatureTypes[i] == creatureType) {
            return 1;
        }
    }
    return 0;
}

int F0139_DUNGEON_IsCreatureAllowedOnMap(
    const DM1_V1_F0139_DungeonWorldPc34* world,
    uint16_t thing,
    int mapIndex)
{
    unsigned int groupIndex;
    const DM1_V1_F0139_GroupRecordPc34* group;
    const DM1_V1_F0139_MapRecordPc34* map;

    if (!world || !world->groups || !world->maps || mapIndex < 0 ||
        (size_t)mapIndex >= world->mapCount ||
        thing == DM1_V1_F0139_THING_NONE_PC34 ||
        thing == DM1_V1_F0139_THING_END_OF_LIST_PC34 ||
        DM1_V1_F0139_ThingTypePc34(thing) !=
            DM1_V1_F0139_THING_TYPE_GROUP_PC34) {
        return 0;
    }

    groupIndex = DM1_V1_F0139_ThingIndexPc34(thing);
    if (groupIndex >= world->groupCount) {
        return 0;
    }

    group = &world->groups[groupIndex];
    map = &world->maps[mapIndex];
    return DM1_V1_Dungeon_IsCreatureTypeAllowedOnMapF0139Pc34Compat(
        map->allowedCreatureTypes,
        map->allowedCreatureTypeCount,
        group->creatureType);
}

int DM1_V1_Dungeon_IsCreatureAllowedOnMapF0139Pc34Compat(
    const DM1_V1_F0139_DungeonWorldPc34* world,
    uint16_t thing,
    int mapIndex)
{
    return F0139_DUNGEON_IsCreatureAllowedOnMap(world, thing, mapIndex);
}
