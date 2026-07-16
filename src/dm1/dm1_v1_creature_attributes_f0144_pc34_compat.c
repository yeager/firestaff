#include "dm1_v1_creature_attributes_f0144_pc34_compat.h"

const char* DM1_V1_F0144_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1249 F0144_DUNGEON_GetCreatureAttributes(P0241_T_Thing)\n"
        "DUNGEON.C:1257 loads the C04 GROUP record for the Thing\n"
        "DUNGEON.C:G0243_as_Graphic559_CreatureInfo supplies the creature "
        "attribute mask consumed by MENU.C, TIMELINE.C, and MOVESENS.C";
}

uint16_t DM1_V1_F0144_MakeThingPc34(unsigned int thingType,
                                    unsigned int thingIndex)
{
    return (uint16_t)(((thingType & 0x0fu) << 10) | (thingIndex & 0x03ffu));
}

unsigned int DM1_V1_F0144_ThingTypePc34(uint16_t thing)
{
    return (unsigned int)((thing & 0x3c00u) >> 10);
}

unsigned int DM1_V1_F0144_ThingIndexPc34(uint16_t thing)
{
    return (unsigned int)(thing & 0x03ffu);
}

int DM1_V1_Dungeon_GetCreatureTypeAttributesF0144Pc34Compat(
    const DM1_V1_F0144_CreatureInfoPc34* creatureInfos,
    size_t creatureInfoCount,
    int creatureType)
{
    if (!creatureInfos || creatureType < 0 ||
        creatureInfoCount > DM1_V1_F0144_CREATURE_TYPE_COUNT_PC34 ||
        (size_t)creatureType >= creatureInfoCount) {
        return 0;
    }
    return (int)creatureInfos[creatureType].attributes;
}

int F0144_DUNGEON_GetCreatureAttributes(
    const DM1_V1_F0144_DungeonWorldPc34* world,
    uint16_t thing)
{
    unsigned int groupIndex;
    const DM1_V1_F0144_GroupRecordPc34* group;

    if (!world || !world->groups || !world->creatureInfos ||
        thing == DM1_V1_F0144_THING_NONE_PC34 ||
        thing == DM1_V1_F0144_THING_END_OF_LIST_PC34 ||
        DM1_V1_F0144_ThingTypePc34(thing) !=
            DM1_V1_F0144_THING_TYPE_GROUP_PC34) {
        return 0;
    }

    groupIndex = DM1_V1_F0144_ThingIndexPc34(thing);
    if (groupIndex >= world->groupCount) {
        return 0;
    }

    group = &world->groups[groupIndex];
    return DM1_V1_Dungeon_GetCreatureTypeAttributesF0144Pc34Compat(
        world->creatureInfos,
        world->creatureInfoCount,
        (int)group->creatureType);
}

int DM1_V1_Dungeon_GetCreatureAttributesF0144Pc34Compat(
    const DM1_V1_F0144_DungeonWorldPc34* world,
    uint16_t thing)
{
    return F0144_DUNGEON_GetCreatureAttributes(world, thing);
}
