#ifndef CSB_V1_ATARI_F0113_FLUXCAGE_PLAN_H
#define CSB_V1_ATARI_F0113_FLUXCAGE_PLAN_H

#include <stdint.h>

enum {
    CSB_V1_ATARI_F0113_VIEW_SQUARE_COUNT = 12,
    CSB_V1_ATARI_F0113_FIELD_MASK_BASE = 69,
    CSB_V1_ATARI_F0113_TELEPORTER_GRAPHIC = 73,
    CSB_V1_ATARI_F0113_FLUXCAGE_GRAPHIC = 74,
    CSB_V1_ATARI_F0113_C050 = 50
};

typedef enum CSB_V1_AtariF0113FluxcageResult {
    CSB_V1_ATARI_F0113_FLUXCAGE_INVALID = -1,
    CSB_V1_ATARI_F0113_FLUXCAGE_NOT_DRAWN = 0,
    CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD = 1
} CSB_V1_AtariF0113FluxcageResult;

typedef struct CSB_V1_AtariF0113FluxcageInput {
    int explosion_type;
    int view_square; /* MEDIA020 M600..M611: D3C through D0R. */
    int door_front_view_pass; /* F0115 draws the field only after pass 1. */
    int suppress_during_endgame;
} CSB_V1_AtariF0113FluxcageInput;

typedef struct CSB_V1_AtariF0113FluxcagePlan {
    int kind;
    int view_square;
    int graphic_index;
    int mask_graphic_index; /* -1 when the field aspect uses no mask. */
    int mask_index; /* -1 when the field aspect uses no mask. */
    int mask_flip_horizontal;
    int transparent_color;
    int source_byte_width;
    int source_height;
    int source_pixel_width;
    int base_last_unit;
    int last_unit_random_range;
    int first_unit_random_range;
    int source_x;
    /* G0163 frame box: x1, x2, y1, y2, byte width, height, x, y. */
    uint8_t destination_frame[8];
    /* G0188 field aspect after F0115 increments byte 0 for C050. */
    uint8_t field_aspect[8];
} CSB_V1_AtariF0113FluxcagePlan;

/* ReDMCSB DUNVIEW.C MEDIA020 F0115/F0113. The caller supplies the live
 * source C15 type and route/pass gates; this planner owns the exact
 * source-compiled G0188/G0163 rows, asset indexes, and mask metadata. */
int csb_v1_atari_f0113_plan_fluxcage(
    const CSB_V1_AtariF0113FluxcageInput *input,
    CSB_V1_AtariF0113FluxcagePlan *out_plan);

#endif
