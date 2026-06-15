#include "dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_pc34_compat.h"

#include <string.h>

enum {
    DM1_D2L2_GUARD_VIEW_SQUARE = 9,
    DM1_D2R2_GUARD_VIEW_SQUARE = 10,
    DM1_D2L_VIEW_SQUARE = 7,
    DM1_D2R_VIEW_SQUARE = 8,
    DM1_D2L_FLOOR_VIEW = 5,
    DM1_D2R_FLOOR_VIEW = 7,
    DM1_FLOOR_ASPECT_SLOT = 558,
    DM1_FLOOR_ZONE_BASE = 1500,
    DM1_PC34_FLOOR_ZONE_STRIDE = 11,
    DM1_D2L_DOOR_ZONE = 3750,
    DM1_D2R_DOOR_ZONE = 3770,
    DM1_D2L_DOOR_FRAME_TOP_ZONE = 729,
    DM1_D2R_DOOR_FRAME_TOP_ZONE = 731
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0108:3940-4011 draws a nonzero floor ornament ordinal,
 *   clears MASK0x8000_FOOTPRINTS, blits with C10_COLOR_FLESH, and recurses
 *   to C15_FLOOR_ORNAMENT_FOOTPRINTS for footprint-marked ordinals.
 * - DUNVIEW.C F0119:6987-7004 draws the D2L door-front floor ornament
 *   before F0115 pass1, door-frame top, F0111, and F0115 pass2.
 * - DUNVIEW.C F0120:7180-7197 mirrors that D2R door-front order.
 * - DUNVIEW.C F0119:7017-7038 and F0120:7210-7232 draw open/pit/side-door
 *   floor ornaments before ceiling pits and the open-row F0115 pass.
 * - DUNVIEW.C F0119:6961-6974 and F0120:7112-7147 prove the wall element
 *   itself only uses F0107 wall ornaments and returns without F0108.
 * - DUNVIEW.C F0678/F0679:6837-6896 prove C09/C10 D2L2/D2R2 guard squares
 *   do not own F0108; the F0108 composition belongs to M604/M605 D2L/D2R.
 * - DEFS.H:2088,2603-2606,2668-2677,2742-2757,4047-4051,4239-4257
 *   anchor C10, view-square ids, cell orders, floor views, wall zones, and
 *   D2 door zones.
 */

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0108:3940-4011 floor ornament "
    "ordinal, MASK0x8000_FOOTPRINTS clear, C10 blit, and footprint "
    "recursion; DUNVIEW.C F0119:6987-7004 D2L door-front order; "
    "DUNVIEW.C F0120:7180-7197 D2R door-front order; DUNVIEW.C "
    "F0119:7017-7038 and F0120:7210-7232 open/pit/side-door order; "
    "DUNVIEW.C F0119:6961-6974 and F0120:7112-7147 wall cases exclude "
    "F0108; DUNVIEW.C F0678/F0679:6837-6896 C09/C10 D2L2/D2R2 guard "
    "squares exclude F0108; DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:"
    "2603-2606 M604/M605 plus C09/C10; DEFS.H:2668-2677 cell orders; "
    "DEFS.H:2742-2757 M591/M593 floor views; DEFS.H:4047-4051 wall "
    "zones; DEFS.H:4239-4257 M627/M629 door zones.";

static const DM1_V1_D2L2D2R2F0108WallSpecPc34 s_specs[] = {
    {
        DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2L_PC34,
        DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34,
        "D2L open/pit/side-door F0108 composition guarded by D2L2",
        0,
        2,
        -1,
        DM1_D2L2_GUARD_VIEW_SQUARE,
        DM1_D2L_VIEW_SQUARE,
        DM1_D2L_FLOOR_VIEW,
        DM1_FLOOR_ASPECT_SLOT,
        DM1_FLOOR_ZONE_BASE,
        DM1_PC34_FLOOR_ZONE_STRIDE,
        false,
        true,
        true,
        true,
        true,
        false,
        0,
        0,
        0x3421u,
        0x0218u,
        0x0349u,
        DM1_V1_D2L2_D2R2_F0108_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0119:7017-7038 T0119019/T0119020 open-row route",
        "DEFS.H:2603-2606/2668-2677/2742-2757/4047-4051"
    },
    {
        DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2R_PC34,
        DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_OPEN_PC34,
        "D2R open/pit/side-door F0108 composition guarded by D2R2",
        1,
        2,
        1,
        DM1_D2R2_GUARD_VIEW_SQUARE,
        DM1_D2R_VIEW_SQUARE,
        DM1_D2R_FLOOR_VIEW,
        DM1_FLOOR_ASPECT_SLOT,
        DM1_FLOOR_ZONE_BASE,
        DM1_PC34_FLOOR_ZONE_STRIDE,
        true,
        true,
        true,
        true,
        true,
        false,
        0,
        0,
        0x4312u,
        0x0128u,
        0x0439u,
        DM1_V1_D2L2_D2R2_F0108_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0120:7210-7232 T0120028/T0120029 open-row route",
        "DEFS.H:2603-2606/2668-2677/2742-2757/4047-4051"
    },
    {
        DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2L_PC34,
        DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34,
        "D2L door-front F0108 before F0111 guarded by D2L2",
        2,
        2,
        -1,
        DM1_D2L2_GUARD_VIEW_SQUARE,
        DM1_D2L_VIEW_SQUARE,
        DM1_D2L_FLOOR_VIEW,
        DM1_FLOOR_ASPECT_SLOT,
        DM1_FLOOR_ZONE_BASE,
        DM1_PC34_FLOOR_ZONE_STRIDE,
        false,
        true,
        true,
        true,
        false,
        true,
        DM1_D2L_DOOR_ZONE,
        DM1_D2L_DOOR_FRAME_TOP_ZONE,
        0x3421u,
        0x0218u,
        0x0349u,
        DM1_V1_D2L2_D2R2_F0108_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0119:6987-7004 door-front F0108/F0111 route",
        "DEFS.H:2603-2606/2668-2677/2742-2757/4239-4257"
    },
    {
        DM1_V1_D2L2_D2R2_F0108_WALL_SIDE_D2R_PC34,
        DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34,
        "D2R door-front F0108 before F0111 guarded by D2R2",
        3,
        2,
        1,
        DM1_D2R2_GUARD_VIEW_SQUARE,
        DM1_D2R_VIEW_SQUARE,
        DM1_D2R_FLOOR_VIEW,
        DM1_FLOOR_ASPECT_SLOT,
        DM1_FLOOR_ZONE_BASE,
        DM1_PC34_FLOOR_ZONE_STRIDE,
        true,
        true,
        true,
        true,
        false,
        true,
        DM1_D2R_DOOR_ZONE,
        DM1_D2R_DOOR_FRAME_TOP_ZONE,
        0x4312u,
        0x0128u,
        0x0439u,
        DM1_V1_D2L2_D2R2_F0108_WALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C F0108:3940-4011",
        "DUNVIEW.C F0120:7180-7197 door-front F0108/F0111 route",
        "DEFS.H:2603-2606/2668-2677/2742-2757/4239-4257"
    }
};

size_t dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D2L2D2R2F0108WallSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D2L2D2R2F0108WallSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_for_pc34(
    DM1_V1_D2L2D2R2F0108WallSidePc34 side,
    DM1_V1_D2L2D2R2F0108WallContextPc34 context)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_count_pc34(); ++i) {
        if (s_specs[i].side == side && s_specs[i].context == context) {
            return &s_specs[i];
        }
    }
    return NULL;
}

bool dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_decode_ordinal_pc34(
    unsigned int floor_ornament_ordinal,
    DM1_V1_D2L2D2R2F0108WallOrdinalPc34 *out)
{
    unsigned int cleared;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->input_ordinal = floor_ornament_ordinal;
    out->has_input_ordinal = floor_ornament_ordinal != 0u;
    if (!out->has_input_ordinal) return true;

    out->footprint_flag_set =
        (floor_ornament_ordinal & DM1_V1_D2L2_D2R2_F0108_WALL_FOOTPRINT_MASK_PC34) != 0u;
    cleared = floor_ornament_ordinal &
        ~DM1_V1_D2L2_D2R2_F0108_WALL_FOOTPRINT_MASK_PC34;
    out->cleared_ordinal = cleared;
    out->primary_draws = !out->footprint_flag_set || cleared != 0u;
    if (out->primary_draws) {
        out->primary_ordinal = out->footprint_flag_set ? cleared : floor_ornament_ordinal;
        out->primary_index = (int)out->primary_ordinal - 1;
    } else {
        out->primary_index = -1;
    }
    out->recursive_footprints_draw = out->footprint_flag_set;
    if (out->recursive_footprints_draw) {
        out->recursive_footprints_index =
            DM1_V1_D2L2_D2R2_F0108_WALL_FOOTPRINT_INDEX_PC34;
        out->recursive_footprints_ordinal =
            (unsigned int)DM1_V1_D2L2_D2R2_F0108_WALL_FOOTPRINT_INDEX_PC34 + 1u;
    }
    return true;
}

uint8_t dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_compose_pixel_pc34(
    const DM1_V1_D2L2D2R2F0108WallSpecPc34 *spec,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t ceiling_or_frame_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D2L2D2R2F0108WallPixelPc34 *out)
{
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->destination_before = destination_before;
    out->floor_pixel = floor_pixel;
    out->ceiling_or_frame_pixel = ceiling_or_frame_pixel;
    out->pass1_pixel = pass1_pixel;
    out->door_pixel = door_pixel;
    out->pass2_pixel = pass2_pixel;
    if (!spec) return false;

    out->door_front_sequence =
        spec->context == DM1_V1_D2L2_D2R2_F0108_WALL_CONTEXT_DOOR_FRONT_PC34;
    pixel = destination_before;

    out->floor_transparent = floor_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
        pixel, floor_pixel, (uint8_t)spec->transparent_color);
    out->after_floor = pixel;

    if (out->door_front_sequence) {
        out->pass1_transparent = pass1_pixel == spec->transparent_color;
        pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
            pixel, pass1_pixel, (uint8_t)spec->transparent_color);
        out->after_pass1 = pixel;

        out->ceiling_or_frame_transparent =
            ceiling_or_frame_pixel == spec->transparent_color;
        pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
            pixel, ceiling_or_frame_pixel, (uint8_t)spec->transparent_color);
        out->after_ceiling_or_frame = pixel;

        out->door_transparent = door_pixel == spec->transparent_color;
        pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
            pixel, door_pixel, (uint8_t)spec->transparent_color);
        out->after_door = pixel;

        out->pass2_transparent = pass2_pixel == spec->transparent_color;
        pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
            pixel, pass2_pixel, (uint8_t)spec->transparent_color);
        out->after_pass2 = pixel;
    } else {
        out->ceiling_or_frame_transparent =
            ceiling_or_frame_pixel == spec->transparent_color;
        pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
            pixel, ceiling_or_frame_pixel, (uint8_t)spec->transparent_color);
        out->after_ceiling_or_frame = pixel;

        out->pass1_transparent = pass1_pixel == spec->transparent_color;
        pixel = dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_blend_pixel_pc34(
            pixel, pass1_pixel, (uint8_t)spec->transparent_color);
        out->after_pass1 = pixel;
        out->after_door = pixel;
        out->after_pass2 = pixel;
    }
    return true;
}

const char *
dm1_v1_viewport_d2l2_d2r2_f0108_wall_composition_source_evidence_pc34(void)
{
    return s_source_evidence;
}
