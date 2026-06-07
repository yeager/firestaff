#include "csb_v1_viewport_d2l2_d2r2_wall_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_D2L2_VIEW_SQUARE = 9,              /* ReDMCSB: DEFS.H:2605 C09_VIEW_SQUARE_D2L2. */
    CSB_D2R2_VIEW_SQUARE = 10,             /* ReDMCSB: DEFS.H:2606 C10_VIEW_SQUARE_D2R2. */
    CSB_D2_VIEW_DEPTH = 2,                 /* ReDMCSB: DUNVIEW.C:8503/8507 F0128. */
    CSB_D2L2_VIEW_LATERAL = -2,            /* ReDMCSB: DUNVIEW.C:8503 F0128 D2L2. */
    CSB_D2R2_VIEW_LATERAL = 2,             /* ReDMCSB: DUNVIEW.C:8507 F0128 D2R2. */
    CSB_D2L2_WALL_ZONE = 707,              /* ReDMCSB: DEFS.H:4047 C707_ZONE_WALL_D2L2. */
    CSB_D2R2_WALL_ZONE = 708,              /* ReDMCSB: DEFS.H:4048 C708_ZONE_WALL_D2R2. */
    CSB_ELEMENT_WALL = 0,                  /* ReDMCSB: DEFS.H:1007 C00_ELEMENT_WALL. */
    CSB_ELEMENT_TELEPORTER = 5,            /* ReDMCSB: DEFS.H:1012 C05_ELEMENT_TELEPORTER. */
    CSB_WALL_D2R2 = 5,                     /* ReDMCSB: DEFS.H:3428 C05_WALL_D2R2. */
    CSB_WALL_D2L2 = 6,                     /* ReDMCSB: DEFS.H:3429 C06_WALL_D2L2. */
    CSB_PC34_WALL_DELTA = 2,               /* ReDMCSB: DUNVIEW.C:6855-6857/6886-6888. */
    CSB_TRANSPARENT_COLOR = 10,            /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_D2L2_FIELD_ASPECT = 5,             /* ReDMCSB: DUNVIEW.C:377 G2035[9]. */
    CSB_D2R2_FIELD_ASPECT = 6,             /* ReDMCSB: DUNVIEW.C:377 G2035[10]. */
    CSB_LINEAGE_FRAME_BLIT_COMMAND = 60200,/* CSBWin Viewport.cpp:592,1817. */
    CSB_LINEAGE_FRAME_RECT_COMMAND = 60250,/* CSBWin Viewport.cpp:650,1817. */
    CSB_LINEAGE_D2L2_PWALLBITMAP = 5,      /* CSBWin Viewport.cpp:2267,3379. */
    CSB_LINEAGE_D2R2_PWALLBITMAP = 6,      /* CSBWin Viewport.cpp:2271,3390. */
    CSB_LINEAGE_D2L2_WALLRECT = 13,        /* CSBWin Viewport.cpp:3379. */
    CSB_LINEAGE_D2R2_WALLRECT = 12         /* CSBWin Viewport.cpp:3390. */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap parity and no CSB "
    "game-data load. ReDMCSB DUNVIEW.C:6837-6865 F0678_DrawD2L2 switches on "
    "C0_ELEMENT, routes C00_ELEMENT_WALL through F0104 with "
    "G2107_WallSet[C06_WALL_D2L2 + 2] and C707_ZONE_WALL_D2L2 under "
    "PC_FIX_CODE_SIZE, routes MEDIA709 through F0105 with C05_WALL_D2R2, "
    "returns before F0111/F0115, and routes teleporter fields through "
    "G2035[C09_VIEW_SQUARE_D2L2]. DUNVIEW.C:6868-6896 F0679_DrawD2R2 mirrors "
    "the contract with G2107_WallSet[C05_WALL_D2R2 + 2], C708_ZONE_WALL_D2R2, "
    "MEDIA709 C06_WALL_D2L2, and G2035[C10_VIEW_SQUARE_D2R2]. "
    "DUNVIEW.C:8503-8508 F0128 draws D2L2 before D2R2 at depth 2 lateral "
    "-2/+2. DUNVIEW.C:3113-3129 F0104 and 3185-3204 F0105 preserve "
    "C10_COLOR_FLESH transparency. DEFS.H:2605-2606 defines C09/C10 view "
    "squares, DEFS.H:3428-3429 defines C05/C06 walls, DEFS.H:4047-4048 "
    "defines C707/C708 zones, and DEFS.H:2088 defines C10_COLOR_FLESH. "
    "CSB-lineage Viewport.cpp:1813-1820 StdDrawF3L1DoorFacing binds command "
    "60200 to the frame bitmap and 60250 to the frame rect; Viewport.cpp:"
    "2267/2271 and 3379/3390 bind the D2L2/D2R2 wall pair to pWallBitmaps "
    "5/6 and wall rectangles 13/12.";

static const CSB_V1_ViewportD2L2D2R2WallRouteSpec s_routes[] = {
    {
        CSB_ROUTE_PRESENT,
        CSB_D2L2_VIEW_SQUARE,
        8,
        CSB_D2_VIEW_DEPTH,
        CSB_D2L2_VIEW_LATERAL,
        CSB_D2L2_WALL_ZONE,
        CSB_ELEMENT_WALL,
        CSB_ELEMENT_TELEPORTER,
        CSB_WALL_D2L2,
        CSB_PC34_WALL_DELTA,
        CSB_WALL_D2L2 + CSB_PC34_WALL_DELTA,
        CSB_WALL_D2R2,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_TRANSPARENT_COLOR,
        CSB_ROUTE_PRESENT,
        CSB_LINEAGE_FRAME_BLIT_COMMAND,
        CSB_LINEAGE_FRAME_RECT_COMMAND,
        CSB_LINEAGE_D2L2_PWALLBITMAP,
        CSB_LINEAGE_D2L2_WALLRECT,
        CSB_D2L2_FIELD_ASPECT,
        "D2L2 wall route",
        "ReDMCSB DUNVIEW.C:6837-6865 F0678_DrawD2L2",
        "CSB-lineage Viewport.cpp:1813-1820,2267,3379",
        s_source_evidence
    },
    {
        CSB_ROUTE_PRESENT,
        CSB_D2R2_VIEW_SQUARE,
        9,
        CSB_D2_VIEW_DEPTH,
        CSB_D2R2_VIEW_LATERAL,
        CSB_D2R2_WALL_ZONE,
        CSB_ELEMENT_WALL,
        CSB_ELEMENT_TELEPORTER,
        CSB_WALL_D2R2,
        CSB_PC34_WALL_DELTA,
        CSB_WALL_D2R2 + CSB_PC34_WALL_DELTA,
        CSB_WALL_D2L2,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_TRANSPARENT_COLOR,
        CSB_ROUTE_PRESENT,
        CSB_LINEAGE_FRAME_BLIT_COMMAND,
        CSB_LINEAGE_FRAME_RECT_COMMAND,
        CSB_LINEAGE_D2R2_PWALLBITMAP,
        CSB_LINEAGE_D2R2_WALLRECT,
        CSB_D2R2_FIELD_ASPECT,
        "D2R2 wall route",
        "ReDMCSB DUNVIEW.C:6868-6896 F0679_DrawD2R2",
        "CSB-lineage Viewport.cpp:1813-1820,2271,3390",
        s_source_evidence
    }
};

size_t csb_v1_viewport_d2l2_d2r2_wall_route_spec_count_pc34(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_ViewportD2L2D2R2WallRouteSpec *
csb_v1_viewport_d2l2_d2r2_wall_route_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l2_d2r2_wall_route_spec_count_pc34()) return NULL;
    return &s_routes[index];
}

const CSB_V1_ViewportD2L2D2R2WallRouteSpec *
csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d2l2_d2r2_wall_route_spec_count_pc34(); ++i) {
        if (s_routes[i].view_square == view_square) return &s_routes[i];
    }
    return NULL;
}

int csb_v1_viewport_d2l2_d2r2_wall_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height,
    int flip_horizontal)
{
    int copied = 0;

    if (!spec || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;

    /* ReDMCSB: DUNVIEW.C F0104 lines 3113-3129 and F0105 lines 3185-3204
     * route wall pixels through C10_COLOR_FLESH transparency. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int source_x = flip_horizontal ? width - 1 - x : x;
            const uint8_t pixel = source[(y * source_stride) + source_x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

int csb_v1_viewport_d2l2_d2r2_wall_pc34_compat_run(
    CSB_V1_ViewportD2L2D2R2WallRunResult *out_result)
{
    uint8_t left_source[8] = { 1, 10, 2, 3, 4, 10, 5, 6 };
    uint8_t right_source[8] = { 6, 5, 10, 4, 3, 2, 10, 1 };
    uint8_t left_destination[8] = { 77, 77, 77, 77, 77, 77, 77, 77 };
    uint8_t right_destination[8] = { 77, 77, 77, 77, 77, 77, 77, 77 };
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(CSB_D2L2_VIEW_SQUARE);
    const CSB_V1_ViewportD2L2D2R2WallRouteSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_wall_route_spec_for_square_pc34(CSB_D2R2_VIEW_SQUARE);
    int left_copied;
    int right_copied;
    CSB_V1_ViewportD2L2D2R2WallRunResult result;

    if (!d2l2 || !d2r2) return -1;

    left_copied = csb_v1_viewport_d2l2_d2r2_wall_apply_c10_blit_pc34(
        d2l2, left_source, 4, left_destination, 4, 4, 2, 0);
    right_copied = csb_v1_viewport_d2l2_d2r2_wall_apply_c10_blit_pc34(
        d2r2, right_source, 4, right_destination, 4, 4, 2, 1);

    result.route_count = (int)csb_v1_viewport_d2l2_d2r2_wall_route_spec_count_pc34();
    result.wall_zone_draw_order_ok =
        d2l2->f0128_draw_order_index == 8 &&
        d2r2->f0128_draw_order_index == 9 &&
        d2l2->wall_zone == CSB_D2L2_WALL_ZONE &&
        d2r2->wall_zone == CSB_D2R2_WALL_ZONE;
    result.palette_indices_ok =
        d2l2->transparent_color == CSB_TRANSPARENT_COLOR &&
        d2r2->transparent_color == CSB_TRANSPARENT_COLOR &&
        left_copied == 6 &&
        right_copied == 6 &&
        left_destination[1] == 77 &&
        right_destination[1] == 77;
    result.lineage_frame_bindings_ok =
        d2l2->frame_blit_command_60200 == CSB_LINEAGE_FRAME_BLIT_COMMAND &&
        d2r2->frame_blit_command_60200 == CSB_LINEAGE_FRAME_BLIT_COMMAND &&
        d2l2->frame_rect_command_60250 == CSB_LINEAGE_FRAME_RECT_COMMAND &&
        d2r2->frame_rect_command_60250 == CSB_LINEAGE_FRAME_RECT_COMMAND;
    result.symmetry_ok =
        d2l2->view_square + 1 == d2r2->view_square &&
        d2l2->f0128_relative_depth == d2r2->f0128_relative_depth &&
        d2l2->f0128_relative_lateral == -d2r2->f0128_relative_lateral &&
        d2l2->wall_zone + 1 == d2r2->wall_zone &&
        d2l2->native_wall_index_base == d2r2->media709_flipped_wall_index &&
        d2r2->native_wall_index_base == d2l2->media709_flipped_wall_index &&
        d2l2->csb_viewport_wall_bitmap_index + 1 == d2r2->csb_viewport_wall_bitmap_index &&
        d2l2->csb_viewport_wall_rectangle_index - 1 == d2r2->csb_viewport_wall_rectangle_index;
    result.d2l2_copied_pixels = left_copied;
    result.d2r2_copied_pixels = right_copied;
    result.ok = result.route_count == 2 &&
                result.wall_zone_draw_order_ok &&
                result.palette_indices_ok &&
                result.lineage_frame_bindings_ok &&
                result.symmetry_ok;

    if (out_result) *out_result = result;
    return result.ok ? 0 : 1;
}

const char *csb_v1_viewport_d2l2_d2r2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
