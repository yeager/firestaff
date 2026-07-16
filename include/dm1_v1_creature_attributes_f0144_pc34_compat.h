#ifndef FIRESTAFF_DM1_V1_CREATURE_ATTRIBUTES_F0144_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CREATURE_ATTRIBUTES_F0144_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0144_THING_TYPE_GROUP_PC34 = 4,
    DM1_V1_F0144_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0144_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0144_CREATURE_TYPE_COUNT_PC34 = 27,
    DM1_V1_F0144_ATTR_NON_MATERIAL_PC34 = 0x0040u,
    DM1_V1_F0144_ATTR_DROP_FIXED_POSSESSIONS_PC34 = 0x0200u,
    DM1_V1_F0144_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34 = 0x0400u,
    DM1_V1_F0144_ATTR_SEE_INVISIBLE_PC34 = 0x0800u,
    DM1_V1_F0144_ATTR_NIGHT_VISION_PC34 = 0x1000u,
    DM1_V1_F0144_ATTR_ARCHENEMY_PC34 = 0x2000u
};

typedef struct {
    uint8_t creatureType;
} DM1_V1_F0144_GroupRecordPc34;

typedef struct {
    uint16_t attributes;
} DM1_V1_F0144_CreatureInfoPc34;

typedef struct {
    const DM1_V1_F0144_GroupRecordPc34* groups;
    size_t groupCount;
    const DM1_V1_F0144_CreatureInfoPc34* creatureInfos;
    size_t creatureInfoCount;
} DM1_V1_F0144_DungeonWorldPc34;

const char* DM1_V1_F0144_SourceEvidencePc34(void);

uint16_t DM1_V1_F0144_MakeThingPc34(unsigned int thingType,
                                    unsigned int thingIndex);
unsigned int DM1_V1_F0144_ThingTypePc34(uint16_t thing);
unsigned int DM1_V1_F0144_ThingIndexPc34(uint16_t thing);

int F0144_DUNGEON_GetCreatureAttributes(
    const DM1_V1_F0144_DungeonWorldPc34* world,
    uint16_t thing);

int DM1_V1_Dungeon_GetCreatureAttributesF0144Pc34Compat(
    const DM1_V1_F0144_DungeonWorldPc34* world,
    uint16_t thing);

int DM1_V1_Dungeon_GetCreatureTypeAttributesF0144Pc34Compat(
    const DM1_V1_F0144_CreatureInfoPc34* creatureInfos,
    size_t creatureInfoCount,
    int creatureType);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_ATTRIBUTES_F0144_PC34_COMPAT_H */
