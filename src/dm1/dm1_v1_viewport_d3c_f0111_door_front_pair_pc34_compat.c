#include "dm1_v1_viewport_d3c_f0111_door_front_pair_pc34_compat.h"

#include <string.h>

enum {
    DM1_D3C_VIEW_SQUARE = 11,
    DM1_D3C_WALL_ZONE = 704,
    DM1_D3C_WALL_ORNAMENT_VIEW = 5,
    DM1_D3C_FLOOR_ORNAMENT_VIEW = 3,
    DM1_FRONT_WALL_ORNAMENT_SLOT = 552,
    DM1_FLOOR_ORNAMENT_SLOT = 558,
    DM1_D3L_DOOR_ZONE = 3720,
    DM1_D3C_DOOR_ZONE = 3730,
    DM1_D3R_DOOR_ZONE = 3740,
    DM1_D2C_DOOR_ZONE = 3760,
    DM1_D2R_DOOR_ZONE = 3770,
    DM1_D3C_DOOR_FRAME_LEFT_ZONE = 722,
    DM1_D3C_DOOR_FRAME_RIGHT_ZONE = 723,
    DM1_D3C_DOOR_FRAME_BITMAP = 2119,
    DM1_D3_FRONT_BITMAP = 693,
    DM1_D3_DOOR_ORNAMENT_VIEW = 0,
    DM1_D3C_PASS1_ORDER = 0x0218,
    DM1_D3C_PASS2_ORDER = 0x0349
};

static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0118:6721-6747 is the D3C "
    "door-front composition: F0108 floor ornament, F0115 door pass 1, "
    "F0104 left door-frame bitmap, F0105 flipped right door-frame bitmap, "
    "F0111 door-front, then F0115 pass 2. DUNVIEW.C F0111:4218-4337 "
    "copies the native door bitmap through the 4243-4266 path, draws door "
    "ornaments, and calls F0791 "
    "with C10_COLOR_FLESH. DUNVIEW.C F0107:3502-3938 owns wall-ornament "
    "palette/alcove behavior and is intentionally kept out of the D3C "
    "door-front frame; DUNVIEW.C:6716-6720 is the D3C wall-only F0107 "
    "return path. DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 provide "
    "the native and flipped C10 blits used by D3C door-frame halves at "
    "DUNVIEW.C:6734-6735. DUNVIEW.C F0115:4547-4581 defines the thing-pass "
    "cell ordering used at DUNVIEW.C:6723 and 6746-6747. DUNVIEW.C "
    "F0128:8478-8508 and 8534-8542 dispatches D3C after D3L/D3R and before "
    "D2/D1/D0. DEFS.H:2088 defines C10_COLOR_FLESH; DEFS.H:2668-2677 gives "
    "the door-pass cell orders; DEFS.H:2698-2702 gives M575..M579 wall "
    "views with M578_VIEW_WALL_D3C_FRONT=5; DEFS.H:4044-4046 gives C704 "
    "D3C plus adjacent C705/C706 wall zones; DEFS.H:4239-4254 gives the "
    "M624_ZONE_DOOR_D3L/M625/M626 door-zone family where PC34 D3C is "
    "M625_ZONE_DOOR_D3C=3730, disjoint from M628_ZONE_DOOR_D2C and "
    "M629_ZONE_DOOR_D2R.";

static const DM1_V1_D3CF0111DoorFrontSpecPc34 s_spec = {
    0,
    "D3C centered door-front route with wall-ornament keepout",
    0,
    DM1_D3C_VIEW_SQUARE,
    3,
    0,
    DM1_D3C_WALL_ZONE,
    DM1_D3C_WALL_ORNAMENT_VIEW,
    DM1_D3C_FLOOR_ORNAMENT_VIEW,
    DM1_FLOOR_ORNAMENT_SLOT,
    DM1_FRONT_WALL_ORNAMENT_SLOT,
    DM1_D3C_DOOR_ZONE,
    DM1_D3L_DOOR_ZONE,
    DM1_D3R_DOOR_ZONE,
    DM1_D2C_DOOR_ZONE,
    DM1_D2R_DOOR_ZONE,
    DM1_D3C_DOOR_FRAME_LEFT_ZONE,
    DM1_D3C_DOOR_FRAME_RIGHT_ZONE,
    DM1_D3C_DOOR_FRAME_BITMAP,
    DM1_D3_FRONT_BITMAP,
    "G0693_ai_DoorNativeBitmapIndex_Front_D3LCR",
    DM1_D3_DOOR_ORNAMENT_VIEW,
    DM1_D3C_PASS1_ORDER,
    DM1_D3C_PASS2_ORDER,
    88,
    135,
    28,
    67,
    24,
    41,
    0,
    0,
    DM1_V1_D3C_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34,
    DM1_V1_D3C_F0111_DOOR_FRONT_C45_PRESERVED_PIXEL_PC34,
    true,
    true,
    true,
    "DUNVIEW.C:6734 F0104 native C10 left door-frame blit",
    "DUNVIEW.C:6735 F0105 flipped C10 right door-frame blit",
    "DUNVIEW.C:3502-3938 F0107 wall-ornament palette; 6716-6720 D3C wall keepout",
    "DUNVIEW.C:6744 F0111 D3C M625_ZONE_DOOR_D3C; F0111:4243-4266 copy/ornament path",
    "DUNVIEW.C:6723 pass1 and 6746-6747 pass2; F0115:4547-4581 ordering",
    "DUNVIEW.C:8478-8508,8534-8542 F0128 D3C dispatch position",
    "DEFS.H:2088,2668-2677,2698-2702,4044-4046,4239-4254"
};

static const DM1_V1_D3CF0111WallOrnamentPc34 s_wall_ornament = {
    1,
    DM1_D3C_WALL_ORNAMENT_VIEW,
    0,
    0,
    1004 + DM1_D3C_WALL_ORNAMENT_VIEW,
    10,
    DM1_V1_D3C_F0111_DOOR_FRONT_C10_COLOR_FLESH_PC34,
    { 74, 149, 25, 75 },
    { 88, 135, 28, 67 },
    true,
    true,
    "DUNVIEW.C F0107:3502-3938 palette/F0791 wall ornament; "
    "DUNVIEW.C:6716-6720 returns before door-front; DEFS.H:2701 M578"
};

static const DM1_V1_D3CF0111DoorFrontStepInfoPc34 s_steps[] = {
    {
        DM1_V1_D3C_F0111_STEP_F0108_FLOOR_ORNAMENT_PC34,
        "F0108 floor ornament before door pass",
        "DUNVIEW.C:6722 F0108_DUNGEONVIEW_DrawFloorOrnament"
    },
    {
        DM1_V1_D3C_F0111_STEP_F0115_PASS1_PC34,
        "F0115 back cells before door",
        "DUNVIEW.C:6723 C0x0218 door pass 1; F0115:4547-4581"
    },
    {
        DM1_V1_D3C_F0111_STEP_F0104_LEFT_FRAME_PC34,
        "F0104 native left door frame",
        "DUNVIEW.C:6734 F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap"
    },
    {
        DM1_V1_D3C_F0111_STEP_F0105_RIGHT_FRAME_FLIPPED_PC34,
        "F0105 flipped right door frame",
        "DUNVIEW.C:6735 F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally"
    },
    {
        DM1_V1_D3C_F0111_STEP_F0111_DOOR_FRONT_PC34,
        "F0111 centered door front",
        "DUNVIEW.C:6744 F0111_DUNGEONVIEW_DrawDoor M625_ZONE_DOOR_D3C"
    },
    {
        DM1_V1_D3C_F0111_STEP_F0115_PASS2_PC34,
        "F0115 front cells after door",
        "DUNVIEW.C:6746-6747 C0x0349 door pass 2; F0115:4547-4581"
    }
};

size_t dm1_v1_viewport_d3c_f0111_door_front_pair_count_pc34(void)
{
    return 1U;
}

const DM1_V1_D3CF0111DoorFrontSpecPc34 *
dm1_v1_viewport_d3c_f0111_door_front_pair_at_pc34(size_t index)
{
    return index == 0U ? &s_spec : NULL;
}

const DM1_V1_D3CF0111DoorFrontSpecPc34 *
dm1_v1_viewport_d3c_f0111_door_front_pair_center_pc34(void)
{
    return &s_spec;
}

size_t dm1_v1_viewport_d3c_f0111_door_front_pair_steps_pc34(
    DM1_V1_D3CF0111DoorFrontStepInfoPc34 *out,
    size_t cap)
{
    size_t i;
    const size_t count = sizeof(s_steps) / sizeof(s_steps[0]);
    const size_t n = cap < count ? cap : count;

    if (out) {
        for (i = 0; i < n; ++i) {
            out[i] = s_steps[i];
        }
    }
    return count;
}

const DM1_V1_D3CF0111WallOrnamentPc34 *
dm1_v1_viewport_d3c_f0111_wall_ornament_keepout_pc34(void)
{
    return &s_wall_ornament;
}

bool dm1_v1_viewport_d3c_f0111_f0107_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

bool dm1_v1_viewport_d3c_f0111_rects_overlap_pc34(
    DM1_V1_D3CF0111DoorFrontRectPc34 a,
    DM1_V1_D3CF0111DoorFrontRectPc34 b)
{
    return a.left <= b.right && a.right >= b.left &&
           a.top <= b.bottom && a.bottom >= b.top;
}

int dm1_v1_viewport_d3c_f0111_decode_cell_pc34(
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

uint8_t dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d3c_f0111_compose_pixel_pc34(
    const DM1_V1_D3CF0111DoorFrontSpecPc34 *spec,
    int viewport_x,
    int viewport_y,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t left_frame_pixel,
    uint8_t right_frame_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D3CF0111DoorFrontPixelPc34 *out)
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
    out->left_frame_pixel = left_frame_pixel;
    out->right_frame_pixel = right_frame_pixel;
    out->door_pixel = door_pixel;
    out->pass2_pixel = pass2_pixel;
    if (!spec) return false;

    if (viewport_x < spec->door_frame_x_first || viewport_x > spec->door_frame_x_last ||
        viewport_y < spec->door_frame_y_first || viewport_y > spec->door_frame_y_last) {
        out->outside_untouched = true;
        out->after_floor = destination_before;
        out->after_pass1 = destination_before;
        out->after_left_frame = destination_before;
        out->after_right_frame = destination_before;
        out->after_door = destination_before;
        out->after_pass2 = destination_before;
        return true;
    }

    out->in_frame = true;
    pixel = destination_before;
    out->floor_transparent = floor_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
        pixel, floor_pixel, (uint8_t)spec->transparent_color);
    out->after_floor = pixel;
    out->pass1_transparent = pass1_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
        pixel, pass1_pixel, (uint8_t)spec->transparent_color);
    out->after_pass1 = pixel;
    out->left_frame_transparent = left_frame_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
        pixel, left_frame_pixel, (uint8_t)spec->transparent_color);
    out->after_left_frame = pixel;
    out->right_frame_transparent = right_frame_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
        pixel, right_frame_pixel, (uint8_t)spec->transparent_color);
    out->after_right_frame = pixel;
    out->door_transparent = door_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
        pixel, door_pixel, (uint8_t)spec->transparent_color);
    out->after_door = pixel;
    out->pass2_transparent = pass2_pixel == spec->transparent_color;
    pixel = dm1_v1_viewport_d3c_f0111_blend_pixel_pc34(
        pixel, pass2_pixel, (uint8_t)spec->transparent_color);
    out->after_pass2 = pixel;
    return true;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    hash ^= value & 0xffU;
    hash *= 16777619U;
    hash ^= (value >> 8) & 0xffU;
    hash *= 16777619U;
    hash ^= (value >> 16) & 0xffU;
    hash *= 16777619U;
    hash ^= (value >> 24) & 0xffU;
    hash *= 16777619U;
    return hash;
}

uint32_t dm1_v1_viewport_d3c_f0111_door_front_pair_hash_pc34(void)
{
    uint32_t h = 2166136261U;

    h = hash_u32(h, (uint32_t)s_spec.view_square_index);
    h = hash_u32(h, (uint32_t)s_spec.wall_zone);
    h = hash_u32(h, (uint32_t)s_spec.wall_ornament_view);
    h = hash_u32(h, (uint32_t)s_spec.floor_ornament_view);
    h = hash_u32(h, (uint32_t)s_spec.door_zone);
    h = hash_u32(h, (uint32_t)s_spec.door_frame_left_zone);
    h = hash_u32(h, (uint32_t)s_spec.door_frame_right_zone);
    h = hash_u32(h, (uint32_t)s_spec.front_bitmap_id);
    h = hash_u32(h, s_spec.pass1_cell_order);
    h = hash_u32(h, s_spec.pass2_cell_order);
    h = hash_u32(h, (uint32_t)s_spec.door_frame_x_first);
    h = hash_u32(h, (uint32_t)s_spec.door_frame_x_last);
    h = hash_u32(h, (uint32_t)s_spec.door_frame_y_first);
    h = hash_u32(h, (uint32_t)s_spec.door_frame_y_last);
    h = hash_u32(h, (uint32_t)s_spec.transparent_color);
    h = hash_u32(h, (uint32_t)s_wall_ornament.keepout_rect.left);
    h = hash_u32(h, (uint32_t)s_wall_ornament.keepout_rect.right);
    h = hash_u32(h, (uint32_t)s_wall_ornament.keepout_rect.top);
    h = hash_u32(h, (uint32_t)s_wall_ornament.keepout_rect.bottom);
    return h;
}

const char *
dm1_v1_viewport_d3c_f0111_door_front_pair_source_evidence_pc34(void)
{
    return s_source_evidence;
}
