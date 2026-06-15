#include "dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3L2_VIEW_SQUARE = 14,
    DM1_D3R2_VIEW_SQUARE = 15,
    DM1_D3L2_WALL_ZONE = 702,
    DM1_D3R2_WALL_ZONE = 703,
    DM1_D3L_ADJACENT_WALL_ZONE = 705,
    DM1_D3R_ADJACENT_WALL_ZONE = 706,
    DM1_D3L2_RIGHT_WALL_VIEW = 0,
    DM1_D3R2_LEFT_WALL_VIEW = 1,
    DM1_D3L2_FLOOR_VIEW = 0,
    DM1_D3R2_FLOOR_VIEW = 1,
    DM1_FRONT_WALL_ORNAMENT_SLOT = 552,
    DM1_FLOOR_ORNAMENT_SLOT = 558,
    DM1_D3L2_DOOR_ZONE = 3700,
    DM1_D3R2_DOOR_ZONE = 3710,
    DM1_D3L_ADJACENT_DOOR_ZONE = 3720,
    DM1_D3R_ADJACENT_DOOR_ZONE = 3740,
    DM1_D3_FRONT_BITMAP = 693,
    DM1_D3_DOOR_ORNAMENT_VIEW = 0,
    DM1_D3L2_PASS1_ORDER = 0x0218,
    DM1_D3R2_PASS1_ORDER = 0x0128,
    DM1_D3L2_PASS2_ORDER = 0x0349,
    DM1_D3R2_PASS2_ORDER = 0x0439
};

const char
dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_csb_lineage_evidence_pc34[] =
    "CSB-lineage Viewport.cpp:1192-1209 open-row order reference; "
    "Viewport.cpp:1853-1889 F2L1/F2R1 door-facing routes cross-check "
    "D3L2/D3R2-style side-pair transparency; Viewport.cpp:1903-1915 "
    "F1 door-facing route cross-checks the centered F0111 door-front path.";

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 draws wall ornaments "
    "and returns alcove classification; DUNVIEW.C F0108:3940-4011 draws "
    "floor ornaments with C10 transparency; DUNVIEW.C F0111:4218-4337 "
    "copies and draws door fronts with C10 transparency. DUNVIEW.C:6235-6290 "
    "F0676_DrawD3L2 orders F0108, F0115 pass1, F0111, then F0115 pass2 for "
    "front doors; DUNVIEW.C:6293-6357 F0677_DrawD3R2 mirrors the route. "
    "DUNVIEW.C:6270-6330 is the requested D3L2/D3R2 ornament ordering "
    "anchor. DEFS.H:2608-2611 separates adjacent M601/M602 D3L/D3R from "
    "C14/C15 D3L2/D3R2; DEFS.H:2698-2702 gives M575..M579 while "
    "DEFS.H:2696-2697 gives C00/C01 for D3L2/D3R2; DEFS.H:4042-4046 "
    "places C702/C703 before C705/C706; DEFS.H:4239-4254 places C3700/"
    "C3710 in the same door-zone family as M624/M626. DUNGEON.C:"
    "1769-1838 F0163 maintains square thing lists and DUNGEON.C:"
    "2466-2589 F0172 classifies square aspects before F0676/F0677.";

static const DM1_V1_D3L2D3R2F0111DoorFrontSpecPc34 s_specs[] = {
    {
        DM1_V1_D3L2_D3R2_F0111_SIDE_D3L2_PC34,
        "D3L2 far-left door-front route",
        0,
        DM1_D3L2_VIEW_SQUARE,
        3,
        -2,
        DM1_D3L2_WALL_ZONE,
        DM1_D3L_ADJACENT_WALL_ZONE,
        DM1_D3R_ADJACENT_WALL_ZONE,
        DM1_D3L2_RIGHT_WALL_VIEW,
        DM1_D3L2_FLOOR_VIEW,
        DM1_FRONT_WALL_ORNAMENT_SLOT,
        DM1_D3L2_DOOR_ZONE,
        DM1_D3L_ADJACENT_DOOR_ZONE,
        DM1_D3R_ADJACENT_DOOR_ZONE,
        DM1_D3_FRONT_BITMAP,
        "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR",
        DM1_D3_DOOR_ORNAMENT_VIEW,
        DM1_D3L2_PASS1_ORDER,
        DM1_D3L2_PASS2_ORDER,
        0,
        15,
        25,
        73,
        DM1_V1_D3L2_D3R2_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C:6263 F0107 C00_VIEW_WALL_D3L2_RIGHT wall branch",
        "DUNVIEW.C:6270 F0108 D3L2 front-wall ornament ordinal cross-link",
        "DUNVIEW.C:6272 F0111 uses C3700_ZONE_DOOR_D3L2",
        "DEFS.H:2608-2611,2696-2702,4042-4046,4239-4254",
        "DUNVIEW.C:8478-8482 dispatches D3L2 before D3R2"
    },
    {
        DM1_V1_D3L2_D3R2_F0111_SIDE_D3R2_PC34,
        "D3R2 far-right door-front route",
        1,
        DM1_D3R2_VIEW_SQUARE,
        3,
        2,
        DM1_D3R2_WALL_ZONE,
        DM1_D3L_ADJACENT_WALL_ZONE,
        DM1_D3R_ADJACENT_WALL_ZONE,
        DM1_D3R2_LEFT_WALL_VIEW,
        DM1_D3R2_FLOOR_VIEW,
        DM1_FLOOR_ORNAMENT_SLOT,
        DM1_D3R2_DOOR_ZONE,
        DM1_D3L_ADJACENT_DOOR_ZONE,
        DM1_D3R_ADJACENT_DOOR_ZONE,
        DM1_D3_FRONT_BITMAP,
        "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR",
        DM1_D3_DOOR_ORNAMENT_VIEW,
        DM1_D3R2_PASS1_ORDER,
        DM1_D3R2_PASS2_ORDER,
        208,
        223,
        25,
        73,
        DM1_V1_D3L2_D3R2_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C:6330 F0107 C01_VIEW_WALL_D3R2_LEFT wall branch",
        "DUNVIEW.C:6337 F0108 D3R2 floor ornament ordinal",
        "DUNVIEW.C:6339 F0111 uses C3710_ZONE_DOOR_D3R2",
        "DEFS.H:2608-2611,2696-2702,4042-4046,4239-4254",
        "DUNVIEW.C:8483-8486 dispatches D3R2 after D3L2"
    }
};

size_t dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D3L2D3R2F0111DoorFrontSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D3L2D3R2F0111DoorFrontSpecPc34 *
dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

int dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_decode_cell_pc34(
    unsigned int cell_order,
    int ordinal_index)
{
    unsigned int nibble;

    if (ordinal_index < 0 || ordinal_index >= 4) return -1;
    if ((cell_order & 0xFU) == 0x8U || (cell_order & 0xFU) == 0x9U) {
        cell_order >>= 4;
    }
    nibble = (cell_order >> ((unsigned int)ordinal_index * 4U)) & 0xFU;
    if (nibble == 0U) return -1;
    return (int)nibble - 1;
}

uint8_t dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_compose_pixel_pc34(
    const DM1_V1_D3L2D3R2F0111DoorFrontSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D3L2D3R2F0111DoorFrontPixelPc34 *out)
{
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->viewport_x = viewport_x;
    out->viewport_y = viewport_y;
    out->destination_before = destination_before;
    out->floor_pixel = floor_pixel;
    out->pass1_pixel = pass1_pixel;
    out->door_pixel = door_pixel;
    out->pass2_pixel = pass2_pixel;
    if (!spec) return false;

    if (viewport_x < spec->door_frame_x_first || viewport_x > spec->door_frame_x_last ||
        viewport_y < spec->door_frame_y_first || viewport_y > spec->door_frame_y_last) {
        out->no_write_metadata = true;
        out->after_floor = destination_before;
        out->after_pass1 = destination_before;
        out->after_door = destination_before;
        out->after_pass2 = destination_before;
        return true;
    }

    out->in_clip = true;
    pixel = destination_before;
    out->floor_transparent = floor_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, floor_pixel, (uint8_t)spec->transparent_color);
    out->after_floor = pixel;
    out->pass1_transparent = pass1_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, pass1_pixel, (uint8_t)spec->transparent_color);
    out->after_pass1 = pixel;
    out->door_transparent = door_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, door_pixel, (uint8_t)spec->transparent_color);
    out->after_door = pixel;
    out->pass2_transparent = pass2_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, pass2_pixel, (uint8_t)spec->transparent_color);
    out->after_pass2 = pixel;
    return true;
}

const char *
dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_source_evidence_pc34(void)
{
    return s_source_evidence;
}
