#include "csb_v1_viewport_f0109_door_ornament_pc34_compat.h"
#include "dm1_v1_door_ornament_render_pc34_compat.h"

/*
 * CSB viewport bridge for ReDMCSB DUNVIEW.C:4013-4117
 * F0109_DUNGEONVIEW_DrawDoorOrnament.
 *
 * Wraps the DM1 door ornament plan infrastructure for use in the CSB
 * viewport pipeline.  The underlying ornament coordinate sets (G0207),
 * palette remaps (G0200/G0201), and transparency color (C09=9) are
 * identical between DM1 and CSB PC 3.4.
 */

enum {
    /* ReDMCSB DEFS.H C09_COLOR_GOLD. */
    CSB_C09_COLOR_GOLD = 9,
    /* View-door-ornament-index mapping: C0=D3LCR, C1=D2LCR, C2=D1LCR.
     * depthIndex in dm1_v1_door_ornament_render_plan_pc34 uses 0=D1, 1=D2, 2=D3. */
    CSB_VIEW_INDEX_D3LCR = 0,
    CSB_VIEW_INDEX_D2LCR = 1,
    CSB_VIEW_INDEX_D1LCR = 2,
    /* Palette remap sentinel: -1 = no remap (D1 uses native bitmap directly). */
    CSB_PALETTE_REMAP_NONE = -1,
    /* ReDMCSB DUNVIEW.C:4071 G0200_auc_Graphic558_DoorOrnamentPaletteD3. */
    CSB_PALETTE_REMAP_D3 = 200,
    /* ReDMCSB DUNVIEW.C:4083 G0201_auc_Graphic558_DoorOrnamentPaletteD2. */
    CSB_PALETTE_REMAP_D2 = 201,
    /* Native bitmap size at D1 (48x88). */
    CSB_D1_NATIVE_WIDTH = 48,
    CSB_D1_NATIVE_HEIGHT = 88,
    CSB_COORD_SET_COUNT = 4,
    CSB_VIEW_INDEX_COUNT = 3
};

/* ReDMCSB DUNVIEW.C G0207_aaauc_Graphic558_DoorOrnamentCoordinateSets.
 * Indexed [coordSet][viewIndex] where viewIndex 0=D3, 1=D2, 2=D1.
 * Each row: relX, relX2, relY, relY2, byteWidth, height.
 * Duplicated from DM1 for CSB-local coordinate lookup. */
static const unsigned char s_coordSets[4][3][6] = {
    {{17,31, 8,17, 8,10}, {22,42,11,23,16,13}, {32,63,13,31,16,19}},
    {{ 0,47, 0,40,24,41}, { 0,63, 0,60,32,61}, { 0,95, 0,87,48,88}},
    {{17,31,15,24, 8,10}, {22,42,22,34,16,13}, {32,63,31,49,16,19}},
    {{23,35,31,39, 8, 9}, {30,48,41,52,16,12}, {44,75,61,79,16,19}}
};

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C:4013-4117 F0109_DUNGEONVIEW_DrawDoorOrnament. "
    "Ordinal is 1-based; decremented to index G0103_as_CurrentMapDoorOrnamentsInfo[] "
    "which gives NativeBitmapIndex and CoordinateSet. "
    "D1 (view_index C2): native bitmap 48x88, no scaling, no palette remap. "
    "D2 (view_index C1): F0129 scales, palette G0201 (DUNVIEW.C:4083). "
    "D3 (view_index C0): F0129 scales, palette G0200 (DUNVIEW.C:4071). "
    "Transparency: C09_COLOR_GOLD (9). "
    "G0207 coordinate sets at DUNVIEW.C:4044-4068.";

/* Convert CSB view_door_ornament_index (C0=D3, C1=D2, C2=D1) to
 * DM1 depthIndex (0=D1, 1=D2, 2=D3). */
static int view_index_to_depth(int view_index)
{
    switch (view_index) {
    case CSB_VIEW_INDEX_D3LCR: return 2; /* D3 */
    case CSB_VIEW_INDEX_D2LCR: return 1; /* D2 */
    case CSB_VIEW_INDEX_D1LCR: return 0; /* D1 */
    default: return -1;
    }
}

bool csb_v1_viewport_f0109_door_ornament_plan_pc34(
    int ordinal,
    int view_door_ornament_index,
    int cache_loaded,
    const int local_to_global[16],
    CSB_V1_ViewportDoorOrnamentPlanPc34 *out_plan)
{
    CSB_V1_ViewportDoorOrnamentPlanPc34 plan = {0};
    DM1_DoorOrnamentInfoPc34 info;
    int depth;
    const unsigned char *coord;

    plan.source_evidence = s_source_evidence;

    if (!out_plan) {
        return false;
    }

    /* Ordinal 0 means no ornament. */
    if (ordinal <= 0) {
        plan.accepted = false;
        *out_plan = plan;
        return false;
    }

    depth = view_index_to_depth(view_door_ornament_index);
    if (depth < 0) {
        plan.accepted = false;
        *out_plan = plan;
        return false;
    }

    /* Resolve ordinal through the DM1 ornament cache. */
    if (!dm1_v1_door_ornament_info_for_ordinal_pc34(
            ordinal, cache_loaded, local_to_global, &info)) {
        plan.accepted = false;
        *out_plan = plan;
        return false;
    }

    plan.ordinal = ordinal;
    plan.view_index = view_door_ornament_index;
    plan.depth = depth;
    plan.coord_set = info.coordSet;
    plan.native_bitmap_index = info.graphicIndex;
    plan.transparent_color = CSB_C09_COLOR_GOLD;

    /* D1 uses native bitmap directly; D2/D3 need scaling. */
    if (depth == 0) {
        /* D1: no scaling, no palette remap. */
        plan.needs_scaling = false;
        plan.scale_width = CSB_D1_NATIVE_WIDTH;
        plan.scale_height = CSB_D1_NATIVE_HEIGHT;
        plan.palette_remap_index = CSB_PALETTE_REMAP_NONE;
    } else {
        /* D2 or D3: look up coordinate set for scaled dimensions. */
        int vi = depth == 2 ? 0 : 1; /* viewIndex in G0207: 0=D3, 1=D2 */
        if (info.coordSet < 0 || info.coordSet >= CSB_COORD_SET_COUNT) {
            plan.accepted = false;
            *out_plan = plan;
            return false;
        }
        coord = s_coordSets[info.coordSet][vi];
        plan.needs_scaling = true;
        plan.scale_width = coord[4]; /* byteWidth */
        plan.scale_height = coord[5]; /* height */
        plan.palette_remap_index = (depth == 2)
            ? CSB_PALETTE_REMAP_D3
            : CSB_PALETTE_REMAP_D2;
    }

    plan.accepted = true;
    *out_plan = plan;
    return true;
}

bool csb_v1_viewport_f0109_door_ornament_coords_pc34(
    int coord_set,
    int view_index,
    int *out_x,
    int *out_y,
    int *out_w,
    int *out_h)
{
    const unsigned char *coord;

    if (coord_set < 0 || coord_set >= CSB_COORD_SET_COUNT ||
        view_index < 0 || view_index >= CSB_VIEW_INDEX_COUNT ||
        !out_x || !out_y || !out_w || !out_h) {
        return false;
    }

    coord = s_coordSets[coord_set][view_index];
    *out_x = coord[0];
    *out_y = coord[2];
    *out_w = coord[1] - coord[0] + 1;
    *out_h = coord[3] - coord[2] + 1;
    return true;
}

const char *csb_v1_viewport_f0109_door_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}
