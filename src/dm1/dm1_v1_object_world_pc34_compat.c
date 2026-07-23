/* DM1 V1 Object/World Management — source-locked from ReDMCSB OBJECT.C/DUNGEON.C
 * G0237 ObjectInfo[180], G0238 WeaponInfo[46], G0239 ArmourInfo[58],
 * G0243 CreatureInfo[27], G0254 DoorInfo[4], F0031_LoadNames pattern */

#include "dm1_v1_object_world_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

void DM1_V1_ObjectWorld_InitPc34Compat(DM1_V1_ObjectWorldStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_ObjectWorldStatePc34));
    state->loaded = false;
}

/* F0031_OBJECT_LoadNames: reads null-terminated strings sequentially from
 * graphic #559 data buffer. Up to 199 object names. */
bool DM1_V1_ObjectWorld_LoadObjectNamesPc34Compat(DM1_V1_ObjectWorldStatePc34* state, const uint8_t* data, size_t size) {
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

const char* DM1_V1_ObjectWorld_GetObjectNamePc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx) {
    if (!state || idx >= DM1_OBJ_NAME_COUNT) return NULL;
    return state->obj_names[idx];
}

const DM1_V1_ObjectWorldObjectInfoPc34* DM1_V1_ObjectWorld_GetObjectInfoPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx) {
    if (!state || idx >= DM1_OBJ_INFO_COUNT) return NULL;
    return &state->obj_info[idx];
}

const DM1_V1_ObjectWorldWeaponInfoPc34* DM1_V1_ObjectWorld_GetWeaponPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx) {
    if (!state || idx >= DM1_WEAPON_INFO_COUNT) return NULL;
    return &state->weapons[idx];
}

const DM1_V1_ObjectWorldArmourInfoPc34* DM1_V1_ObjectWorld_GetArmourPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx) {
    if (!state || idx >= DM1_ARMOUR_INFO_COUNT) return NULL;
    return &state->armour[idx];
}

const DM1_V1_ObjectWorldCreatureInfoPc34* DM1_V1_ObjectWorld_GetCreaturePc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx) {
    if (!state || idx >= DM1_CREATURE_INFO_COUNT) return NULL;
    return &state->creatures[idx];
}

const DM1_V1_ObjectWorldDoorInfoPc34* DM1_V1_ObjectWorld_GetDoorPc34Compat(const DM1_V1_ObjectWorldStatePc34* state, uint16_t idx) {
    if (!state || idx >= DM1_DOOR_INFO_COUNT) return NULL;
    return &state->doors[idx];
}

void DM1_V1_ObjectWorld_CleanupPc34Compat(DM1_V1_ObjectWorldStatePc34* state) {
    if (!state) return;
    for (int i = 0; i < DM1_OBJ_NAME_COUNT; i++) {
        free(state->obj_names[i]);
        state->obj_names[i] = NULL;
    }
    state->loaded = false;
}

int DM1_V1_ObjectWorld_AdmitPc34GraphicsSurfacePc34Compat(
    const DM1_V1_ObjectWorldGraphicsSurfacePc34 *surfaces,
    int surfaceCount,
    int expectedGraphicIndex)
{
    int i;
    if (!surfaces || surfaceCount <= 0 || expectedGraphicIndex <= 0) return 0;
    for (i = 0; i < surfaceCount; ++i) {
        const DM1_V1_ObjectWorldGraphicsSurfacePc34 *surface = &surfaces[i];
        size_t required;
        if (surface->graphicIndex != expectedGraphicIndex ||
            !surface->verifiedPc34GraphicsDat || !surface->pixels ||
            surface->width <= 0 || surface->height <= 0) continue;
        required = (size_t)surface->width * (size_t)surface->height;
        if (surface->pixelCount >= required) return 1;
    }
    return 0;
}
