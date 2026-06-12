#include "dm1_v1_viewport_d3l_d3r_sidewall_pair_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3L_VIEW_SQUARE = 12,
    DM1_D3R_VIEW_SQUARE = 13,
    DM1_D3L_WALL_ZONE = 705,
    DM1_D3R_WALL_ZONE = 706,
    DM1_D3R_WALL_BITMAP = 12,
    DM1_D3L_WALL_BITMAP = 13,
    DM1_D3L_RIGHT_ORNAMENT_VIEW = 2,
    DM1_D3R_LEFT_ORNAMENT_VIEW = 3,
    DM1_D3L_FRONT_ORNAMENT_VIEW = 4,
    DM1_D3C_FRONT_ORNAMENT_VIEW = 5,
    DM1_D3R_FRONT_ORNAMENT_VIEW = 6,
    DM1_D3L_DOOR_ZONE = 3720,
    DM1_D3R_DOOR_ZONE = 3740,
    DM1_D3LCR_FRONT_DOOR_BITMAP = 693,
    DM1_D3LCR_DOOR_ORNAMENT_VIEW = 0,
    DM1_D3L_DOOR_PASS1_ORDER = 0x0218,
    DM1_D3R_DOOR_PASS1_ORDER = 0x0128,
    DM1_D3L_DOOR_PASS2_ORDER = 0x0349,
    DM1_D3R_DOOR_PASS2_ORDER = 0x0439,
    DM1_ALCOVE_CELL_ORDER = 0x0000
};

const char
dm1_v1_viewport_d3l_d3r_sidewall_pair_csb_lineage_viewport_cpp_evidence_pc34[] =
    "CSB-lineage Viewport.cpp:1192-1209 quotes: "
    "ui16 StdDrawF0L1Open[] = { F0L1Contents, F0L1xy, F0L1, "
    "DrawOrder02, StdDrawRoomObjects, F0L1, F0L1xy, "
    "StdDrawCeilingPit, Return }; ui16 StdDrawF0Open[] = { F0, "
    "F0xy, StdDrawCeilingPit, F0Contents, F0xy, F0, DrawOrder21, "
    "StdDrawRoomObjects, Return }; ui16 StdDrawF0R1Open[] = { "
    "F0R1, F0R1xy, StdDrawCeilingPit, F0R1Contents, F0R1xy, F0R1, "
    "DrawOrder01, StdDrawRoomObjects. Viewport.cpp:1903-1915 quotes: "
    "ui16 StdDrawF1DoorFacing[] = { F1, FloorDecorationGraphicOrdinalF1, "
    "StdDrawFloorDecoration, F1Contents, F1xy, F1, DrawOrder218, "
    "StdDrawRoomObjects, StdDoorFacingTopTrackBitmapF1, "
    "StdDoorFacingTopTrackRectF1, StdBltShapeToViewport, "
    "StdDoorFacingFrameLeftBitmapF1, StdDoorFacingFrameLeftRectF1, "
    "StdBltShapeToViewport, StdDoorFacingFrameRightBitmapF1, "
    "StdDoorFacingFrameRightRectF1, StdBltShapeToViewport, F1, "
    "DoorSwitch, JumpZ, 5, Literal, 1, Literal, 3, StdDrawDoorSwitch, "
    "F1DoorRecordIndex, F1DoorState, StdDoorGraphicsF1, StdDoorRectsF1, "
    "StdDrawDoor, F1Contents, F1xy, F1, DrawOrder349, "
    "StdDrawRoomObjects.";

static const char s_source_evidence[] =
    "ReDMCSB anchors: DUNVIEW.C F0107:3502-3938 "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF alcove path; "
    "DUNVIEW.C F0111:4218-4337 F0111_DUNGEONVIEW_DrawDoor door-front "
    "path; DUNVIEW.C F0115:4547-4581,5668-5671 "
    "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF "
    "thing-pass metadata; DUNVIEW.C F0116:6361-6480 and "
    "F0117:6500-6622 D3L/D3R view-square draw path only; "
    "DUNVIEW.C:6432-6600 M575_VIEW_WALL_D3L_RIGHT / "
    "M576_VIEW_WALL_D3R_LEFT / M577_VIEW_WALL_D3L_FRONT / "
    "M578_VIEW_WALL_D3C_FRONT / M579_VIEW_WALL_D3R_FRONT route; "
    "DEFS.H:2088 C10_COLOR_FLESH transparency; DEFS.H:2608-2609 "
    "M601_VIEW_SQUARE_D3L and M602_VIEW_SQUARE_D3R are D3L/D3R, "
    "not D2L/D2R; DEFS.H:2668-2677 and 2698-2702 cell orders and "
    "M575..M579 view-square ordinals; DEFS.H:4045-4046 C705/C706 "
    "wall zones; DEFS.H:4239-4254 M624_ZONE_DOOR_D3L and "
    "M626_ZONE_DOOR_D3R PC34 door-front zones. CSB-lineage "
    "Viewport.cpp:1192-1209,1903-1915 is cross-reference evidence only.";

static const DM1_V1_D3LD3RSidewallSpecPc34 s_specs[] = {
    {
        DM1_V1_D3L_D3R_SIDEWALL_SIDE_D3L_PC34,
        "D3L side-wall pair",
        0,
        DM1_D3L_VIEW_SQUARE,
        3,
        -1,
        DM1_D3L_WALL_ZONE,
        DM1_D3L_WALL_BITMAP,
        DM1_D3R_WALL_BITMAP,
        DM1_D3L_RIGHT_ORNAMENT_VIEW,
        DM1_D3L_FRONT_ORNAMENT_VIEW,
        DM1_D3C_FRONT_ORNAMENT_VIEW,
        DM1_D3L_DOOR_ZONE,
        DM1_D3LCR_FRONT_DOOR_BITMAP,
        "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR",
        DM1_D3LCR_DOOR_ORNAMENT_VIEW,
        DM1_D3L_DOOR_PASS1_ORDER,
        DM1_D3L_DOOR_PASS2_ORDER,
        DM1_ALCOVE_CELL_ORDER,
        DM1_D3L_VIEW_SQUARE,
        1,
        1,
        4,
        0,
        31,
        25,
        75,
        32,
        0,
        DM1_V1_D3L_D3R_SIDEWALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C:6406-6480 F0116 D3L wall, door-front, and thing-pass route",
        "DUNVIEW.C:6432-6435 F0107 calls M575_VIEW_WALL_D3L_RIGHT then M577_VIEW_WALL_D3L_FRONT",
        "DUNVIEW.C:6457 F0111 uses M624_ZONE_DOOR_D3L and G0693 front bitmap",
        "DUNVIEW.C:6448 and 6480 F0115 use M601_VIEW_SQUARE_D3L with C0x0218 then L0200_i_Order"
    },
    {
        DM1_V1_D3L_D3R_SIDEWALL_SIDE_D3R_PC34,
        "D3R side-wall pair",
        1,
        DM1_D3R_VIEW_SQUARE,
        3,
        1,
        DM1_D3R_WALL_ZONE,
        DM1_D3R_WALL_BITMAP,
        DM1_D3L_WALL_BITMAP,
        DM1_D3R_LEFT_ORNAMENT_VIEW,
        DM1_D3R_FRONT_ORNAMENT_VIEW,
        DM1_D3C_FRONT_ORNAMENT_VIEW,
        DM1_D3R_DOOR_ZONE,
        DM1_D3LCR_FRONT_DOOR_BITMAP,
        "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR",
        DM1_D3LCR_DOOR_ORNAMENT_VIEW,
        DM1_D3R_DOOR_PASS1_ORDER,
        DM1_D3R_DOOR_PASS2_ORDER,
        DM1_ALCOVE_CELL_ORDER,
        DM1_D3R_VIEW_SQUARE,
        2,
        2,
        5,
        139,
        202,
        25,
        75,
        0,
        0,
        DM1_V1_D3L_D3R_SIDEWALL_C10_COLOR_FLESH_PC34,
        true,
        true,
        true,
        "DUNVIEW.C:6545-6622 F0117 D3R wall, door-front, and thing-pass route",
        "DUNVIEW.C:6568-6571 F0107 calls M576_VIEW_WALL_D3R_LEFT then M579_VIEW_WALL_D3R_FRONT",
        "DUNVIEW.C:6599 F0111 uses M626_ZONE_DOOR_D3R and G0693 front bitmap",
        "DUNVIEW.C:6584 and 6622 F0115 use M602_VIEW_SQUARE_D3R with C0x0128 then L0202_i_Order"
    }
};

size_t dm1_v1_viewport_d3l_d3r_sidewall_pair_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D3LD3RSidewallSpecPc34 *
dm1_v1_viewport_d3l_d3r_sidewall_pair_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d3l_d3r_sidewall_pair_count_pc34()) return NULL;
    return &s_specs[index];
}

const DM1_V1_D3LD3RSidewallSpecPc34 *
dm1_v1_viewport_d3l_d3r_sidewall_pair_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d3l_d3r_sidewall_pair_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

bool dm1_v1_viewport_d3l_d3r_sidewall_pair_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

int dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_door_zone_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec)
{
    return spec ? spec->door_zone : -1;
}

int dm1_v1_viewport_d3l_d3r_sidewall_pair_f0111_front_bitmap_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec)
{
    return spec ? spec->door_front_bitmap_id : -1;
}

int dm1_v1_viewport_d3l_d3r_sidewall_pair_f0115_view_square_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec)
{
    return spec ? spec->f0115_view_square : -1;
}

int dm1_v1_viewport_d3l_d3r_sidewall_pair_decode_cell_pc34(
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

uint8_t dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3l_d3r_sidewall_pair_compose_pixel_pc34(
    const DM1_V1_D3LD3RSidewallSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t wall_pixel,
    uint8_t ornament_pixel,
    uint8_t door_pixel,
    uint8_t thing_pixel,
    DM1_V1_D3LD3RSidewallPixelPc34 *out)
{
    uint8_t pixel;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->viewport_x = viewport_x;
    out->viewport_y = viewport_y;
    out->destination_before = destination_before;
    out->wall_pixel = wall_pixel;
    out->ornament_pixel = ornament_pixel;
    out->door_pixel = door_pixel;
    out->thing_pixel = thing_pixel;
    if (!spec) return false;

    if (viewport_x < spec->frame_x_first || viewport_x > spec->frame_x_last ||
        viewport_y < spec->frame_y_first || viewport_y > spec->frame_y_last) {
        out->no_write_metadata = true;
        out->after_wall = destination_before;
        out->after_ornament = destination_before;
        out->after_door = destination_before;
        out->after_thing = destination_before;
        return true;
    }

    out->in_clip = true;
    pixel = destination_before;
    out->wall_transparent = wall_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(
        pixel, wall_pixel, (uint8_t)spec->transparent_color);
    out->after_wall = pixel;
    out->ornament_transparent = ornament_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(
        pixel, ornament_pixel, (uint8_t)spec->transparent_color);
    out->after_ornament = pixel;
    out->door_transparent = door_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(
        pixel, door_pixel, (uint8_t)spec->transparent_color);
    out->after_door = pixel;
    out->thing_transparent = thing_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3l_d3r_sidewall_pair_blend_pixel_pc34(
        pixel, thing_pixel, (uint8_t)spec->transparent_color);
    out->after_thing = pixel;
    return true;
}

const char *
dm1_v1_viewport_d3l_d3r_sidewall_pair_source_evidence_pc34(void)
{
    return s_source_evidence;
}
