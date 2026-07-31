#include "csb/csb_v1_viewport_d1l2_d1r2_wall_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D1L2 = 1,
    CSB_SIDE_D1R2 = 2,
    CSB_ELEMENT_WALL = 0,             /* ReDMCSB DEFS.H:1007 C00_ELEMENT_WALL. */
    CSB_ELEMENT_TELEPORTER = 5,       /* ReDMCSB DEFS.H:1012 C05_ELEMENT_TELEPORTER. */
    CSB_VIEW_SQUARE_D1C = 3,          /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C. */
    CSB_VIEW_SQUARE_D1L = 4,          /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L. */
    CSB_VIEW_SQUARE_D1R = 5,          /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R. */
    CSB_VIEW_DEPTH_D1 = 1,            /* ReDMCSB DUNVIEW.C:8524/8528 F0128. */
    CSB_VIEW_LANE_D1L = -1,           /* ReDMCSB DUNVIEW.C:8524 F0128. */
    CSB_VIEW_LANE_D1R = 1,            /* ReDMCSB DUNVIEW.C:8528 F0128. */
    CSB_WALL_D1R = 2,                 /* ReDMCSB DEFS.H:3425 C02_WALL_D1R. */
    CSB_WALL_D1L = 3,                 /* ReDMCSB DEFS.H:3426 C03_WALL_D1L. */
    CSB_ZONE_WALL_D1C = 712,          /* ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C. */
    CSB_ZONE_WALL_D1L = 713,          /* ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L. */
    CSB_ZONE_WALL_D1R = 714,          /* ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R. */
    CSB_C10_COLOR_FLESH = 10,         /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_VIEWPORT_WIDTH = 224,
    CSB_VIEWPORT_HEIGHT = 136,
    CSB_D1_SOURCE_WIDTH = 256,
    CSB_D1_SOURCE_HEIGHT = 111,
    CSB_D1_CLIP_WIDTH = 64,
    CSB_D1_CLIP_HEIGHT = 111,
    CSB_D1L_FRAME_ROW = 7,            /* ReDMCSB DUNVIEW.C:590 G0163 D1L row. */
    CSB_D1R_FRAME_ROW = 8,            /* ReDMCSB DUNVIEW.C:591 G0163 D1R row. */
    CSB_ZONE_MATH_BASE = 709          /* 709 + M607/M608 gives C713/C714. */
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate; no real-asset bitmap parity and no CSB "
    "game-data load. ReDMCSB DUNVIEW.C:7391-7557 "
    "F0122_DUNGEONVIEW_DrawSquareD1L and 7559-7725 "
    "F0123_DUNGEONVIEW_DrawSquareD1R both read the square aspect through "
    "DUNGEON.C:2466-2523 F0172_DUNGEON_SetSquareAspect. Their "
    "C00_ELEMENT_WALL cases at DUNVIEW.C:7436-7460 and 7604-7628 route "
    "through the CSB/I34 wall bitmaps G2107_WallSet[C03_WALL_D1L] or "
    "G2107_WallSet[C02_WALL_D1R], with the opposite side selected by "
    "F0105 when G0076_B_UseFlippedWallAndFootprintsBitmaps is true. "
    "DUNVIEW.C:3113-3156 F0104 and 3185-3218 F0105 blit with "
    "C10_COLOR_FLESH transparency from DEFS.H:2088. The wall cases return "
    "before F0111 and F0115; DUNVIEW.C:4547-4581 and 5668-5671 anchor the "
    "thing-pass keep-out, while DUNGEON.C:1769-1840 F0163 and 1840-1905 "
    "F0164 anchor thing-list mutation routines that this wall route must "
    "not call. DUNVIEW.C:8524-8529 F0128 draws D1L at depth 1 lateral -1 "
    "then D1R at depth 1 lateral +1, resets map coordinates between the "
    "sides, and DUNVIEW.C:8532-8533 follows with D1C. DUNVIEW.C:8294 "
    "anchors the later F0127 object-pass boundary. DEFS.H:2599-2601 binds "
    "M606/M607/M608, DEFS.H:3425-3426 binds C02/C03 walls, DEFS.H:4052-4054 "
    "binds C712/C713/C714 zones, DEFS.H:2445-2452 and 4147-4162 bind the "
    "D1 stairs ordinals/zones, and DEFS.H:4205-4207 binds D1 pit zones.";

static const CSB_V1_D1L2D1R2WallRouteSpecPc34 s_routes[] = {
    {
        CSB_SIDE_D1L2,
        "D1L2 wall route via F0122",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        122,
        0,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_D1L,
        CSB_VIEW_SQUARE_D1L,
        CSB_ELEMENT_WALL,
        CSB_ELEMENT_TELEPORTER,
        CSB_ZONE_WALL_D1L,
        CSB_ZONE_WALL_D1C,
        CSB_WALL_D1L,
        CSB_WALL_D1R,
        CSB_D1L_FRAME_ROW,
        0,
        63,
        9,
        119,
        128,
        CSB_D1_CLIP_HEIGHT,
        192,
        0,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_ZONE_MATH_BASE,
        CSB_VIEW_SQUARE_D1C,
        "G0700_puc_Bitmap_WallSet_Wall_D1LCR / G2107_WallSet[C03_WALL_D1L]",
        "G0163_aauc_Graphic558_Frame_Walls D1L row {0,63,9,119,128,111,192,0}",
        "ReDMCSB DUNVIEW.C:7391-7557 F0122 and 8524-8525 F0128",
        s_source_evidence
    },
    {
        CSB_SIDE_D1R2,
        "D1R2 wall route via F0123",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        123,
        1,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_D1R,
        CSB_VIEW_SQUARE_D1R,
        CSB_ELEMENT_WALL,
        CSB_ELEMENT_TELEPORTER,
        CSB_ZONE_WALL_D1R,
        CSB_ZONE_WALL_D1C,
        CSB_WALL_D1R,
        CSB_WALL_D1L,
        CSB_D1R_FRAME_ROW,
        160,
        223,
        9,
        119,
        128,
        CSB_D1_CLIP_HEIGHT,
        0,
        0,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_ZONE_MATH_BASE,
        CSB_VIEW_SQUARE_D1C,
        "G0700_puc_Bitmap_WallSet_Wall_D1LCR / G2107_WallSet[C02_WALL_D1R]",
        "G0163_aauc_Graphic558_Frame_Walls D1R row {160,223,9,119,128,111,0,0}",
        "ReDMCSB DUNVIEW.C:7559-7725 F0123 and 8528-8529 F0128",
        s_source_evidence
    }
};

size_t csb_v1_viewport_d1l2_d1r2_wall_route_spec_count_pc34(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_D1L2D1R2WallRouteSpecPc34 *
csb_v1_viewport_d1l2_d1r2_wall_route_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1l2_d1r2_wall_route_spec_count_pc34()) {
        return 0;
    }
    return &s_routes[index];
}

const CSB_V1_D1L2D1R2WallRouteSpecPc34 *
csb_v1_viewport_d1l2_d1r2_wall_route_spec_for_side_pc34(int side)
{
    for (size_t i = 0; i < csb_v1_viewport_d1l2_d1r2_wall_route_spec_count_pc34(); ++i) {
        if (s_routes[i].side == side) return &s_routes[i];
    }
    return 0;
}

int csb_v1_viewport_d1l2_d1r2_wall_expected_zone_for_view_square_pc34(
    int view_square)
{
    if (view_square != CSB_VIEW_SQUARE_D1L && view_square != CSB_VIEW_SQUARE_D1R) {
        return -1;
    }
    return CSB_ZONE_MATH_BASE + view_square;
}

int csb_v1_viewport_d1l2_d1r2_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *spec,
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
    if (local_x < 0 || local_x >= CSB_D1_CLIP_WIDTH) return -1;
    *out_source_x = spec->wall_frame_source_x +
                    (flipped_variant ? (CSB_D1_CLIP_WIDTH - 1 - local_x) : local_x);
    return 0;
}

uint16_t csb_v1_viewport_d1l2_d1r2_wall_preserve_first_thing_pc34(
    const CSB_V1_D1L2D1R2WallRouteSpecPc34 *spec,
    uint16_t first_thing)
{
    /* ReDMCSB: DUNVIEW.C F0122 lines 7436-7460 and F0123 lines 7604-7628
     * return from WALL cases before F0115, so DUNGEON.C F0163/F0164
     * thing-list mutation contracts are not part of this path. */
    if (!spec || !spec->f0115_thing_pass_keepout) return 0xffffu;
    return first_thing;
}

const char *csb_v1_viewport_d1l2_d1r2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
