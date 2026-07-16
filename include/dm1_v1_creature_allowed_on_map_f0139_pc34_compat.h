#ifndef FIRESTAFF_DM1_V1_CREATURE_ALLOWED_ON_MAP_F0139_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CREATURE_ALLOWED_ON_MAP_F0139_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0139_THING_TYPE_GROUP_PC34 = 4,
    DM1_V1_F0139_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0139_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0139_MAP_ALLOWED_CREATURE_CAPACITY_PC34 = 16
};

typedef struct {
    uint8_t creatureType;
} DM1_V1_F0139_GroupRecordPc34;

typedef struct {
    const uint8_t* allowedCreatureTypes;
    size_t allowedCreatureTypeCount;
} DM1_V1_F0139_MapRecordPc34;

typedef struct {
    const DM1_V1_F0139_GroupRecordPc34* groups;
    size_t groupCount;
    const DM1_V1_F0139_MapRecordPc34* maps;
    size_t mapCount;
} DM1_V1_F0139_DungeonWorldPc34;

const char* DM1_V1_F0139_SourceEvidencePc34(void);

uint16_t DM1_V1_F0139_MakeThingPc34(unsigned int thingType,
                                    unsigned int thingIndex);
unsigned int DM1_V1_F0139_ThingTypePc34(uint16_t thing);
unsigned int DM1_V1_F0139_ThingIndexPc34(uint16_t thing);

int F0139_DUNGEON_IsCreatureAllowedOnMap(
    const DM1_V1_F0139_DungeonWorldPc34* world,
    uint16_t thing,
    int mapIndex);

int DM1_V1_Dungeon_IsCreatureAllowedOnMapF0139Pc34Compat(
    const DM1_V1_F0139_DungeonWorldPc34* world,
    uint16_t thing,
    int mapIndex);

int DM1_V1_Dungeon_IsCreatureTypeAllowedOnMapF0139Pc34Compat(
    const uint8_t* allowedCreatureTypes,
    size_t allowedCreatureTypeCount,
    uint8_t creatureType);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CREATURE_ALLOWED_ON_MAP_F0139_PC34_COMPAT_H */
