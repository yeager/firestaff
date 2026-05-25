#include "dm1_v1_object_interaction_pc34_compat.h"
#include "dm1_v1_inventory_consumables_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include <string.h>
#include <stdio.h>

void m11_obj_init(M11_ObjectState* s) {
    if (!s) return;
    memset(s, 0, sizeof(M11_ObjectState));
    s->objectCount = 0;
    for (int l = 0; l < 16; l++) {
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                s->floors[l][y][x].floorCount = 0;
            }
        }
    }
}

int m11_obj_spawn(M11_ObjectState* s, int type, int x, int y, int level, int weight) {
    if (!s) return -1;
    if (s->objectCount >= M11_MAX_WORLD_OBJECTS) return -1;
    if (x < 0 || x >= 32 || y < 0 || y >= 32 || level < 0 || level >= 16) return -1;

    int idx = s->objectCount;
    M11_WorldObject* obj = &s->objects[idx];
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

    M11_FloorCell* cell = &s->floors[level][y][x];
    if (cell->floorCount < M11_MAX_FLOOR_OBJECTS) {
        cell->floorObjects[cell->floorCount] = *obj;
        cell->floorCount++;
    }

    s->objectCount++;
    return idx;
}

int m11_obj_pickup(M11_ObjectState* s, int objIdx, int* outWeight) {
    /* Source: DUNGEON.C:F0161_DUNGEON_GetSquareFirstThing (floor lookup),
     * DUNGEON.C:F0156_DUNGEON_GetThingData (thing data pointer).
     * The x=-1 / x=-2 marking convention (carried/in-flight) is derived
     * from examining how ReDMCSB clears leader-hand things after use
     * (PANEL.C:1891 F0298_CHAMPION_GetObjectRemovedFromLeaderHand) and
     * how thrown objects are placed into flight state. */
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (obj->x == -1 || obj->x == -2) return -1; /* Already picked up or in flight */

    int x = obj->x;
    int y = obj->y;
    int level = obj->level;

    /* Remove from floor cell */
    M11_FloorCell* cell = &s->floors[level][y][x];
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
int m11_obj_drop(M11_ObjectState* s, int objIdx, int x, int y, int level) {
    /* Source: DUNGEON.C:F0140_DUNGEON_GetObjectWeight (weight lookup), 
     * DUNGEON.C:F0159_DUNGEON_GetNextThing (thing list traversal), 
     * DUNGEON.C:1111-1117 (container weight includes contents recursively). 
     * The floor cell API is a simplification of ReDMCSB THING linked-list 
     * system anchored in dungeon squares (DUNGEON square data). */
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    if (x < 0 || x >= 32 || y < 0 || y >= 32 || level < 0 || level >= 16) return -1;

    M11_WorldObject* obj = &s->objects[objIdx];
    obj->x = x;
    obj->y = y;
    obj->level = level;

    M11_FloorCell* cell = &s->floors[level][y][x];
    if (cell->floorCount < M11_MAX_FLOOR_OBJECTS) {
        cell->floorObjects[cell->floorCount] = *obj;
        cell->floorCount++;
    }

    return 0;
}

/* ReDMCSB OBJECT.C F0349 — use or consume an object.
 * Delegating to module-specific handlers per object type.
 * Source: OBJECT.C + INVENTORY.C + CONSUM.C
 *
 * m11_obj_use delegates to the consumables module for items that affect
 * champion stats (potions, food, junk/water). Equipment (weapons, armor,
 * accessories) is handled by the inventory slot system (m11_inventory_equip)
 * and does not go through this function — items in slots are used by
 * being equipped, not consumed. */
int m11_obj_use(M11_ObjectState* s, int champIdx, int objIdx,
                DM1ConsumableChampionPc34* champData,
                DM1ConsumableResultPc34* result)
{
    (void)s;
    if (!m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];

    switch (obj->objectType) {
        case DM1_OBJTYPE_POTION: {
            /* Potion: delegate to consumables module (CONSUM.C F0340).
             * champData carries current stat snapshot; result returns
             * delta to apply. Caller is responsible for committing deltas
             * to the actual champion state (health, stamina, mana, attrs).
             * Source: PANEL.C:1850-1917 F0349 potion application + VI wound cure. */
            if (!champData || !result) return 0;
            return dm1_inventory_consume_potion_pc34(champData,
                                                    0 /* potionType */,
                                                    obj->weight /* power proxy */,
                                                    NULL /* woundMasks */,
                                                    0 /* woundMaskCount */,
                                                    result);
        }
        case DM1_OBJTYPE_FOOD: {
            /* Food: delegate to food consumption (CONSUM.C F0343).
             * obj->weight carries food amount proxy for food-type items.
             * Source: PANEL.C:1918-1919 G0242 food amounts. */
            if (!champData || !result) return 0;
            return dm1_inventory_consume_food_junk_pc34(champData,
                                                        0 /* iconIndex proxy */,
                                                        result);
        }
        case DM1_OBJTYPE_WATER: {
            /* Water/junk: delegate to water consumption (CONSUM.C F0342).
             * obj->stackCount carries charge count for waterskins.
             * Source: PANEL.C:1824-1844 waterskin charge use. */
            if (!champData || !result) return 0;
            return dm1_inventory_consume_water_junk_pc34(champData,
                                                         0 /* iconIndex */,
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
            /* Equipment: handled by m11_inventory_equip() slot system.
             * Not consumed; mouth-click returns 0 (non-usable here).
             * Source: INVENTORY.C F0300-F0302 slot equip. */
            return 0;
        }
        default:
            return 0;
    }
}

int m11_obj_throw(M11_ObjectState* s, int objIdx, int dir, int force) {
    /* Source: CLIKVIEW.C / DUNGEON.C -- thrown objects are marked as in-flight 
     * (x=-2) until they land or are consumed. ReDMCSB thrown object handling 
     * is in the click/viewport routing and THING projectile system. */
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (!obj->throwable) return 0;

    obj->x = -2;
    return 1;
}

int m11_obj_activate(M11_ObjectState* s, int objIdx) {
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (obj->activatable) {
        return 1;
    }
    return 0;
}

int m11_obj_examine(const M11_ObjectState* s, int objIdx, char* desc, int descLen) {
    /* Source: OBJECT.C:F0033_OBJECT_GetIconIndex (icon), 
     * OBJECT.C:237 (G0352_apc_ObjectNames[name]), 
     * PANEL.C:1444-1469 (weight display format WEIGHS X.Y KG. 
     * via F0140/10 and F0140%10, CHAMDRAW.C:349-392). 
     * Note: obj->weight is the stored spawn weight; ReDMCSB recomputes 
     * weight from type-specific tables (G0238/G0239/G0241) per F0140. 
     * This means the displayed weight may diverge from ReDMCSB for items 
     * whose weight varies by type detail (e.g. waterskin by charges). */
    if (!s || !m11_obj_is_valid(s, objIdx) || !desc || descLen <= 0) return -1;
    const M11_WorldObject* obj = &s->objects[objIdx];
    const char* typeName = m11_obj_type_name(obj->objectType); 
    snprintf(desc, descLen, "%s (Weight: %d)", typeName, obj->weight);
    return 0;
}

int m11_obj_get_at(const M11_ObjectState* s, int x, int y, int level, int* outIndices, int maxOut) {
    if (!s || !outIndices) return 0;
    if (x < 0 || x >= 32 || y < 0 || y >= 32 || level < 0 || level >= 16) return 0;

    const M11_FloorCell* cell = &s->floors[level][y][x];
    int count = 0;
    for (int i = 0; i < cell->floorCount && count < maxOut; i++) {
        outIndices[count++] = cell->floorObjects[i].objectId;
    }
    return count;
}

int m11_obj_remove(M11_ObjectState* s, int objIdx) {
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;

    M11_WorldObject* obj = &s->objects[objIdx];
    int x = obj->x;
    int y = obj->y;
    int level = obj->level;

    if (x >= 0 && x < 32 && y >= 0 && y < 32 && level >= 0 && level < 16) {
        M11_FloorCell* cell = &s->floors[level][y][x];
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

    memset(obj, 0, sizeof(M11_WorldObject));
    return 0;
}

const char* m11_obj_type_name(int type) {
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

int m11_obj_is_valid(const M11_ObjectState* s, int objIdx) {
    if (!s || objIdx < 0 || objIdx >= M11_MAX_WORLD_OBJECTS) return 0;
    return s->objects[objIdx].objectId == objIdx;
}
