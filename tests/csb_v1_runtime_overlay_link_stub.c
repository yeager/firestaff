#include "csb_v1_runtime_pc34_compat.h"

uint16_t csb_v1_runtime_next_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing)
{
    (void)dungeon;
    (void)thing;
    return 0xFFFEu;
}

int csb_v1_runtime_object_overlay_info(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t object_thing,
    CSB_V1_RuntimeObjectOverlayInfo *out_info)
{
    (void)profile;
    (void)object_thing;
    if (out_info) {
        *out_info = (CSB_V1_RuntimeObjectOverlayInfo){0};
    }
    return 0;
}

int csb_v1_runtime_object_icon_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing)
{
    (void)profile;
    (void)thing;
    return -1;
}

int csb_v1_runtime_group_overlay_info(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    CSB_V1_RuntimeGroupOverlayInfo *out_info)
{
    (void)dungeon;
    (void)group_thing;
    if (out_info) {
        *out_info = (CSB_V1_RuntimeGroupOverlayInfo){0};
    }
    return 0;
}
