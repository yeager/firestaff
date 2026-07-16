#ifndef FIRESTAFF_DM1_V1_OBJECT_INFO_INDEX_F0141_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_INFO_INDEX_F0141_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0141_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0141_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0141_THING_TYPE_WEAPON_PC34 = 5,
    DM1_V1_F0141_THING_TYPE_ARMOUR_PC34 = 6,
    DM1_V1_F0141_THING_TYPE_SCROLL_PC34 = 7,
    DM1_V1_F0141_THING_TYPE_POTION_PC34 = 8,
    DM1_V1_F0141_THING_TYPE_CONTAINER_PC34 = 9,
    DM1_V1_F0141_THING_TYPE_JUNK_PC34 = 10,
    DM1_V1_F0141_OBJECT_INFO_COUNT_PC34 = 180
};

typedef struct {
    uint16_t thing;
    uint8_t thingType;
    uint8_t objectType;
} DM1_V1_F0141_ObjectRecordPc34;

typedef struct {
    const DM1_V1_F0141_ObjectRecordPc34* records;
    size_t recordCount;
} DM1_V1_F0141_ObjectWorldPc34;

const char* DM1_V1_F0141_SourceEvidencePc34(void);

int F0141_DUNGEON_GetObjectInfoIndex(
    const DM1_V1_F0141_ObjectWorldPc34* world,
    uint16_t thing);

int DM1_V1_Dungeon_GetObjectInfoIndexF0141Pc34Compat(
    const DM1_V1_F0141_ObjectWorldPc34* world,
    uint16_t thing);

int DM1_V1_Dungeon_GetObjectInfoIndexForTypeF0141Pc34Compat(
    int thingType,
    int objectType);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_OBJECT_INFO_INDEX_F0141_PC34_COMPAT_H */
