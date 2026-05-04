#include "dm1_v1_object_interaction_pc34_compat.h"
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
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (obj->x == -1 || obj->x == -2) return -1; // Already picked up or in flight

    int x = obj->x;
    int y = obj->y;
    int level = obj->level;

    // Remove from floor cell
    M11_FloorCell* cell = &s->floors[level][y][x];
    for (int i = 0; i < cell->floorCount; i++) {
        if (cell->floorObjects[i].objectId == objIdx) {
            // Shift down
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

int m11_obj_use(M11_ObjectState* s, int objIdx) {
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (obj->usable) {
        // Effect TBD
        return 1;
    }
    return 0;
}

int m11_obj_throw(M11_ObjectState* s, int objIdx, int dir __attribute__((unused)), int force __attribute__((unused))) {
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (!obj->throwable) return 0;

    // Mark as in-flight
    obj->x = -2;
    // Direction and force are not stored in struct, but action is recorded
    return 1;
}

int m11_obj_activate(M11_ObjectState* s, int objIdx) {
    if (!s || !m11_obj_is_valid(s, objIdx)) return -1;
    M11_WorldObject* obj = &s->objects[objIdx];
    if (obj->activatable) {
        // Toggle state logic would go here
        return 1;
    }
    return 0;
}

int m11_obj_examine(const M11_ObjectState* s, int objIdx, char* desc, int descLen) {
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

    // Remove from floor cell if present
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

    // Zero out object slot
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
        case DM1_OBJTYPE_MISC: return "Misc";
        default: return "Unknown";
    }
}

int m11_obj_is_valid(const M11_ObjectState* s, int objIdx) {
    if (!s) return 0;
    if (objIdx < 0 || objIdx >= M11_MAX_WORLD_OBJECTS) return 0;
    if (s->objects[objIdx].objectType == DM1_OBJTYPE_NONE) return 0;
    return 1;
}