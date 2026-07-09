#include "dm1_v1_object_interaction_pc34_compat.h"
#include "dm1_v1_inventory_consumables_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include <string.h>
#include <stdio.h>

enum {
    DM1_OBJ_USE_ICON_WATER_PC34 = 8,
    DM1_OBJ_USE_ICON_WATERSKIN_PC34 = 9,
    DM1_OBJ_USE_ICON_FOOD_FIRST_PC34 = 168,
    DM1_OBJ_USE_ICON_FOOD_LAST_PC34 = 175,
    DM1_OBJ_USE_POTION_ROS_PC34 = 6,
    DM1_OBJ_USE_POTION_WATER_FLASK_PC34 = 15
};

static int dm1_v1_obj_use_food_icon_from_object(const DM1_V1_WorldObjectPc34* obj)
{
    if (obj && obj->weight >= DM1_OBJ_USE_ICON_FOOD_FIRST_PC34 &&
        obj->weight <= DM1_OBJ_USE_ICON_FOOD_LAST_PC34) {
        return obj->weight;
    }
    return DM1_OBJ_USE_ICON_FOOD_FIRST_PC34;
}

static int dm1_v1_obj_use_water_icon_from_object(const DM1_V1_WorldObjectPc34* obj)
{
    if (obj && (obj->weight == DM1_OBJ_USE_ICON_WATER_PC34 ||
                obj->weight == DM1_OBJ_USE_ICON_WATERSKIN_PC34)) {
        return obj->weight;
    }
    return DM1_OBJ_USE_ICON_WATERSKIN_PC34;
}

static int dm1_v1_obj_use_potion_type_from_object(const DM1_V1_WorldObjectPc34* obj)
{
    if (obj && obj->stackCount >= DM1_OBJ_USE_POTION_ROS_PC34 &&
        obj->stackCount <= DM1_OBJ_USE_POTION_WATER_FLASK_PC34) {
        return obj->stackCount;
    }
    return 0;
}

void DM1_V1_Object_InitPc34Compat(DM1_V1_ObjectStatePc34* s) {
    if (!s) return;
    memset(s, 0, sizeof(DM1_V1_ObjectStatePc34));
    s->objectCount = 0;
    for (int l = 0; l < 16; l++) {
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                s->floors[l][y][x].floorCount = 0;
            }
        }
    }
}

int DM1_V1_Object_SpawnPc34Compat(DM1_V1_ObjectStatePc34* s, int type, int x, int y, int level, int weight) {
    if (!s) return -1;
    if (s->objectCount >= DM1_V1_MAX_WORLD_OBJECTS_PC34) return -1;
    if (x < 0 || x >= 32 || y < 0 || y >= 32 || level < 0 || level >= 16) return -1;

    int idx = s->objectCount;
    DM1_V1_WorldObjectPc34* obj = &s->objects[idx];
    obj->objectType = type;
    obj->objectId = idx;
    obj->weight = weight;
    obj->stackable = 0;
    obj->stackCount = 1;
    obj->usable = (type == DM1_OBJTYPE_POTION || type == DM1_OBJTYPE_FOOD || type == DM1_OBJTYPE_WATER || type == DM1_OBJTYPE_TORCH);
    obj->throwable = (type == DM1_OBJTYPE_WEAPON || type == DM1_OBJTYPE_POTION || type == DM1_OBJTYPE_TORCH);
    obj->activatable = (type == DM1_OBJTYPE_LEVER || type == DM1_OBJTYPE_BUTTON || type == DM1_OBJTYPE_ALCOVE);
    obj->x = x;
    obj->y = y;
    obj->level = level;

    DM1_V1_FloorCellPc34* cell = &s->floors[level][y][x];
    if (cell->floorCount < DM1_V1_MAX_FLOOR_OBJECTS_PC34) {
        cell->floorObjects[cell->floorCount] = *obj;
        cell->floorCount++;
    }

    s->objectCount++;
    return idx;
}

int DM1_V1_Object_PickupPc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx, int* outWeight) {
    /* Source: DUNGEON.C:F0161_DUNGEON_GetSquareFirstThing (floor lookup),
     * DUNGEON.C:F0156_DUNGEON_GetThingData (thing data pointer).
     * The x=-1 / x=-2 marking convention (carried/in-flight) is derived
     * from examining how ReDMCSB clears leader-hand things after use
     * (PANEL.C:1891 F0298_CHAMPION_GetObjectRemovedFromLeaderHand) and
     * how thrown objects are placed into flight state. */
    if (!s || !DM1_V1_Object_IsValidPc34Compat(s, objIdx)) return -1;
    DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];
    if (obj->x == -1 || obj->x == -2) return -1; /* Already picked up or in flight */

    int x = obj->x;
    int y = obj->y;
    int level = obj->level;

    /* Remove from floor cell */
    DM1_V1_FloorCellPc34* cell = &s->floors[level][y][x];
    for (int i = 0; i < cell->floorCount; i++) {
        if (cell->floorObjects[i].objectId == objIdx) {
            for (int j = i; j < cell->floorCount - 1; j++) {
                cell->floorObjects[j] = cell->floorObjects[j + 1];
            }
            cell->floorCount--;
            break;
        }
    }

    obj->x = -1;
    obj->y = -1;
    obj->level = -1;

    if (outWeight) *outWeight = obj->weight;
    return 0;
}
int DM1_V1_Object_DropPc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx, int x, int y, int level) {
    /* Source: DUNGEON.C:F0140_DUNGEON_GetObjectWeight (weight lookup),
     * DUNGEON.C:F0159_DUNGEON_GetNextThing (thing list traversal),
     * DUNGEON.C:1111-1117 (container weight includes contents recursively).
     * The floor cell API is a simplification of ReDMCSB THING linked-list
     * system anchored in dungeon squares (DUNGEON square data). */
    if (!s || !DM1_V1_Object_IsValidPc34Compat(s, objIdx)) return -1;
    if (x < 0 || x >= 32 || y < 0 || y >= 32 || level < 0 || level >= 16) return -1;

    DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];
    obj->x = x;
    obj->y = y;
    obj->level = level;

    DM1_V1_FloorCellPc34* cell = &s->floors[level][y][x];
    if (cell->floorCount < DM1_V1_MAX_FLOOR_OBJECTS_PC34) {
        cell->floorObjects[cell->floorCount] = *obj;
        cell->floorCount++;
    }

    return 0;
}

/* ReDMCSB OBJECT.C F0349 — use or consume an object.
 * Delegating to module-specific handlers per object type.
 * Source: OBJECT.C + INVENTORY.C + CONSUM.C
 *
 * DM1_V1_Object_UsePc34Compat delegates to the consumables module for items that affect
 * champion stats (potions, food, junk/water). Equipment (weapons, armor,
 * accessories) is handled by the inventory slot system (DM1_V1_Inventory_EquipPc34Compat)
 * and does not go through this function — items in slots are used by
 * being equipped, not consumed. */
int DM1_V1_Object_UsePc34Compat(DM1_V1_ObjectStatePc34* s, int champIdx, int objIdx,
                DM1ConsumableChampionPc34* champData,
                DM1ConsumableResultPc34* result)
{
    (void)s;
    (void)champIdx;
    if (!DM1_V1_Object_IsValidPc34Compat(s, objIdx)) return -1;
    DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];

    switch (obj->objectType) {
        case DM1_OBJTYPE_POTION: {
            /* Potion: delegate to consumables module (CONSUM.C F0340).
             * champData carries current stat snapshot; result returns the
             * transformed champion values. Caller commits those values to the
             * actual champion state (health, stamina, mana, attrs).
             * Source: PANEL.C:1850-1917 F0349 potion application + VI wound cure. */
            if (!champData || !result) return 0;
            return dm1_inventory_consume_potion_pc34(champData,
                                                    dm1_v1_obj_use_potion_type_from_object(obj),
                                                    obj->weight /* power proxy */,
                                                    NULL /* woundMasks */,
                                                    0 /* woundMaskCount */,
                                                    result);
        }
        case DM1_OBJTYPE_FOOD: {
            /* Food: delegate to food consumption (CONSUM.C F0343).
             * obj->weight carries the food icon proxy (C168..C175) in this
             * compact object-state abstraction.
             * Source: PANEL.C:1918-1919 G0242 food amounts. */
            if (!champData || !result) return 0;
            return dm1_inventory_consume_food_junk_pc34(champData,
                                                        dm1_v1_obj_use_food_icon_from_object(obj),
                                                        result);
        }
        case DM1_OBJTYPE_WATER: {
            /* Water/junk: delegate to water consumption (CONSUM.C F0342).
             * obj->weight carries C008 water / C009 waterskin icon proxy and
             * obj->stackCount carries charge count for waterskins.
             * Source: PANEL.C:1824-1844 waterskin charge use. */
            if (!champData || !result) return 0;
            return dm1_inventory_consume_water_junk_pc34(champData,
                                                         dm1_v1_obj_use_water_icon_from_object(obj),
                                                         obj->stackCount,
                                                         result);
        }
        case DM1_OBJTYPE_SCROLL: {
            /* Scrolls are readable but not consumed via mouth-click in the
             * original PC 3.4 — they are read in the dungeon view or read
             * command. For mouth-click, return 0 (non-usable via this path).
             * The scroll reading mechanic is handled elsewhere (READ.C). */
            return 0;
        }
        case DM1_OBJTYPE_WEAPON:
        case DM1_OBJTYPE_ARMOR: {
            /* Equipment: handled by DM1_V1_Inventory_EquipPc34Compat() slot system.
             * Not consumed; mouth-click returns 0 (non-usable here).
             * Source: INVENTORY.C F0300-F0302 slot equip. */
            return 0;
        }
        default:
            return 0;
    }
}

int DM1_V1_Object_ThrowPc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx, int dir, int force) {
    /* Source: CLIKVIEW.C / DUNGEON.C -- thrown objects are marked as in-flight
     * (x=-2) until they land or are consumed. ReDMCSB thrown object handling
     * is in the click/viewport routing and THING projectile system. */
    (void)dir;
    (void)force;
    if (!s || !DM1_V1_Object_IsValidPc34Compat(s, objIdx)) return -1;
    DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];
    if (!obj->throwable) return 0;

    obj->x = -2;
    return 1;
}

int DM1_V1_Object_ActivatePc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx) {
    if (!s || !DM1_V1_Object_IsValidPc34Compat(s, objIdx)) return -1;
    DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];
    if (obj->activatable) {
        return 1;
    }
    return 0;
}

int DM1_V1_Object_ExaminePc34Compat(const DM1_V1_ObjectStatePc34* s, int objIdx, char* desc, int descLen) {
    /* Source: OBJECT.C:F0033_OBJECT_GetIconIndex (icon),
     * OBJECT.C:237 (G0352_apc_ObjectNames[name]),
     * PANEL.C:1444-1469 (weight display format WEIGHS X.Y KG.
     * via F0140/10 and F0140%10, CHAMDRAW.C:349-392).
     * Note: obj->weight is the stored spawn weight; ReDMCSB recomputes
     * weight from type-specific tables (G0238/G0239/G0241) per F0140.
     * This means the displayed weight may diverge from ReDMCSB for items
     * whose weight varies by type detail (e.g. waterskin by charges). */
    if (!s || !DM1_V1_Object_IsValidPc34Compat(s, objIdx) || !desc || descLen <= 0) return -1;
    const DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];
    const char* typeName = DM1_V1_Object_TypeNamePc34Compat(obj->objectType);
    snprintf(desc, descLen, "%s (Weight: %d)", typeName, obj->weight);
    return 0;
}

int DM1_V1_Object_GetAtPc34Compat(const DM1_V1_ObjectStatePc34* s, int x, int y, int level, int* outIndices, int maxOut) {
    if (!s || !outIndices) return 0;
    if (x < 0 || x >= 32 || y < 0 || y >= 32 || level < 0 || level >= 16) return 0;

    const DM1_V1_FloorCellPc34* cell = &s->floors[level][y][x];
    int count = 0;
    for (int i = 0; i < cell->floorCount && count < maxOut; i++) {
        outIndices[count++] = cell->floorObjects[i].objectId;
    }
    return count;
}

int DM1_V1_Object_RemovePc34Compat(DM1_V1_ObjectStatePc34* s, int objIdx) {
    if (!s || !DM1_V1_Object_IsValidPc34Compat(s, objIdx)) return -1;

    DM1_V1_WorldObjectPc34* obj = &s->objects[objIdx];
    int x = obj->x;
    int y = obj->y;
    int level = obj->level;

    if (x >= 0 && x < 32 && y >= 0 && y < 32 && level >= 0 && level < 16) {
        DM1_V1_FloorCellPc34* cell = &s->floors[level][y][x];
        for (int i = 0; i < cell->floorCount; i++) {
            if (cell->floorObjects[i].objectId == objIdx) {
                for (int j = i; j < cell->floorCount - 1; j++) {
                    cell->floorObjects[j] = cell->floorObjects[j + 1];
                }
                cell->floorCount--;
                break;
            }
        }
    }

    memset(obj, 0, sizeof(DM1_V1_WorldObjectPc34));
    return 0;
}

const char* DM1_V1_Object_TypeNamePc34Compat(int type) {
    switch (type) {
        case DM1_OBJTYPE_NONE: return "None";
        case DM1_OBJTYPE_WEAPON: return "Weapon";
        case DM1_OBJTYPE_ARMOR: return "Armor";
        case DM1_OBJTYPE_POTION: return "Potion";
        case DM1_OBJTYPE_SCROLL: return "Scroll";
        case DM1_OBJTYPE_KEY: return "Key";
        case DM1_OBJTYPE_FOOD: return "Food";
        case DM1_OBJTYPE_WATER: return "Water";
        case DM1_OBJTYPE_TORCH: return "Torch";
        case DM1_OBJTYPE_CHEST: return "Chest";
        case DM1_OBJTYPE_LEVER: return "Lever";
        case DM1_OBJTYPE_BUTTON: return "Button";
        case DM1_OBJTYPE_ALCOVE: return "Alcove";
        default: return "Unknown";
    }
}

int DM1_V1_Object_IsValidPc34Compat(const DM1_V1_ObjectStatePc34* s, int objIdx) {
    if (!s || objIdx < 0 || objIdx >= DM1_V1_MAX_WORLD_OBJECTS_PC34) return 0;
    return s->objects[objIdx].objectId == objIdx;
}
