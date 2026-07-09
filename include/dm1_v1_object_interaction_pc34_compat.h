#ifndef FIRESTAFF_DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_inventory_consumables_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_OBJ_PICKUP = 0,
    DM1_OBJ_DROP,
    DM1_OBJ_USE,
    DM1_OBJ_THROW,
    DM1_OBJ_ACTIVATE,
    DM1_OBJ_EXAMINE,
    DM1_OBJ_OPEN,
    DM1_OBJ_CLOSE,
    DM1_OBJ_ACTION_COUNT
};

enum {
    DM1_OBJTYPE_NONE = 0,
    DM1_OBJTYPE_WEAPON,
    DM1_OBJTYPE_ARMOR,
    DM1_OBJTYPE_POTION,
    DM1_OBJTYPE_SCROLL,
    DM1_OBJTYPE_KEY,
    DM1_OBJTYPE_FOOD,
    DM1_OBJTYPE_WATER,
    DM1_OBJTYPE_TORCH,
    DM1_OBJTYPE_CHEST,
    DM1_OBJTYPE_LEVER,
    DM1_OBJTYPE_BUTTON,
    DM1_OBJTYPE_ALCOVE,
    DM1_OBJTYPE_MISC,
    DM1_OBJTYPE_COUNT
};

typedef struct {
    int objectType;
    int objectId;
    int weight;
    int stackable;
    int stackCount;
    int usable;
    int throwable;
    int activatable;
    int x;
    int y;
    int level;
} DM1_V1_WorldObjectPc34;

#define DM1_V1_MAX_WORLD_OBJECTS_PC34 512
#define DM1_V1_MAX_FLOOR_OBJECTS_PC34 64

typedef struct {
    DM1_V1_WorldObjectPc34 floorObjects[DM1_V1_MAX_FLOOR_OBJECTS_PC34];
    int floorCount;
} DM1_V1_FloorCellPc34;

typedef struct {
    DM1_V1_WorldObjectPc34 objects[DM1_V1_MAX_WORLD_OBJECTS_PC34];
    int objectCount;
    DM1_V1_FloorCellPc34 floors[16][32][32];
} DM1_V1_ObjectStatePc34;

void DM1_V1_Object_InitPc34Compat(DM1_V1_ObjectStatePc34* s);
int DM1_V1_Object_SpawnPc34Compat(DM1_V1_ObjectStatePc34* s, int type, int x, int y, int level, int weight);
int DM1_V1_Object_PickupPc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx, int* outWeight);
int DM1_V1_Object_DropPc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx, int x, int y, int level);

/* DM1_V1_Object_UsePc34Compat — OBJECT.C F0349 use/consume delegation.
 * Delegates to dm1_v1_inventory_consumables_pc34_compat.c for
 * potions, food, and water/junk consumption. Equipment types
 * (weapon/armor/accessory) return 0 as they are handled by
 * DM1_V1_Inventory_EquipPc34Compat() slot system.
 * champData: DM1ConsumableChampionPc34* champion stats snapshot.
 * result: DM1ConsumableResultPc34* output, caller commits deltas.
 * Returns 1 on successful consumption, 0 if not consumable. */
int DM1_V1_Object_UsePc34Compat(DM1_V1_ObjectStatePc34* s, int champIdx, int objIdx,
                DM1ConsumableChampionPc34* champData,
                DM1ConsumableResultPc34* result);

int DM1_V1_Object_ThrowPc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx, int dir, int force);
int DM1_V1_Object_ActivatePc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx);
int DM1_V1_Object_ExaminePc34Compat(const DM1_V1_ObjectStatePc34* s, int objIdx, char* desc, int descLen);
int DM1_V1_Object_GetAtPc34Compat(const DM1_V1_ObjectStatePc34* s, int x, int y, int level, int* outIndices, int maxOut);
int DM1_V1_Object_RemovePc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx);
const char* DM1_V1_Object_TypeNamePc34Compat(int type);
int DM1_V1_Object_IsValidPc34Compat(const DM1_V1_ObjectStatePc34* s, int objIdx);

typedef DM1_V1_WorldObjectPc34 M11_WorldObject;
typedef DM1_V1_FloorCellPc34 M11_FloorCell;
typedef DM1_V1_ObjectStatePc34 M11_ObjectState;

#define M11_MAX_WORLD_OBJECTS DM1_V1_MAX_WORLD_OBJECTS_PC34
#define M11_MAX_FLOOR_OBJECTS DM1_V1_MAX_FLOOR_OBJECTS_PC34

#define m11_obj_init DM1_V1_Object_InitPc34Compat
#define m11_obj_spawn DM1_V1_Object_SpawnPc34Compat
#define m11_obj_pickup DM1_V1_Object_PickupPc34Compat
#define m11_obj_drop DM1_V1_Object_DropPc34Compat
#define m11_obj_use DM1_V1_Object_UsePc34Compat
#define m11_obj_throw DM1_V1_Object_ThrowPc34Compat
#define m11_obj_activate DM1_V1_Object_ActivatePc34Compat
#define m11_obj_examine DM1_V1_Object_ExaminePc34Compat
#define m11_obj_get_at DM1_V1_Object_GetAtPc34Compat
#define m11_obj_remove DM1_V1_Object_RemovePc34Compat
#define m11_obj_type_name DM1_V1_Object_TypeNamePc34Compat
#define m11_obj_is_valid DM1_V1_Object_IsValidPc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H */
