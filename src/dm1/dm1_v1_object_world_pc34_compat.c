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

/* ReDMCSB OBJECT.C F0031 loads the raw M564 object-name record (556 in
 * legacy media, 694 in PC3.4 multilingual media). MEDIA060 terminates names
 * with the high bit; MEDIA574 uses NUL terminators. This helper receives the
 * already-expanded raw record, not GRAPHICS.DAT bitmap 559. */
bool DM1_V1_ObjectWorld_LoadObjectNamesPc34Compat(DM1_V1_ObjectWorldStatePc34* state, const uint8_t* data, size_t size) {
    char* parsed[DM1_OBJ_NAME_COUNT] = { 0 };
    size_t offset = 0u;
    size_t nul_count = 0u;
    size_t index;
    int high_bit_terminated;

    if (!state || !data || size == 0u) return false;
    for (index = 0u; index < size; ++index) {
        if (data[index] == 0u) ++nul_count;
    }
    /* Japanese MEDIA574 can contain high-bit CP932 bytes. MEDIA689 also
     * carries NUL separators; F0031 changes the preceding byte-1 marker to
     * NUL in place for its string consumers. */
    high_bit_terminated = nul_count < DM1_OBJ_NAME_COUNT;

    for (index = 0u; index < DM1_OBJ_NAME_COUNT; ++index) {
        size_t start = offset;
        size_t length;
        size_t written;
        int terminated = 0;

        while (offset < size) {
            uint8_t ch = data[offset++];
            if ((!high_bit_terminated && ch == 0u) ||
                (high_bit_terminated && (ch & 0x80u) != 0u)) {
                terminated = 1;
                break;
            }
        }
        if (!terminated) goto fail;
        length = offset - start - (high_bit_terminated ? 0u : 1u);
        if (!high_bit_terminated && length > 0u && data[start + length - 1u] == 1u) {
            --length; /* MEDIA689 F0031 overwrites its byte-1 end marker with NUL. */
        }
        parsed[index] = (char*)malloc(length + 1u);
        if (!parsed[index]) goto fail;
        written = 0u;
        while (written < length) {
            uint8_t ch = data[start + written];
            if (high_bit_terminated && written + 1u == length) ch &= 0x7fu;
            parsed[index][written] = (char)ch;
            ++written;
        }
        parsed[index][written] = '\0';
    }

    for (index = 0u; index < DM1_OBJ_NAME_COUNT; ++index) {
        free(state->obj_names[index]);
        state->obj_names[index] = parsed[index];
    }
    state->loaded = true;
    return true;

fail:
    for (index = 0u; index < DM1_OBJ_NAME_COUNT; ++index) free(parsed[index]);
    return false;
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
