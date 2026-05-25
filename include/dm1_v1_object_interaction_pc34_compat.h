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
} M11_WorldObject;

#define M11_MAX_WORLD_OBJECTS 512
#define M11_MAX_FLOOR_OBJECTS 64

typedef struct {
    M11_WorldObject floorObjects[M11_MAX_FLOOR_OBJECTS];
    int floorCount;
} M11_FloorCell;

typedef struct {
    M11_WorldObject objects[M11_MAX_WORLD_OBJECTS];
    int objectCount;
    M11_FloorCell floors[16][32][32];
} M11_ObjectState;

void m11_obj_init(M11_ObjectState* s);
int m11_obj_spawn(M11_ObjectState* s, int type, int x, int y, int level, int weight);
int m11_obj_pickup(M11_ObjectState* s, int objIdx, int* outWeight);
int m11_obj_drop(M11_ObjectState* s, int objIdx, int x, int y, int level);

/* m11_obj_use — OBJECT.C F0349 use/consume delegation.
 * Delegates to dm1_v1_inventory_consumables_pc34_compat.c for
 * potions, food, and water/junk consumption. Equipment types
 * (weapon/armor/accessory) return 0 as they are handled by
 * m11_inventory_equip() slot system.
 * champData: DM1ConsumableChampionPc34* champion stats snapshot.
 * result: DM1ConsumableResultPc34* output, caller commits deltas.
 * Returns 1 on successful consumption, 0 if not consumable. */
int m11_obj_use(M11_ObjectState* s, int champIdx, int objIdx,
                DM1ConsumableChampionPc34* champData,
                DM1ConsumableResultPc34* result);

int m11_obj_throw(M11_ObjectState* s, int objIdx, int dir, int force);
int m11_obj_activate(M11_ObjectState* s, int objIdx);
int m11_obj_examine(const M11_ObjectState* s, int objIdx, char* desc, int descLen);
int m11_obj_get_at(const M11_ObjectState* s, int x, int y, int level, int* outIndices, int maxOut);
int m11_obj_remove(M11_ObjectState* s, int objIdx);
const char* m11_obj_type_name(int type);
int m11_obj_is_valid(const M11_ObjectState* s, int objIdx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_OBJECT_INTERACTION_PC34_COMPAT_H */
