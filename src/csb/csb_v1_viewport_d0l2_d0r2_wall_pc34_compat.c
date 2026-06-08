#include "csb_v1_viewport_d0l2_d0r2_wall_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D0L2 = 1,
    CSB_SIDE_D0R2 = 2,
    CSB_ELEMENT_WALL = 0,
    CSB_VIEW_SQUARE_D0L = 1,       /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L. */
    CSB_VIEW_SQUARE_D0R = 2,       /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R. */
    CSB_VIEW_DEPTH_D0 = 0,         /* ReDMCSB DUNVIEW.C:8536/8540 F0128. */
    CSB_VIEW_LANE_D0L = -1,        /* ReDMCSB DUNVIEW.C:8536 F0128. */
    CSB_VIEW_LANE_D0R = 1,         /* ReDMCSB DUNVIEW.C:8540 F0128. */
    CSB_WALL_D0R = 0,              /* ReDMCSB DEFS.H:3423 C00_WALL_D0R. */
    CSB_WALL_D0L = 1,              /* ReDMCSB DEFS.H:3424 C01_WALL_D0L. */
    CSB_ZONE_WALL_D0L = 716,       /* ReDMCSB DEFS.H:4056 C716_ZONE_WALL_D0L. */
    CSB_ZONE_WALL_D0R = 717,       /* ReDMCSB DEFS.H:4057 C717_ZONE_WALL_D0R. */
    CSB_C10_COLOR_FLESH = 10,      /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_D0L_FRAME_ROW = 10,        /* ReDMCSB DUNVIEW.C:592-593 G0163 D0L row. */
    CSB_D0R_FRAME_ROW = 11,        /* ReDMCSB DUNVIEW.C:594 G0163 D0R row. */
    CSB_VIEWPORT_WIDTH = 224,
    CSB_VIEWPORT_HEIGHT = 136,
    CSB_D0_SIDE_SOURCE_WIDTH = 32,
    CSB_LINEAGE_RF0L1 = 18,        /* CSB-lineage Viewport.cpp:343. */
    CSB_LINEAGE_RF0R1 = 19,        /* CSB-lineage Viewport.cpp:345. */
    CSB_LINEAGE_F0L1_CONTENTS = 60128, /* CSB-lineage Viewport.cpp:514. */
    CSB_LINEAGE_F0R1_CONTENTS = 60130, /* CSB-lineage Viewport.cpp:516. */
    CSB_LINEAGE_STD_ROOM_OBJECTS = 60006, /* CSB-lineage Viewport.cpp:379. */
    CSB_LINEAGE_DRAWORDER02 = 60288, /* CSB-lineage Viewport.cpp:690/1194. */
    CSB_LINEAGE_DRAWORDER01 = 60287, /* CSB-lineage Viewport.cpp:689/1209. */
    CSB_LINEAGE_DRAWORDER218 = 60279, /* CSB-lineage Viewport.cpp:681/1906. */
    CSB_LINEAGE_DRAWORDER349 = 60280 /* CSB-lineage Viewport.cpp:682/1915. */
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate; no real-asset bitmap parity and no CSB "
    "game-data load. ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_"
    "DrawSquareD0L reads the square aspect through DUNGEON.C:2466-2523 "
    "F0172_DUNGEON_SetSquareAspect, routes C00_ELEMENT_WALL at 8007-8038 "
    "through either F0105(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L) "
    "or F0104(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L), and returns "
    "before the corridor F0115 call at 8005 can be used. ReDMCSB DUNVIEW.C:"
    "8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R mirrors the route at "
    "8117-8144 with F0105(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R) "
    "or F0104(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R), then returns. "
    "ReDMCSB DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 both blit with "
    "C10_COLOR_FLESH transparency, anchored by DEFS.H:2088. ReDMCSB "
    "DUNVIEW.C:4547-4581 F0115 is a thing-pass keep-out for WALL cases; "
    "DUNGEON.C:1769-1838 F0163 and DUNGEON.C:1840-1905 F0164 anchor the thing-list "
    "mutation routines that this wall route must not call or emulate. "
    "ReDMCSB DUNVIEW.C:8478-8508 draws the farther D3L2/D3R2 and D2L2/D2R2 "
    "wall rows first; DUNVIEW.C:8534-8542 then dispatches D0L at relative "
    "0,-1, D0R at relative 0,1, and F0127 D0C after the pair. DUNVIEW.C:"
    "6361-6480 F0116 and DUNVIEW.C:8294 F0127 are anchor-only references, "
    "not duplicated here. DEFS.H:4040-4057 binds the D*-L/D*-R wall-zone "
    "family including C716/C717. CSB-lineage Viewport.cpp:1192-1209 binds "
    "F0L1/F0R1 open side room-object overlays and Viewport.cpp:1903-1915 "
    "binds the distinct door-facing two-pass room-object overlay.";

static const CSB_V1_D0L2D0R2WallRouteSpecPc34 s_routes[] = {
    {
        CSB_SIDE_D0L2,
        "D0L2 wall route via F0125",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        0,
        CSB_VIEW_DEPTH_D0,
        CSB_VIEW_LANE_D0L,
        CSB_VIEW_SQUARE_D0L,
        CSB_ELEMENT_WALL,
        CSB_ZONE_WALL_D0L,
        CSB_D0L_FRAME_ROW,
        0,
        31,
        0,
        135,
        16,
        136,
        CSB_WALL_D0L,
        CSB_WALL_D0R,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        0,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_LINEAGE_RF0L1,
        CSB_LINEAGE_F0L1_CONTENTS,
        CSB_LINEAGE_DRAWORDER02,
        CSB_LINEAGE_STD_ROOM_OBJECTS,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        "ReDMCSB DUNVIEW.C:7960-8062 F0125 and 8536-8537 F0128",
        "ReDMCSB DUNVIEW.C:8017/8033 C716 wall binding",
        s_source_evidence
    },
    {
        CSB_SIDE_D0R2,
        "D0R2 wall route via F0126",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        1,
        CSB_VIEW_DEPTH_D0,
        CSB_VIEW_LANE_D0R,
        CSB_VIEW_SQUARE_D0R,
        CSB_ELEMENT_WALL,
        CSB_ZONE_WALL_D0R,
        CSB_D0R_FRAME_ROW,
        192,
        223,
        0,
        135,
        16,
        136,
        CSB_WALL_D0R,
        CSB_WALL_D0L,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        0,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_LINEAGE_RF0R1,
        CSB_LINEAGE_F0R1_CONTENTS,
        CSB_LINEAGE_DRAWORDER01,
        CSB_LINEAGE_STD_ROOM_OBJECTS,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        "ReDMCSB DUNVIEW.C:8064-8162 F0126 and 8540-8541 F0128",
        "ReDMCSB DUNVIEW.C:8127/8139 C717 wall binding",
        s_source_evidence
    }
};

size_t csb_v1_viewport_d0l2_d0r2_wall_route_spec_count_pc34(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_D0L2D0R2WallRouteSpecPc34 *
csb_v1_viewport_d0l2_d0r2_wall_route_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d0l2_d0r2_wall_route_spec_count_pc34()) {
        return NULL;
    }
    return &s_routes[index];
}

const CSB_V1_D0L2D0R2WallRouteSpecPc34 *
csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(int side)
{
    for (size_t i = 0; i < csb_v1_viewport_d0l2_d0r2_wall_route_spec_count_pc34(); ++i) {
        if (s_routes[i].side == side) return &s_routes[i];
    }
    return NULL;
}

int csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *spec,
    int viewport_x,
    int flipped_variant,
    int *out_source_x)
{
    int local_x;

    if (!spec || !out_source_x) return -1;
    if (viewport_x < spec->wall_frame_x1 || viewport_x > spec->wall_frame_x2) {
        return 1;
    }
    local_x = viewport_x - spec->wall_frame_x1;
    if (local_x < 0 || local_x >= CSB_D0_SIDE_SOURCE_WIDTH) return -1;
    *out_source_x = flipped_variant ? (CSB_D0_SIDE_SOURCE_WIDTH - 1 - local_x) : local_x;
    return 0;
}

uint8_t csb_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    /* ReDMCSB: DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3247
     * both call F0132 with DEFS.H:2088 C10_COLOR_FLESH transparency. */
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

int csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *spec,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    int viewport_x,
    int viewport_y,
    int flipped_variant)
{
    int source_x;
    size_t source_offset;
    size_t viewport_offset;
    int mapped;

    if (!spec || !source || !viewport) return -1;
    if (viewport_y < spec->wall_frame_y1 || viewport_y > spec->wall_frame_y2) return 1;
    if (viewport_y < 0 || viewport_y >= CSB_VIEWPORT_HEIGHT) return -1;
    mapped = csb_v1_viewport_d0l2_d0r2_wall_map_viewport_x_to_source_pc34(
        spec, viewport_x, flipped_variant, &source_x);
    if (mapped != 0) return mapped;
    source_offset = (size_t)(viewport_y - spec->wall_frame_y1) *
                    (size_t)CSB_D0_SIDE_SOURCE_WIDTH + (size_t)source_x;
    viewport_offset = (size_t)viewport_y * (size_t)CSB_VIEWPORT_WIDTH + (size_t)viewport_x;
    if (source_offset >= source_len || viewport_offset >= viewport_len) return -1;
    viewport[viewport_offset] = csb_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(
        viewport[viewport_offset], source[source_offset], (uint8_t)spec->transparent_color);
    return 0;
}

uint16_t csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *spec,
    uint16_t first_thing)
{
    /* ReDMCSB: DUNVIEW.C F0125 lines 8007-8038 and F0126 lines 8117-8144
     * return before F0115, so the wall path never reaches DUNGEON.C F0163
     * or F0164 thing-list mutation contracts. */
    if (!spec || !spec->f0115_thing_pass_keepout) return 0xffffu;
    return first_thing;
}

int csb_v1_viewport_d0l2_d0r2_wall_pc34_compat_run(
    CSB_V1_D0L2D0R2WallRunResultPc34 *out_result)
{
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(CSB_SIDE_D0L2);
    const CSB_V1_D0L2D0R2WallRouteSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_wall_route_spec_for_side_pc34(CSB_SIDE_D0R2);
    uint8_t left_source[CSB_D0_SIDE_SOURCE_WIDTH * CSB_VIEWPORT_HEIGHT];
    uint8_t right_source[CSB_D0_SIDE_SOURCE_WIDTH * CSB_VIEWPORT_HEIGHT];
    uint8_t viewport[CSB_VIEWPORT_WIDTH * CSB_VIEWPORT_HEIGHT];
    CSB_V1_D0L2D0R2WallRunResultPc34 result;
    int copied = 0;

    if (!left || !right) return -1;
    for (size_t i = 0; i < sizeof(left_source); ++i) left_source[i] = CSB_C10_COLOR_FLESH;
    for (size_t i = 0; i < sizeof(right_source); ++i) right_source[i] = CSB_C10_COLOR_FLESH;
    for (size_t i = 0; i < sizeof(viewport); ++i) viewport[i] = 0xeeu;
    left_source[67u * CSB_D0_SIDE_SOURCE_WIDTH + 0u] = 0x21u;
    left_source[67u * CSB_D0_SIDE_SOURCE_WIDTH + 31u] = 0x22u;
    right_source[67u * CSB_D0_SIDE_SOURCE_WIDTH + 0u] = 0x31u;
    right_source[67u * CSB_D0_SIDE_SOURCE_WIDTH + 31u] = 0x32u;

    copied += csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
        left, left_source, sizeof(left_source), viewport, sizeof(viewport), 0, 67, 0) == 0;
    copied += csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
        left, left_source, sizeof(left_source), viewport, sizeof(viewport), 31, 67, 0) == 0;
    copied += csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
        right, right_source, sizeof(right_source), viewport, sizeof(viewport), 192, 67, 1) == 0;
    copied += csb_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
        right, right_source, sizeof(right_source), viewport, sizeof(viewport), 223, 67, 1) == 0;

    result.route_count = (int)csb_v1_viewport_d0l2_d0r2_wall_route_spec_count_pc34();
    result.dispatch_order_ok =
        left->f0128_draw_order_index == 0 &&
        right->f0128_draw_order_index == 1 &&
        left->f0128_relative_depth == 0 &&
        right->f0128_relative_depth == 0 &&
        left->f0128_relative_lateral == -right->f0128_relative_lateral;
    result.wall_variant_binding_ok =
        left->native_wall_index == CSB_WALL_D0L &&
        left->flipped_wall_index == CSB_WALL_D0R &&
        left->wall_zone == CSB_ZONE_WALL_D0L &&
        right->native_wall_index == CSB_WALL_D0R &&
        right->flipped_wall_index == CSB_WALL_D0L &&
        right->wall_zone == CSB_ZONE_WALL_D0R;
    result.c10_transparency_ok =
        viewport[67u * CSB_VIEWPORT_WIDTH + 0u] == 0x21u &&
        viewport[67u * CSB_VIEWPORT_WIDTH + 31u] == 0x22u &&
        viewport[67u * CSB_VIEWPORT_WIDTH + 192u] == 0x32u &&
        viewport[67u * CSB_VIEWPORT_WIDTH + 223u] == 0x31u &&
        viewport[67u * CSB_VIEWPORT_WIDTH + 1u] == 0xeeu &&
        left->transparent_color == CSB_C10_COLOR_FLESH &&
        right->transparent_color == CSB_C10_COLOR_FLESH;
    result.f0115_keepout_ok =
        left->f0115_call_count_for_wall == 0 &&
        right->f0115_call_count_for_wall == 0 &&
        left->f0115_thing_pass_keepout &&
        right->f0115_thing_pass_keepout &&
        left->f0111_door_keepout &&
        right->f0111_door_keepout;
    result.first_thing_before = 0x1234u;
    result.first_thing_after =
        csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
            right,
            csb_v1_viewport_d0l2_d0r2_wall_preserve_first_thing_pc34(
                left, result.first_thing_before));
    result.thing_list_keepout_ok =
        result.first_thing_after == result.first_thing_before &&
        left->thing_list_link_mutation == 0 &&
        right->thing_list_link_mutation == 0 &&
        left->thing_list_unlink_mutation == 0 &&
        right->thing_list_unlink_mutation == 0;
    result.row_followup_ok =
        left->f0127_d0c_followup_after_pair &&
        right->f0127_d0c_followup_after_pair;
    result.lineage_binding_ok =
        left->lineage_contents_opcode == CSB_LINEAGE_F0L1_CONTENTS &&
        right->lineage_contents_opcode == CSB_LINEAGE_F0R1_CONTENTS &&
        left->lineage_room_objects_opcode == CSB_LINEAGE_STD_ROOM_OBJECTS &&
        right->lineage_room_objects_opcode == CSB_LINEAGE_STD_ROOM_OBJECTS &&
        left->lineage_door_facing_first_order_opcode == CSB_LINEAGE_DRAWORDER218 &&
        right->lineage_door_facing_second_order_opcode == CSB_LINEAGE_DRAWORDER349;
    result.copied_pixels = copied;
    result.ok = result.route_count == 2 &&
                result.dispatch_order_ok &&
                result.wall_variant_binding_ok &&
                result.c10_transparency_ok &&
                result.f0115_keepout_ok &&
                result.thing_list_keepout_ok &&
                result.row_followup_ok &&
                result.lineage_binding_ok &&
                result.copied_pixels == 4;
    if (out_result) *out_result = result;
    return result.ok ? 0 : 1;
}

const char *csb_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
