#ifndef FIRESTAFF_DM1_V1_OBJECT_WORLD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_WORLD_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_OBJ_NAME_COUNT 199
#define DM1_OBJ_INFO_COUNT 180
#define DM1_WEAPON_INFO_COUNT 46
#define DM1_ARMOUR_INFO_COUNT 58
#define DM1_CREATURE_INFO_COUNT 27
#define DM1_DOOR_INFO_COUNT 4

typedef struct {
    uint16_t type;
    uint16_t weight;
    int16_t icon_index;
} DM1_V1_ObjectWorldObjectInfoPc34;

typedef struct {
    uint16_t wclass;
    uint16_t strength;
    uint16_t kineticEnergy;
    int16_t range;
} DM1_V1_ObjectWorldWeaponInfoPc34;

typedef struct {
    uint16_t aclass;
    uint16_t defense;
    uint16_t weight;
} DM1_V1_ObjectWorldArmourInfoPc34;

typedef struct {
    uint16_t type;
    uint16_t hp;
    uint16_t attack;
    uint16_t defense;
    uint16_t speed;
    uint8_t side_attack;
    uint8_t preferred_distance;
} DM1_V1_ObjectWorldCreatureInfoPc34;

typedef struct {
    uint16_t type;
    uint16_t resistance;
    bool destroyable;
} DM1_V1_ObjectWorldDoorInfoPc34;

typedef struct {
    char* obj_names[DM1_OBJ_NAME_COUNT];
    DM1_V1_ObjectWorldObjectInfoPc34 obj_info[DM1_OBJ_INFO_COUNT];
    DM1_V1_ObjectWorldWeaponInfoPc34 weapons[DM1_WEAPON_INFO_COUNT];
    DM1_V1_ObjectWorldArmourInfoPc34 armour[DM1_ARMOUR_INFO_COUNT];
    DM1_V1_ObjectWorldCreatureInfoPc34 creatures[DM1_CREATURE_INFO_COUNT];
    DM1_V1_ObjectWorldDoorInfoPc34 doors[DM1_DOOR_INFO_COUNT];
    bool loaded;
} DM1_V1_ObjectWorldStatePc34;

void DM1_V1_ObjectWorld_InitPc34Compat(DM1_V1_ObjectWorldStatePc34* state);
bool DM1_V1_ObjectWorld_LoadObjectNamesPc34Compat(DM1_V1_ObjectWorldStatePc34* state, const uint8_t* data, size_t size);
const char* DM1_V1_ObjectWorld_GetObjectNamePc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx);
const DM1_V1_ObjectWorldObjectInfoPc34* DM1_V1_ObjectWorld_GetObjectInfoPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx);
const DM1_V1_ObjectWorldWeaponInfoPc34* DM1_V1_ObjectWorld_GetWeaponPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx);
const DM1_V1_ObjectWorldArmourInfoPc34* DM1_V1_ObjectWorld_GetArmourPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx);
const DM1_V1_ObjectWorldCreatureInfoPc34* DM1_V1_ObjectWorld_GetCreaturePc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx);
const DM1_V1_ObjectWorldDoorInfoPc34* DM1_V1_ObjectWorld_GetDoorPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx);
void DM1_V1_ObjectWorld_CleanupPc34Compat(DM1_V1_ObjectWorldStatePc34* state);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_OBJECT_WORLD_PC34_COMPAT_H */
