/* DM1 V1 Object/World Management — source-locked from ReDMCSB OBJECT.C/DUNGEON.C
 * G0237 ObjectInfo[180], G0238 WeaponInfo[46], G0239 ArmourInfo[58],
 * G0243 CreatureInfo[27], G0254 DoorInfo[4], F0031_LoadNames pattern */

#include "dm1_v1_object_world_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

void m11_ow_init(M11_OW_WorldState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_OW_WorldState));
    state->loaded = false;
}

/* F0031_OBJECT_LoadNames: reads null-terminated strings sequentially from
 * graphic #559 data buffer. Up to 199 object names. */
bool m11_ow_load_object_names(M11_OW_WorldState* state, const uint8_t* data, size_t size) {
    if (!state || !data || size == 0) return false;

    size_t offset = 0;
    int name_idx = 0;

    while (offset < size && name_idx < DM1_OBJ_NAME_COUNT) {
        /* Find end of current null-terminated string */
        size_t start = offset;
        while (offset < size && data[offset] != '\0') {
            offset++;
        }
        if (offset >= size) break;

        size_t len = offset - start;
        char* name = (char*)malloc(len + 1);
        if (!name) return false;
        memcpy(name, data + start, len);
        name[len] = '\0';

        /* Free any previously allocated name */
        free(state->obj_names[name_idx]);
        state->obj_names[name_idx] = name;
        name_idx++;
        offset++; /* skip null terminator */
    }

    state->loaded = true;
    return true;
}

const char* m11_ow_get_obj_name(const M11_OW_WorldState* state, uint16_t idx) {
    if (!state || idx >= DM1_OBJ_NAME_COUNT) return NULL;
    return state->obj_names[idx];
}

const M11_OW_ObjectInfo* m11_ow_get_obj_info(const M11_OW_WorldState* state, uint16_t idx) {
    if (!state || idx >= DM1_OBJ_INFO_COUNT) return NULL;
    return &state->obj_info[idx];
}

const M11_OW_WeaponInfo* m11_ow_get_weapon(const M11_OW_WorldState* state, uint16_t idx) {
    if (!state || idx >= DM1_WEAPON_INFO_COUNT) return NULL;
    return &state->weapons[idx];
}

const M11_OW_ArmourInfo* m11_ow_get_armour(const M11_OW_WorldState* state, uint16_t idx) {
    if (!state || idx >= DM1_ARMOUR_INFO_COUNT) return NULL;
    return &state->armour[idx];
}

const M11_OW_CreatureInfo* m11_ow_get_creature(const M11_OW_WorldState* state, uint16_t idx) {
    if (!state || idx >= DM1_CREATURE_INFO_COUNT) return NULL;
    return &state->creatures[idx];
}

const M11_OW_DoorInfo* m11_ow_get_door(const M11_OW_WorldState* state, uint16_t idx) {
    if (!state || idx >= DM1_DOOR_INFO_COUNT) return NULL;
    return &state->doors[idx];
}

void m11_ow_cleanup(M11_OW_WorldState* state) {
    if (!state) return;
    for (int i = 0; i < DM1_OBJ_NAME_COUNT; i++) {
        free(state->obj_names[i]);
        state->obj_names[i] = NULL;
    }
    state->loaded = false;
}
