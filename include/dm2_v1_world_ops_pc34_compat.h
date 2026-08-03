#ifndef FIRESTAFF_DM2_V1_WORLD_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_WORLD_OPS_PC34_COMPAT_H

/*
 * dm2_v1_world_ops_pc34_compat.h — DM2 V1 world/tile operations from
 * skproject/SKWIN/SkWinCore.cpp.
 *
 * Callback-based implementations of:
 *   SET_TILE_ATTRIBUTE_02             SkWinCore.cpp:3050
 *   __CHECK_ROOM_FOR_CONTAINER        SkWinCore.cpp:7682
 *   DM2_CREATURE_ROTATES_TARGET_CREATURE  c_creature.cpp:2394
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_OBJECT_NULL    0xFFFFu
#define DM2_V1_OBJECT_END     0xFFFEu
#define DM2_V1_CONTAINER_MAX  8

/* ---- SET_TILE_ATTRIBUTE_02 (SkWinCore.cpp:3050) ----
 * Set bit 0x02 on a tile's attribute byte. */
typedef struct {
    uint8_t *(*get_tile_attr)(void *ctx, int16_t x, int16_t y, int16_t map);
} DM2_V1_TileAttrCallbacks;

void dm2_v1_set_tile_attribute_02(
    int16_t x, int16_t y, int16_t map,
    const DM2_V1_TileAttrCallbacks *cb, void *ctx);

/* ---- __CHECK_ROOM_FOR_CONTAINER (SkWinCore.cpp:7682) ----
 * Extract up to 8 items from a container into an output array. */
typedef struct {
    uint16_t (*get_contained_object)(void *ctx, uint16_t container_rw);
    void (*cut_record_from)(void *ctx, uint16_t container_rw, uint16_t item_rw);
} DM2_V1_ContainerCallbacks;

int dm2_v1_check_room_for_container(
    uint16_t container_rw, uint16_t *current_container,
    uint16_t *items_out, int max_items,
    const DM2_V1_ContainerCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_ROTATES_TARGET_CREATURE (c_creature.cpp:2394) ----
 * Find creature at target position and rotate it to face the given direction.
 * Returns 0 on success, 1 if no creature at target. */
typedef struct {
    uint16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    void (*rotate_creature)(void *ctx, uint16_t creature_rw, int mode, int dir);
} DM2_V1_CreatureRotateTargetCallbacks;

int dm2_v1_creature_rotates_target_creature(
    int16_t target_x, int16_t target_y, uint8_t facing_dir,
    const DM2_V1_CreatureRotateTargetCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_WORLD_OPS_PC34_COMPAT_H */
