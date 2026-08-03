/* DM2 V1 world/tile operations — skproject SkWinCore.cpp + c_creature.cpp. */

#include "dm2_v1_world_ops_pc34_compat.h"
#include <stddef.h>

void dm2_v1_set_tile_attribute_02(
    int16_t x, int16_t y, int16_t map,
    const DM2_V1_TileAttrCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *attr = cb->get_tile_attr(ctx, x, y, map);
    if (attr)
        *attr |= 0x02;
}

int dm2_v1_check_room_for_container(
    uint16_t container_rw, uint16_t *current_container,
    uint16_t *items_out, int max_items,
    const DM2_V1_ContainerCallbacks *cb, void *ctx)
{
    if (!cb || !items_out || !current_container)
        return 0;
    if (container_rw == DM2_V1_OBJECT_NULL)
        return 0;
    if (container_rw == *current_container)
        return 0;
    int count = 0;
    while (count < max_items) {
        uint16_t item = cb->get_contained_object(ctx, container_rw);
        if (item == (uint16_t)DM2_V1_OBJECT_END || item == DM2_V1_OBJECT_NULL)
            break;
        items_out[count++] = item;
        cb->cut_record_from(ctx, container_rw, item);
    }
    for (int i = count; i < max_items; i++)
        items_out[i] = DM2_V1_OBJECT_NULL;
    *current_container = container_rw;
    return count;
}

int dm2_v1_creature_rotates_target_creature(
    int16_t target_x, int16_t target_y, uint8_t facing_dir,
    const DM2_V1_CreatureRotateTargetCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;
    uint16_t creature = cb->get_creature_at(ctx, target_x, target_y);
    if (creature == DM2_V1_OBJECT_NULL)
        return 1;
    cb->rotate_creature(ctx, creature, 1, facing_dir);
    return 0;
}
