#include "dm1/dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat.h"

#include <string.h>

enum {
    DM1_PRESENT = 1,
    DM1_VIEW_SQUARE_D0L = 1,     /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L. */
    DM1_VIEW_SQUARE_D0R = 2,     /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R. */
    DM1_VIEW_SQUARE_D3L = 12,    /* ReDMCSB DEFS.H:2608 M601_VIEW_SQUARE_D3L. */
    DM1_VIEW_SQUARE_D3R = 13,    /* ReDMCSB DEFS.H:2609 M602_VIEW_SQUARE_D3R. */
    DM1_WALL_ZONE_D3L = 705,     /* ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L. */
    DM1_WALL_ZONE_D3R = 706,     /* ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R. */
    DM1_DOOR_ZONE_D3L = 3720,    /* ReDMCSB DEFS.H:4252 M624_ZONE_DOOR_D3L. */
    DM1_DOOR_ZONE_D3R = 3740,    /* ReDMCSB DEFS.H:4254 M626_ZONE_DOOR_D3R. */
    DM1_DOOR_FRAME_LEFT_D3L = 718,
    DM1_DOOR_FRAME_RIGHT_D3L = 719,
    DM1_DOOR_FRAME_LEFT_D3R = 720,
    DM1_DOOR_FRAME_RIGHT_D3R = 721,
    DM1_FLOOR_VIEW_D3L = 2,      /* ReDMCSB DEFS.H:2752 M588_VIEW_FLOOR_D3L. */
    DM1_FLOOR_VIEW_D3R = 4,      /* ReDMCSB DEFS.H:2754 M590_VIEW_FLOOR_D3R. */
    DM1_DOOR_FRONT_BITMAP_D3LCR = 693,
    DM1_DOOR_ORNAMENT_D3LCR = 0,
    DM1_D3L_PASS1 = 0x0218,
    DM1_D3R_PASS1 = 0x0128,
    DM1_D3L_PASS2 = 0x0349,
    DM1_D3R_PASS2 = 0x0439
};

static const char s_source_evidence[] =
    "DM1 V1 D0L/D0R F0111 door-front source-lock; contract-only, "
    "asset-free, deterministic, and no game-data load. Required anchors: "
    "DUNVIEW.C F0111:4218-4337 copies the native door bitmap, applies "
    "door ornaments, decrements partly-open states at 4308, selects "
    "LeftHorizontal and RightHorizontal frames at 4312-4313, performs the "
    "horizontal C6 transparent half-blit at 4322-4324, applies "
    "3|MASK0x4000 at 4325, and finishes with F0791 using "
    "C10_COLOR_FLESH at 4334. DUNVIEW.C F0128:8318-8486 and 8536-8541 "
    "dispatch the depth-0 left/right side cells through F0125/F0126. "
    "DUNVIEW.C F0125:7960-8062 and F0126:8064-8162 are the D0L/D0R "
    "source dispatchers; the door-front composition used for the left and "
    "right corridor wall cells is the same F0111 shape as D3L/D3R callers "
    "at DUNVIEW.C:6442-6460 and 6578-6602. DUNVIEW.C F0104:3113-3156, "
    "F0105:3185-3247, and F0107:3502-3938 are the wall composition callers "
    "around those routes. DUNGEON.C F0163:1769-1838 and F0164:1840-1905 "
    "are thing-list mutation anchors not called by the draw contract, and "
    "F0172:2466-2523 supplies square aspect. DEFS.H:2088 C10_COLOR_FLESH; "
    "2596-2611 view squares; 2662 and 2668-2677 cell orders; 4045-4046 "
    "C705/C706 wall zones; and 4139-4153 cell-order zone band are pinned.";

static const DM1_V1_D0LD0RF0111DoorSpecPc34 s_specs[] = {
    {
        DM1_V1_D0L_D0R_F0111_SIDE_D0L_PC34,
        "D0L corridor-left wall-cell F0111 door-front contract",
        DM1_VIEW_SQUARE_D0L,
        DM1_VIEW_SQUARE_D3L,
        8536,
        8537,
        7960,
        8062,
        0,
        -1,
        3,
        -1,
        DM1_WALL_ZONE_D3L,
        DM1_DOOR_ZONE_D3L,
        DM1_DOOR_FRAME_LEFT_D3L,
        DM1_DOOR_FRAME_RIGHT_D3L,
        DM1_FLOOR_VIEW_D3L,
        DM1_DOOR_FRONT_BITMAP_D3LCR,
        DM1_DOOR_ORNAMENT_D3LCR,
        DM1_D3L_PASS1,
        DM1_D3L_PASS2,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        { 24, 71, 28, 67, 24, 41, 0, 0 },
        { { 24, 71, 28, 38, 24, 41, 0, 30 },
          { 24, 71, 28, 48, 24, 41, 0, 20 },
          { 24, 71, 28, 58, 24, 41, 0, 10 } },
        { { 24, 29, 28, 67, 24, 41, 18, 0 },
          { 24, 35, 28, 67, 24, 41, 12, 0 },
          { 24, 41, 28, 67, 24, 41, 6, 0 } },
        { { 66, 71, 28, 67, 24, 41, 24, 0 },
          { 60, 71, 28, 67, 24, 41, 24, 0 },
          { 54, 71, 28, 67, 24, 41, 24, 0 } },
        "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
        "ReDMCSB DUNVIEW.C:8318-8486/8536-8541 F0128_DUNGEONVIEW_Draw_CPSF",
        "ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L",
        "ReDMCSB DUNVIEW.C:3113-3156 F0104; 3185-3247 F0105; 3502-3938 F0107",
        "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "ReDMCSB DEFS.H:2088,2596-2611,2662,2668-2677,4045-4046,4139-4153",
        "ReDMCSB DUNVIEW.C:6442-6460 F0116 D3L door-front caller"
    },
    {
        DM1_V1_D0L_D0R_F0111_SIDE_D0R_PC34,
        "D0R corridor-right wall-cell F0111 door-front contract",
        DM1_VIEW_SQUARE_D0R,
        DM1_VIEW_SQUARE_D3R,
        8540,
        8541,
        8064,
        8162,
        0,
        1,
        3,
        1,
        DM1_WALL_ZONE_D3R,
        DM1_DOOR_ZONE_D3R,
        DM1_DOOR_FRAME_LEFT_D3R,
        DM1_DOOR_FRAME_RIGHT_D3R,
        DM1_FLOOR_VIEW_D3R,
        DM1_DOOR_FRONT_BITMAP_D3LCR,
        DM1_DOOR_ORNAMENT_D3LCR,
        DM1_D3R_PASS1,
        DM1_D3R_PASS2,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        DM1_PRESENT,
        { 150, 197, 28, 67, 24, 41, 0, 0 },
        { { 150, 197, 28, 38, 24, 41, 0, 30 },
          { 150, 197, 28, 48, 24, 41, 0, 20 },
          { 150, 197, 28, 58, 24, 41, 0, 10 } },
        { { 150, 153, 28, 67, 24, 41, 18, 0 },
          { 150, 161, 28, 67, 24, 41, 12, 0 },
          { 150, 167, 28, 67, 24, 41, 6, 0 } },
        { { 192, 197, 28, 67, 24, 41, 24, 0 },
          { 186, 197, 28, 67, 24, 41, 24, 0 },
          { 180, 197, 28, 67, 24, 41, 24, 0 } },
        "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
        "ReDMCSB DUNVIEW.C:8318-8486/8536-8541 F0128_DUNGEONVIEW_Draw_CPSF",
        "ReDMCSB DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R",
        "ReDMCSB DUNVIEW.C:3113-3156 F0104; 3185-3247 F0105; 3502-3938 F0107",
        "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "ReDMCSB DEFS.H:2088,2596-2611,2662,2668-2677,4045-4046,4139-4153",
        "ReDMCSB DUNVIEW.C:6578-6602 F0117 D3R door-front caller"
    }
};

size_t dm1_v1_viewport_d0l_d0r_f0111_door_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D0LD0RF0111DoorSpecPc34 *
dm1_v1_viewport_d0l_d0r_f0111_door_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d0l_d0r_f0111_door_count_pc34()) return NULL;
    return &s_specs[index];
}

const DM1_V1_D0LD0RF0111DoorSpecPc34 *
dm1_v1_viewport_d0l_d0r_f0111_door_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d0l_d0r_f0111_door_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return NULL;
}

uint8_t dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    return source_pixel == DM1_V1_D0L_D0R_F0111_C10_COLOR_FLESH_PC34 ?
        destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d0l_d0r_f0111_door_compose_closed_pixel_pc34(
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec,
    uint8_t destination_before,
    uint8_t floor_pixel,
    uint8_t pass1_pixel,
    uint8_t door_pixel,
    uint8_t pass2_pixel,
    DM1_V1_D0LD0RF0111DoorPixelTracePc34 *out)
{
    uint8_t pixel;

    if (!spec || !out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->in_closed_clip = true;
    out->destination_before = destination_before;
    out->floor_transparent =
        floor_pixel == DM1_V1_D0L_D0R_F0111_C10_COLOR_FLESH_PC34;
    out->pass1_transparent =
        pass1_pixel == DM1_V1_D0L_D0R_F0111_C10_COLOR_FLESH_PC34;
    out->door_transparent =
        door_pixel == DM1_V1_D0L_D0R_F0111_C10_COLOR_FLESH_PC34;
    out->pass2_transparent =
        pass2_pixel == DM1_V1_D0L_D0R_F0111_C10_COLOR_FLESH_PC34;

    pixel = dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(destination_before,
                                                           floor_pixel);
    out->after_floor = pixel;
    pixel = dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(pixel, pass1_pixel);
    out->after_pass1 = pixel;
    pixel = dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(pixel, door_pixel);
    out->after_door = pixel;
    pixel = dm1_v1_viewport_d0l_d0r_f0111_door_blend_pc34(pixel, pass2_pixel);
    out->after_pass2 = pixel;
    return true;
}

bool dm1_v1_viewport_d0l_d0r_f0111_door_state_trace_pc34(
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec,
    int door_state,
    bool vertical,
    DM1_V1_D0LD0RF0111DoorStateTracePc34 *out)
{
    int decremented;

    if (!spec || !out) return false;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->input_state = door_state;
    out->first_zone = spec->door_zone;
    out->c6_zone = -1;
    out->final_zone = spec->door_zone;
    if (door_state == DM1_V1_D0L_D0R_F0111_DOOR_STATE_OPEN_PC34) {
        out->skipped_open_guard = true;
        out->decremented_state = -1;
        return true;
    }
    if (door_state == DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_PC34 ||
        door_state == DM1_V1_D0L_D0R_F0111_DOOR_STATE_DESTROYED_PC34) {
        out->closed_or_destroyed_frame_selected = true;
        out->decremented_state = door_state;
        out->selected_closed_or_vertical = spec->closed_or_destroyed;
        return true;
    }
    if (door_state < DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_ONE_FOURTH_PC34 ||
        door_state > DM1_V1_D0L_D0R_F0111_DOOR_STATE_CLOSED_THREE_FOURTH_PC34) {
        return false;
    }

    decremented = door_state - 1;
    out->decremented_state = decremented;
    out->first_zone = spec->door_zone + decremented;
    if (vertical) {
        out->vertical_frame_selected = true;
        out->selected_closed_or_vertical = spec->vertical[decremented];
        out->final_zone = out->first_zone;
    } else {
        out->left_horizontal_selected = true;
        out->right_horizontal_selected = true;
        out->horizontal_c6_transparent_blit = true;
        out->mask0x4000_applied = true;
        out->selected_left_horizontal = spec->left_horizontal[decremented];
        out->selected_right_horizontal = spec->right_horizontal[decremented];
        out->zone_shift_x = spec->closed_or_destroyed.byte_width >> 1;
        out->zone_shift_y = 0;
        out->c6_zone = out->first_zone + DM1_V1_D0L_D0R_F0111_C6_UNKNOWN_PC34;
        out->final_zone = out->first_zone + 3 +
            DM1_V1_D0L_D0R_F0111_MASK0X4000_PC34;
    }
    return true;
}

int dm1_v1_viewport_d0l_d0r_f0111_door_decode_cell_pc34(
    unsigned int order,
    int ordinal)
{
    unsigned int cell;

    if (ordinal < 0 || ordinal >= 4) return -1;
    if ((order & 0x0fu) == 0x08u || (order & 0x0fu) == 0x09u) {
        order >>= 4;
    }
    cell = (order >> ((unsigned int)ordinal * 4u)) & 0x0fu;
    if (cell == 0u) return -1;
    return (int)cell - 1;
}

bool dm1_v1_viewport_d0l_d0r_f0111_dispatch_pc34(
    const DM1_V1_D0LD0RF0111DoorSpecPc34 *spec,
    int direction,
    int origin_x,
    int origin_y,
    DM1_V1_D0LD0RF0111DispatchTracePc34 *out)
{
    int depth;
    int lateral;

    if (!spec || !out || direction < 0 || direction > 3) return false;
    memset(out, 0, sizeof(*out));
    depth = spec->requested_depth;
    lateral = spec->requested_lateral;
    out->spec = spec;
    out->direction = direction;
    out->origin_x = origin_x;
    out->origin_y = origin_y;
    out->relative_depth = depth;
    out->relative_lateral = lateral;
    out->f0128_update_line = spec->f0128_update_line;
    out->f0128_draw_line = spec->f0128_draw_line;
    out->draw_function = spec->side == DM1_V1_D0L_D0R_F0111_SIDE_D0L_PC34 ?
        "F0125_DUNGEONVIEW_DrawSquareD0L" :
        "F0126_DUNGEONVIEW_DrawSquareD0R";

    switch (direction) {
    case 0:
        out->updated_x = origin_x + lateral;
        out->updated_y = origin_y - depth;
        break;
    case 1:
        out->updated_x = origin_x + depth;
        out->updated_y = origin_y + lateral;
        break;
    case 2:
        out->updated_x = origin_x - lateral;
        out->updated_y = origin_y + depth;
        break;
    default:
        out->updated_x = origin_x - depth;
        out->updated_y = origin_y - lateral;
        break;
    }
    return true;
}

const char *
dm1_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
