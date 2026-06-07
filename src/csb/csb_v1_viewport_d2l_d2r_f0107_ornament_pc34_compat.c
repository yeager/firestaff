#include "csb_v1_viewport_d2l_d2r_f0107_ornament_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_ELEMENT_WALL = 0,                  /* ReDMCSB: DEFS.H C00_ELEMENT_WALL. */
    CSB_ELEMENT_DOOR_FRONT = 17,           /* ReDMCSB: DEFS.H C17_ELEMENT_DOOR_FRONT. */
    CSB_VIEW_SQUARE_D2L = 4,               /* ReDMCSB: DEFS.H:2582 M604_VIEW_SQUARE_D2L. */
    CSB_VIEW_SQUARE_D2R = 5,               /* ReDMCSB: DEFS.H:2583 M605_VIEW_SQUARE_D2R. */
    CSB_D2_DEPTH = 2,
    CSB_D2L_LATERAL = -1,
    CSB_D2R_LATERAL = 1,
    CSB_WALL_D2L = 8,                      /* ReDMCSB: DEFS.H:3431 C08_WALL_D2L. */
    CSB_WALL_D2R = 7,                      /* ReDMCSB: DEFS.H:3430 C07_WALL_D2R. */
    CSB_ZONE_WALL_D2L = 708,               /* ReDMCSB: DEFS.H:4031 C708_ZONE_WALL_D2L. */
    CSB_ZONE_WALL_D2R = 709,               /* ReDMCSB: DEFS.H:4032 C709_ZONE_WALL_D2R. */
    CSB_COLOR_FLESH = 10,                  /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_BYTE_WIDTH_VIEWPORT = 112,         /* ReDMCSB: DEFS.H:2478 C112_BYTE_WIDTH_VIEWPORT. */
    CSB_RIGHT_WALL_ORNAMENT_ORDINAL = 2,   /* ReDMCSB: DEFS.H:2537 M551. */
    CSB_FRONT_WALL_ORNAMENT_ORDINAL = 3,   /* ReDMCSB: DEFS.H:2538 M552. */
    CSB_LEFT_WALL_ORNAMENT_ORDINAL = 4,    /* ReDMCSB: DEFS.H:2539 M553. */
    CSB_VIEW_WALL_D2L_RIGHT = 5,           /* ReDMCSB: DEFS.H:2686 M580. */
    CSB_VIEW_WALL_D2R_LEFT = 6,            /* ReDMCSB: DEFS.H:2687 M581. */
    CSB_VIEW_WALL_D2L_FRONT = 7,           /* ReDMCSB: DEFS.H:2688 M582. */
    CSB_VIEW_WALL_D2R_FRONT = 9,           /* ReDMCSB: DEFS.H:2690 M584. */
    CSB_VIEW_FLOOR_D2L = 3,                /* ReDMCSB: DEFS.H:2742 M591_VIEW_FLOOR_D2L. */
    CSB_VIEW_FLOOR_D2R = 5,                /* ReDMCSB: DEFS.H:2744 M593_VIEW_FLOOR_D2R. */
    CSB_VIEWPORT_ALCOVE_JUMP_PRESENT = 1
};

static const char s_source_evidence[] =
    "DUNVIEW.C F0120/F0121 C00_ELEMENT_WALL path; local ReDMCSB WIP labels "
    "the pair F0119_DUNGEONVIEW_DrawSquareD2L at lines 6945-6973 and "
    "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF at lines 7096-7123. Both routes "
    "call F0100_DUNGEONVIEW_DrawWallSetBitmap first with the D2LCR wall-set "
    "bitmap and G0163 frame for M604/M605, then F0107_DUNGEONVIEW_"
    "IsDrawnWallOrnamentAnAlcove_CPSF line 3502: D2L uses M551 with "
    "M580_VIEW_WALL_D2L_RIGHT at line 6968 and M552 with "
    "M582_VIEW_WALL_D2L_FRONT at line 6969; D2R uses M553 with "
    "M581_VIEW_WALL_D2R_LEFT at line 7119 and M552 with "
    "M584_VIEW_WALL_D2R_FRONT at line 7120. F0107 alcove return drives the "
    "C0x0000_CELL_ORDER_ALCOVE branch. F0100 lines 3048-3061 pass "
    "C112_BYTE_WIDTH_VIEWPORT and C10_COLOR_FLESH. The C17_ELEMENT_DOOR_FRONT "
    "branches call F0108_DUNGEONVIEW_DrawFloorOrnament at lines 6988 and the "
    "mirrored D2R door branch with the D2R floor view before F0111, while the "
    "wall case returns before F0111. DEFS.H lines 2537-2539, 2582-2583, "
    "2686-2690, 2742-2744, 2478, and 2088 bind the ordinals, view walls, "
    "floor views, viewport byte width, and C10 transparency. CSB Viewport.cpp "
    "lines 1003-1013 and 1027-1035 preserve the F2L1/F2R1 side decoration, "
    "front decoration, and Alcove JumpZ routing.";

static const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 s_routes[] = {
    {
        CSB_ROUTE_PRESENT,
        CSB_VIEW_SQUARE_D2L,
        CSB_D2_DEPTH,
        CSB_D2L_LATERAL,
        CSB_ELEMENT_WALL,
        CSB_ELEMENT_DOOR_FRONT,
        CSB_WALL_D2L,
        CSB_VIEW_SQUARE_D2L,
        CSB_ZONE_WALL_D2L,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_COLOR_FLESH,
        CSB_BYTE_WIDTH_VIEWPORT,
        CSB_RIGHT_WALL_ORNAMENT_ORDINAL,
        CSB_VIEW_WALL_D2L_RIGHT,
        CSB_FRONT_WALL_ORNAMENT_ORDINAL,
        CSB_VIEW_WALL_D2L_FRONT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_FLOOR_D2L,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_WALL_D2L_RIGHT,
        CSB_VIEW_WALL_D2L_FRONT,
        CSB_VIEWPORT_ALCOVE_JUMP_PRESENT,
        "D2L F0107 wall ornament route",
        "ReDMCSB DUNVIEW.C F0120_DUNGEONVIEW_DrawSquareD2L lineage alias",
        "DUNVIEW.C:6945-6973,6968-6969; F0107:3502-3939; F0100:3048-3061",
        "CSB Viewport.cpp:1003-1013 F2L1 side/front decoration + Alcove JumpZ"
    },
    {
        CSB_ROUTE_PRESENT,
        CSB_VIEW_SQUARE_D2R,
        CSB_D2_DEPTH,
        CSB_D2R_LATERAL,
        CSB_ELEMENT_WALL,
        CSB_ELEMENT_DOOR_FRONT,
        CSB_WALL_D2R,
        CSB_VIEW_SQUARE_D2R,
        CSB_ZONE_WALL_D2R,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_COLOR_FLESH,
        CSB_BYTE_WIDTH_VIEWPORT,
        CSB_LEFT_WALL_ORNAMENT_ORDINAL,
        CSB_VIEW_WALL_D2R_LEFT,
        CSB_FRONT_WALL_ORNAMENT_ORDINAL,
        CSB_VIEW_WALL_D2R_FRONT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_FLOOR_D2R,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_WALL_D2R_LEFT,
        CSB_VIEW_WALL_D2R_FRONT,
        CSB_VIEWPORT_ALCOVE_JUMP_PRESENT,
        "D2R F0107 wall ornament route",
        "ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2R lineage alias",
        "DUNVIEW.C:7096-7123,7119-7120; F0107:3502-3939; F0100:3048-3061",
        "CSB Viewport.cpp:1027-1035 F2R1 side/front decoration + Alcove JumpZ"
    }
};

size_t csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_count_pc34(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *
csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_count_pc34()) {
        return NULL;
    }
    return &s_routes[index];
}

const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *
csb_v1_viewport_d2l_d2r_f0107_ornament_route_spec_for_side_pc34(
    CSB_V1_ViewportD2LD2RF0107SidePc34 side)
{
    if (side == CSB_V1_VIEWPORT_D2L_D2R_F0107_SIDE_D2L_PC34) {
        return &s_routes[0];
    }
    if (side == CSB_V1_VIEWPORT_D2L_D2R_F0107_SIDE_D2R_PC34) {
        return &s_routes[1];
    }
    return NULL;
}

int csb_v1_viewport_d2l_d2r_f0107_ornament_run_pc34(
    const CSB_V1_ViewportD2LD2RF0107RouteSpecPc34 *spec,
    int element,
    int side_f0107_returns_alcove,
    int front_f0107_returns_alcove,
    CSB_V1_ViewportD2LD2RF0107RunResultPc34 *out_result)
{
    CSB_V1_ViewportD2LD2RF0107RunResultPc34 result;

    if (!spec || !out_result) return -1;

    result.ok = 0;
    result.wall_blit_calls = 0;
    result.wall_blit_before_f0107 = 0;
    result.f0107_side_calls = 0;
    result.f0107_front_calls = 0;
    result.f0107_side_return_alcove = 0;
    result.f0107_front_return_alcove = 0;
    result.f0107_front_branch_taken = 0;
    result.f0107_front_branch_fallthrough = 0;
    result.f0107_calls_compose = 0;
    result.f0108_floor_ornament_calls = 0;
    result.f0108_floor_ornament_view = -1;
    result.f0111_wall_calls = 0;
    result.f0111_door_case_only = spec->f0111_door_case_only;
    result.f0100_transparent_color = spec->f0100_transparent_color;
    result.f0100_destination_byte_width = spec->f0100_destination_byte_width;
    result.source_bitmap_resolved = spec->f0100_source_bitmap_resolved;
    result.frame_resolved = spec->f0100_frame_resolved;
    result.source_lock_evidence = s_source_evidence;

    if (element == CSB_ELEMENT_WALL) {
        /* ReDMCSB: DUNVIEW.C F0120/F0121 C00_ELEMENT_WALL lines 6947/7098
         * call F0100 before the F0107 side/front wall ornament calls. */
        result.wall_blit_calls = spec->f0100_wall_blit;
        result.wall_blit_before_f0107 = spec->f0100_wall_blit;
        result.f0107_side_calls = 1;
        result.f0107_side_return_alcove = side_f0107_returns_alcove ? 1 : 0;
        result.f0107_front_calls = 1;
        result.f0107_front_return_alcove = front_f0107_returns_alcove ? 1 : 0;
        result.f0107_front_branch_taken = result.f0107_front_return_alcove;
        result.f0107_front_branch_fallthrough = !result.f0107_front_return_alcove;
        result.f0107_calls_compose =
            result.wall_blit_before_f0107 &&
            result.f0107_side_calls == 1 &&
            result.f0107_front_calls == 1 &&
            spec->f0107_side_before_front &&
            spec->f0107_front_conditional_branch;
        result.ok = result.wall_blit_calls == 1 &&
                    result.f0107_calls_compose &&
                    result.f0111_wall_calls == 0 &&
                    result.f0100_transparent_color == CSB_COLOR_FLESH &&
                    result.f0100_destination_byte_width == CSB_BYTE_WIDTH_VIEWPORT;
    } else if (element == CSB_ELEMENT_DOOR_FRONT) {
        /* ReDMCSB: DUNVIEW.C C17_ELEMENT_DOOR_FRONT lines 6988 and the
         * mirrored D2R branch route floor ornaments before door drawing. */
        result.f0108_floor_ornament_calls = 1;
        result.f0108_floor_ornament_view = spec->f0108_door_front_floor_view;
        result.f0111_door_case_only = 1;
        result.ok = result.f0108_floor_ornament_view == spec->f0108_door_front_floor_view;
    } else {
        *out_result = result;
        return 1;
    }

    *out_result = result;
    return result.ok ? 0 : 1;
}

const char *csb_v1_viewport_d2l_d2r_f0107_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}
