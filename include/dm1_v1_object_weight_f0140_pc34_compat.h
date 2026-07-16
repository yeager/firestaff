#ifndef FIRESTAFF_DM1_V1_OBJECT_WEIGHT_F0140_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_WEIGHT_F0140_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0140_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0140_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0140_THING_TYPE_WEAPON_PC34 = 5,
    DM1_V1_F0140_THING_TYPE_ARMOUR_PC34 = 6,
    DM1_V1_F0140_THING_TYPE_SCROLL_PC34 = 7,
    DM1_V1_F0140_THING_TYPE_POTION_PC34 = 8,
    DM1_V1_F0140_THING_TYPE_CONTAINER_PC34 = 9,
    DM1_V1_F0140_THING_TYPE_JUNK_PC34 = 10,
    DM1_V1_F0140_CONTAINER_BASE_WEIGHT_PC34 = 50,
    DM1_V1_F0140_SCROLL_WEIGHT_PC34 = 1,
    DM1_V1_F0140_EMPTY_FLASK_POTION_TYPE_PC34 = 0,
    DM1_V1_F0140_EMPTY_FLASK_WEIGHT_PC34 = 1,
    DM1_V1_F0140_POTION_WEIGHT_PC34 = 3,
    DM1_V1_F0140_JUNK_WATERSKIN_TYPE_PC34 = 1
};

typedef struct {
    uint16_t thing;
    uint8_t thingType;
    uint8_t objectType;
    uint8_t chargeCount;
    uint16_t nextThing;
    uint16_t containerSlotHead;
} DM1_V1_F0140_ObjectRecordPc34;

typedef struct {
    const DM1_V1_F0140_ObjectRecordPc34* records;
    size_t recordCount;
    const uint8_t* weaponWeights;
    size_t weaponWeightCount;
    const uint8_t* armourWeights;
    size_t armourWeightCount;
    const uint8_t* junkWeights;
    size_t junkWeightCount;
} DM1_V1_F0140_ObjectWorldPc34;

const char* DM1_V1_F0140_SourceEvidencePc34(void);

int F0140_DUNGEON_GetObjectWeight(
    const DM1_V1_F0140_ObjectWorldPc34* world,
    uint16_t thing,
    int* outWeight);

int DM1_V1_Dungeon_GetObjectWeightF0140Pc34Compat(
    const DM1_V1_F0140_ObjectWorldPc34* world,
    uint16_t thing,
    int* outWeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_OBJECT_WEIGHT_F0140_PC34_COMPAT_H */
