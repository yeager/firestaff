#ifndef CSB_V1_ATARI_F0115_PROJECTILE_PLAN_H
#define CSB_V1_ATARI_F0115_PROJECTILE_PLAN_H

#include <stddef.h>
#include <stdint.h>

enum {
    CSB_V1_ATARI_F0115_PROJECTILE_C558_SIZE = 4720,
    CSB_V1_ATARI_F0115_PROJECTILE_G0215_OFFSET = 1494,
    CSB_V1_ATARI_F0115_PROJECTILE_G0210_OFFSET = 1558,
    CSB_V1_ATARI_F0115_PROJECTILE_G0218_OFFSET = 1156,
    CSB_V1_ATARI_F0115_PROJECTILE_ASPECT_COUNT = 14,
    CSB_V1_ATARI_F0115_PROJECTILE_GFX_BASE = 316,
    CSB_V1_ATARI_F0115_PROJECTILE_DERIVED_BASE = 282,
    CSB_V1_ATARI_F0115_PROJECTILE_NO_PALETTE = 0xff
};

typedef enum CSB_V1_AtariF0115ProjectileResult {
    CSB_V1_ATARI_F0115_PROJECTILE_INVALID = -1,
    CSB_V1_ATARI_F0115_PROJECTILE_NOT_VISIBLE = 0,
    CSB_V1_ATARI_F0115_PROJECTILE_DRAW_PROJECTILE = 1,
    CSB_V1_ATARI_F0115_PROJECTILE_DRAW_OBJECT = 2
} CSB_V1_AtariF0115ProjectileResult;

typedef struct CSB_V1_AtariF0115ProjectileInput {
    const uint8_t *c558;
    size_t c558_size;
    /* Exact signed F0142 selector from the source projectile Slot:
     * negative means 1-based PROJECTIL_ASPECT ordinal; nonnegative means
     * G0209 OBJECT_ASPECT index. Never infer this from attack damage. */
    int f0142_selector;
    int view_square;          /* MEDIA020 M600..M609, D3C=0 through D0C=9 */
    int view_cell;            /* C00..C03 relative cell order */
    int projectile_direction; /* source C14 direction, 0..3 */
    int party_direction;      /* source camera direction, 0..3 */
    int map_x;
    int map_y;
    int kinetic_energy;       /* source C14 value, 0..255 */
} CSB_V1_AtariF0115ProjectileInput;

typedef struct CSB_V1_AtariF0115ProjectilePlan {
    int f0142_selector;
    int uses_object_aspect;
    int aspect_index;
    int object_aspect_index;
    int source_view_square;
    int source_view_cell;
    int source_coordinate_x;
    int source_coordinate_y;
    int destination_center_y; /* ReDMCSB MEDIA009 fixes this at 47 */
    int native_graphic_index;
    int derived_bitmap_index;  /* -1 when source uses native/temp bitmap */
    int bitmap_delta;
    int aspect_type;
    int graphic_info;
    int source_scale_index;
    int source_scale_32;
    int source_byte_width;
    int source_height;
    int draw_byte_width;
    int draw_width_pixels;
    int draw_height;
    int scale_with_kinetic_energy;
    int flip_horizontal;
    int flip_vertical;
    int palette_change_index;  /* G0075 index; 0xff means no palette change */
} CSB_V1_AtariF0115ProjectilePlan;

/* ReDMCSB DUNGEON.C F0142: resolves the source PROJECTILE.Slot handle.
 * `weapon_aspect_ordinal` is the result of M066 from authentic G0238; zero
 * means the weapon has no projectile aspect. `object_aspect_index` is the
 * source G0237/F0141 result. Returns 1 with the signed F0142 selector, or 0
 * when required authentic source state is unavailable. */
int csb_v1_atari_f0115_resolve_f0142_selector(
    uint16_t source_slot_thing,
    int weapon_aspect_ordinal,
    int object_aspect_index,
    int *out_selector);

/* Returns DRAW_PROJECTILE/DRAW_OBJECT with a source-owned plan, NOT_VISIBLE
 * for F0115's zero-X G0218 cells, and INVALID for unsupported/malformed C558
 * or missing source state. C558 must be the authenticated G20E MEDIA020 item
 * 558. This function does not draw or substitute any bitmap. */
int csb_v1_atari_f0115_plan_projectile(
    const CSB_V1_AtariF0115ProjectileInput *input,
    CSB_V1_AtariF0115ProjectilePlan *out_plan);

#endif
