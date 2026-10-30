#ifndef CSB_V1_ATARI_F0115_EXPLOSION_PLAN_H
#define CSB_V1_ATARI_F0115_EXPLOSION_PLAN_H

#include <stdint.h>

typedef enum CSB_V1_AtariF0115ExplosionKind {
    CSB_V1_ATARI_F0115_EXPLOSION_INVALID = -1,
    CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN = 0,
    CSB_V1_ATARI_F0115_EXPLOSION_SPRITE = 1,
    CSB_V1_ATARI_F0115_EXPLOSION_D0C_PATTERN = 2,
    CSB_V1_ATARI_F0115_EXPLOSION_FLUXCAGE = 3
} CSB_V1_AtariF0115ExplosionKind;

typedef struct CSB_V1_AtariF0115ExplosionInput {
    int explosion_type;
    int attack;
    int centered;
    int cell;
    int last_view_cell; /* Final F0115 view-cell ordinal, 0..3, or -1. */
    int party_direction;
    int view_square; /* MEDIA020 M600..M611: D3C through D0R */
    int flip_horizontal; /* M005_RANDOM(2), source-owned G0349 draw */
    int flip_vertical;   /* M005_RANDOM(2), source-owned G0349 draw */
} CSB_V1_AtariF0115ExplosionInput;

typedef struct CSB_V1_AtariF0115ExplosionPlan {
    int kind;
    int view_square;
    int explosion_coordinate_index; /* G0225/G0226 index, view_square + 3 */
    int aspect_index;
    int graphic_index;
    int source_byte_width;
    int source_height;
    int scale_32;
    int draw_byte_width;
    int draw_height;
    int center_x;
    int center_y;
    int flip_horizontal;
    int flip_vertical;
    int transparent_color;
    int smoke_palette;
    int d0c_pattern_size;
} CSB_V1_AtariF0115ExplosionPlan;

/* Source tables are MEDIA020 constants in ReDMCSB DUNVIEW.C, not C558 data.
 * Returns INVALID for unsupported/malformed input, otherwise one of the
 * source draw routes above. This plans source data only; the selected
 * authentic GRAPHICS.DAT still owns the pixels. */
int csb_v1_atari_f0115_plan_explosion(
    const CSB_V1_AtariF0115ExplosionInput *input,
    CSB_V1_AtariF0115ExplosionPlan *out_plan);

#endif
