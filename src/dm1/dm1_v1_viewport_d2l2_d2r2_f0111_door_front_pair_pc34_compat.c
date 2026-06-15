#include "dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat.h"

#include <string.h>

enum {
    DM1_D2L_VIEW_SQUARE = 7,
    DM1_D2R_VIEW_SQUARE = 8,
    DM1_D2L2_VIEW_SQUARE = 9,
    DM1_D2R2_VIEW_SQUARE = 10,
    DM1_D2L_RIGHT_ORNAMENT_VIEW = 7,
    DM1_D2R_LEFT_ORNAMENT_VIEW = 8,
    DM1_D2L_FRONT_ORNAMENT_VIEW = 9,
    DM1_D2R_FRONT_ORNAMENT_VIEW = 11,
    DM1_D2L_FLOOR_VIEW = 5,
    DM1_D2R_FLOOR_VIEW = 7,
    DM1_FLOOR_ORNAMENT_SLOT = 558,
    DM1_D2L_DOOR_ZONE = 3750,
    DM1_D2R_DOOR_ZONE = 3770,
    DM1_D2L_DOOR_FRAME_TOP_ZONE = 729,
    DM1_D2R_DOOR_FRAME_TOP_ZONE = 731,
    DM1_D2_FRONT_BITMAP = 694,
    DM1_D2_DOOR_ORNAMENT_VIEW = 1,
    DM1_D2L_PASS1_ORDER = 0x0218,
    DM1_D2R_PASS1_ORDER = 0x0128,
    DM1_D2L_PASS2_ORDER = 0x0349,
    DM1_D2R_PASS2_ORDER = 0x0439
};

const char
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_csb_lineage_evidence_pc34[] =
    "CSB-lineage Viewport.cpp:1192-1209 open-row draw-order cross-reference; "
    "Viewport.cpp:1853-1889 F2L1/F2R1 door-facing routes use floor decoration, "
    "DrawOrder218 or DrawOrder128, StdDrawDoor, then DrawOrder349/439; "
    "Viewport.cpp:1903-1915 F1 center door-facing route has the same "
    "F0111-style door-front transparency ordering.";

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 returns the alcove "
    "boolean after wall-ornament drawing; DUNVIEW.C F0108:3940-4011 draws "
    "floor ornaments with C10 transparency before door-front work; DUNVIEW.C "
    "F0111:4218-4337 copies the door bitmap, applies door ornaments, then "
    "draws with C10 transparency. DUNVIEW.C:6987-7004 F0119 D2L and "
    "DUNVIEW.C:7180-7197 F0120 D2R are the D2-row F0111 front-door routes "
    "named by DEFS.H:2603-2604 M604/M605. DUNVIEW.C:6837-6896 F0678/F0679 "
    "proves the expanded C09/C10 D2L2/D2R2 side squares do not own F0107, "
    "F0108, or F0111. DEFS.H:2698-2707 separates the D3 wall-ornament "
    "M575..M579 family from D2 M580..M584, and DEFS.H:4239-4257 places "
    "M627/M629 in the same door-zone family as M624/M626. DUNGEON.C:"
    "1769-1838 F0163 maintains square thing lists and DUNGEON.C:"
    "2466-2589 F0172 classifies square aspects before viewport branches.";

static const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 s_specs[] = {
    {
        DM1_V1_D2L2_D2R2_F0111_SIDE_D2L_PC34,
        "D2 row left door-front route (requested D2L2/D2R2 pair guard)",
        0,
        DM1_D2L2_VIEW_SQUARE,
        DM1_D2L_VIEW_SQUARE,
        0,
        2,
        -1,
        DM1_D2L_RIGHT_ORNAMENT_VIEW,
        DM1_D2L_FRONT_ORNAMENT_VIEW,
        DM1_D2L_FLOOR_VIEW,
        DM1_FLOOR_ORNAMENT_SLOT,
        DM1_D2L_DOOR_ZONE,
        DM1_D2L_DOOR_FRAME_TOP_ZONE,
        DM1_D2_FRONT_BITMAP,
        "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR",
        DM1_D2_DOOR_ORNAMENT_VIEW,
        DM1_D2L_PASS1_ORDER,
        DM1_D2L_PASS2_ORDER,
        0,
        63,
        24,
        82,
        32,
        61,
        0,
        0,
        DM1_V1_D2L2_D2R2_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34,
        DM1_D2L2_VIEW_SQUARE,
        DM1_D2R2_VIEW_SQUARE,
        1,
        1,
        true,
        true,
        true,
        "DUNVIEW.C:6968-6972 F0107 D2L side/front wall-ornament alcove gate",
        "DUNVIEW.C:6988 and 7020 F0108 D2L floor-ornament call",
        "DUNVIEW.C:6992/7001 F0111 D2L door-front bitmap and zone",
        "DEFS.H:2603-2606 M604/M605 are not C09/C10 D2L2/D2R2",
        "DUNVIEW.C:8503-8513 draws C09 D2L2 before M604 D2L"
    },
    {
        DM1_V1_D2L2_D2R2_F0111_SIDE_D2R_PC34,
        "D2 row right door-front route (requested D2L2/D2R2 pair guard)",
        1,
        DM1_D2R2_VIEW_SQUARE,
        DM1_D2R_VIEW_SQUARE,
        0,
        2,
        1,
        DM1_D2R_LEFT_ORNAMENT_VIEW,
        DM1_D2R_FRONT_ORNAMENT_VIEW,
        DM1_D2R_FLOOR_VIEW,
        DM1_FLOOR_ORNAMENT_SLOT,
        DM1_D2R_DOOR_ZONE,
        DM1_D2R_DOOR_FRAME_TOP_ZONE,
        DM1_D2_FRONT_BITMAP,
        "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR",
        DM1_D2_DOOR_ORNAMENT_VIEW,
        DM1_D2R_PASS1_ORDER,
        DM1_D2R_PASS2_ORDER,
        160,
        223,
        24,
        82,
        32,
        61,
        0,
        0,
        DM1_V1_D2L2_D2R2_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34,
        DM1_D2L2_VIEW_SQUARE,
        DM1_D2R2_VIEW_SQUARE,
        1,
        1,
        true,
        true,
        true,
        "DUNVIEW.C:7119-7123 F0107 D2R side/front wall-ornament alcove gate",
        "DUNVIEW.C:7181 and 7213 F0108 D2R floor-ornament call",
        "DUNVIEW.C:7185/7194 F0111 D2R door-front bitmap and zone",
        "DEFS.H:2603-2606 M604/M605 are not C09/C10 D2L2/D2R2",
        "DUNVIEW.C:8507-8517 draws C10 D2R2 before M605 D2R"
    }
};

size_t dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

int dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_decode_cell_pc34(
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

uint8_t dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_compose_pixel_pc34(
    const DM1_V1_D2L2D2R2F0111DoorFrontSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D2L2D2R2F0111DoorFrontPixelPc34 *out)
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
    pixel = dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, floor_pixel, (uint8_t)spec->transparent_color);
    out->after_floor = pixel;
    out->pass1_transparent = pass1_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, pass1_pixel, (uint8_t)spec->transparent_color);
    out->after_pass1 = pixel;
    out->door_transparent = door_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, door_pixel, (uint8_t)spec->transparent_color);
    out->after_door = pixel;
    out->pass2_transparent = pass2_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_blend_pixel_pc34(
        pixel, pass2_pixel, (uint8_t)spec->transparent_color);
    out->after_pass2 = pixel;
    return true;
}

const char *
dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_source_evidence_pc34(void)
{
    return s_source_evidence;
}
