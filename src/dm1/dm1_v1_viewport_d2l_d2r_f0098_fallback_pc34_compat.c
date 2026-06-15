#include "dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.h"

enum {
    DM1_V1_D2L_D2R_F0098_VIEW_SQUARE_D2L_PC34 = 4,  /* ReDMCSB: DEFS.H line 2582 M604_VIEW_SQUARE_D2L */
    DM1_V1_D2L_D2R_F0098_VIEW_SQUARE_D2R_PC34 = 5,  /* ReDMCSB: DEFS.H line 2583 M605_VIEW_SQUARE_D2R */
    DM1_V1_D2L_D2R_F0098_VIEW_FLOOR_D2L_PC34 = 5,   /* ReDMCSB: DEFS.H line 2755 M591_VIEW_FLOOR_D2L */
    DM1_V1_D2L_D2R_F0098_VIEW_FLOOR_D2R_PC34 = 7,   /* ReDMCSB: DEFS.H line 2757 M593_VIEW_FLOOR_D2R */
    DM1_V1_D2L_D2R_F0098_ZONE_CEILING_PC34 = 700,   /* ReDMCSB: DEFS.H line 4040 C700_ZONE_VIEWPORT_CEILING_AREA */
    DM1_V1_D2L_D2R_F0098_ZONE_FLOOR_PC34 = 701,     /* ReDMCSB: DEFS.H line 4041 C701_ZONE_VIEWPORT_FLOOR_AREA */
    DM1_V1_D2L_D2R_F0098_ZONE_WALL_D2L_PC34 = 710,  /* ReDMCSB: DEFS.H line 4050 C710_ZONE_WALL_D2L */
    DM1_V1_D2L_D2R_F0098_ZONE_WALL_D2R_PC34 = 711,  /* ReDMCSB: DEFS.H line 4051 C711_ZONE_WALL_D2R */
    DM1_V1_D2L_D2R_F0098_ZONE_FLOOR_ORNAMENT_PC34 = 1500, /* ReDMCSB: DEFS.H line 4223 C1500_ZONE_FLOOR_ORNAMENT */
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_OPEN_PC34 = 0x3421, /* ReDMCSB: DEFS.H line 2676 */
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_OPEN_PC34 = 0x4312, /* ReDMCSB: DEFS.H line 2677 */
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_DOOR_SIDE_PC34 = 0x0342, /* ReDMCSB: DEFS.H line 2671 */
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_DOOR_SIDE_PC34 = 0x0431, /* ReDMCSB: DEFS.H line 2674 */
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_DOOR_PASS1_PC34 = 0x0218,
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_DOOR_PASS1_PC34 = 0x0128,
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_DOOR_PASS2_PC34 = 0x0349,
    DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_DOOR_PASS2_PC34 = 0x0439
};

#define PASS650_GRAPHICS_SHA256 "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"

/*
 * ReDMCSB source-lock anchors:
 * - DUNVIEW.C F0098 line 2962 owns the floor/ceiling refresh and clears
 *   G0297_B_DrawFloorAndCeilingRequested at line 3002.
 * - DUNVIEW.C F0128 lines 8337-8338 call F0098 only when
 *   G0297_B_DrawFloorAndCeilingRequested is true; lines 8357-8431 then
 *   perform floor/ceiling flip handling including F0099, lines 8512-8517
 *   dispatch D2L then D2R, and lines 8606-8610 present with F0097.
 * - DUNVIEW.C F0119 lines 6900-7049 and F0120 lines 7051-7220 bind the
 *   D2L/D2R non-wall body: C00 wall returns through F0107, while
 *   corridor/pit/teleporter/stairs/door-side/door-front reach F0108,
 *   F0112, F0115, and optional F0113 without using the wall-return path.
 * - DEFS.H lines 2582-2583, 2755-2757, 4040-4051, 4144-4213, and 4223
 *   bind view-square, floor-view, floor/ceiling, D2L/D2R wall, pit,
 *   stairs, ceiling-pit, and floor-ornament zone ids.
 */

static DM1_V1_D2LD2RF0098FallbackSpecPc34 s_specs[2] = {
    {
        DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34,
        2,
        -1,
        DM1_V1_D2L_D2R_F0098_VIEW_SQUARE_D2L_PC34,
        DM1_V1_D2L_D2R_F0098_VIEW_FLOOR_D2L_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_WALL_D2L_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_CEILING_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_FLOOR_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_FLOOR_ORNAMENT_PC34,
        855,
        864,
        805,
        818,
        826,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_OPEN_PC34,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_DOOR_SIDE_PC34,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_DOOR_PASS1_PC34,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2L_DOOR_PASS2_PC34,
        10,
        20,
        100,
        900,
        true,
        true,
        true,
        true,
        true,
        "M604_VIEW_SQUARE_D2L",
        "M591_VIEW_FLOOR_D2L",
        "C710_ZONE_WALL_D2L",
        PASS650_GRAPHICS_SHA256,
        "G2108_Floor",
        "G2109_Ceiling",
        NULL
    },
    {
        DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34,
        2,
        1,
        DM1_V1_D2L_D2R_F0098_VIEW_SQUARE_D2R_PC34,
        DM1_V1_D2L_D2R_F0098_VIEW_FLOOR_D2R_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_WALL_D2R_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_CEILING_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_FLOOR_PC34,
        DM1_V1_D2L_D2R_F0098_ZONE_FLOOR_ORNAMENT_PC34,
        857,
        866,
        807,
        820,
        827,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_OPEN_PC34,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_DOOR_SIDE_PC34,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_DOOR_PASS1_PC34,
        DM1_V1_D2L_D2R_F0098_CELL_ORDER_D2R_DOOR_PASS2_PC34,
        10,
        20,
        110,
        900,
        true,
        true,
        true,
        true,
        true,
        "M605_VIEW_SQUARE_D2R",
        "M593_VIEW_FLOOR_D2R",
        "C711_ZONE_WALL_D2R",
        PASS650_GRAPHICS_SHA256,
        "G2108_Floor",
        "G2109_Ceiling",
        NULL
    }
};

static const DM1_V1_D2LD2RF0098OrderStepPc34 s_d2l_open_order[] = {
    { DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34, 10,
      "F0098_DUNGEONVIEW_DrawFloorAndCeiling", "DUNVIEW.C:8337-8338", "G0297_B_DrawFloorAndCeilingRequested" },
    { DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34, 20,
      "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal", "DUNVIEW.C:8363/8425", "party-side parity branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34, 100,
      "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:6900-7049", "C0_ELEMENT != C00_ELEMENT_WALL" },
    { DM1_V1_D2L_D2R_F0098_OP_F0108_FLOOR_ORNAMENT_PC34, 120,
      "F0108_DUNGEONVIEW_DrawFloorOrnament", "DUNVIEW.C:7020", "non-wall open branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0112_CEILING_PIT_PC34, 130,
      "F0112_DUNGEONVIEW_DrawCeilingPit", "DUNVIEW.C:7029", "non-wall open branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34, 140,
      "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "DUNVIEW.C:7031", "C0x3421 open-cell order" },
    { DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34, 900,
      "F0097_DUNGEONVIEW_DrawViewport", "DUNVIEW.C:8606-8610", "after all far-to-near square drawing" }
};

static const DM1_V1_D2LD2RF0098OrderStepPc34 s_d2r_open_order[] = {
    { DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34, 10,
      "F0098_DUNGEONVIEW_DrawFloorAndCeiling", "DUNVIEW.C:8337-8338", "G0297_B_DrawFloorAndCeilingRequested" },
    { DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34, 20,
      "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal", "DUNVIEW.C:8363/8425", "party-side parity branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34, 110,
      "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:7051-7220", "C0_ELEMENT != C00_ELEMENT_WALL" },
    { DM1_V1_D2L_D2R_F0098_OP_F0108_FLOOR_ORNAMENT_PC34, 120,
      "F0108_DUNGEONVIEW_DrawFloorOrnament", "DUNVIEW.C:7213", "non-wall open branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0112_CEILING_PIT_PC34, 130,
      "F0112_DUNGEONVIEW_DrawCeilingPit", "DUNVIEW.C:7221", "non-wall open branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34, 140,
      "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "DUNVIEW.C:7224", "C0x4312 open-cell order" },
    { DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34, 900,
      "F0097_DUNGEONVIEW_DrawViewport", "DUNVIEW.C:8606-8610", "after all far-to-near square drawing" }
};

static const DM1_V1_D2LD2RF0098OrderStepPc34 s_d2l_pit_order[] = {
    { DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34, 10,
      "F0098_DUNGEONVIEW_DrawFloorAndCeiling", "DUNVIEW.C:8337-8338", "G0297_B_DrawFloorAndCeilingRequested" },
    { DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34, 20,
      "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal", "DUNVIEW.C:8363/8425", "party-side parity branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34, 100,
      "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:6900-7049", "C02_ELEMENT_PIT, not wall_return" },
    { DM1_V1_D2L_D2R_F0098_OP_F0104_FLOOR_PIT_OR_STAIRS_PC34, 115,
      "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap", "DUNVIEW.C:7013", "C855_ZONE_FLOORPIT_D2L" },
    { DM1_V1_D2L_D2R_F0098_OP_F0108_FLOOR_ORNAMENT_PC34, 120,
      "F0108_DUNGEONVIEW_DrawFloorOrnament", "DUNVIEW.C:7020", "BUG0_64 open-pit overdraw path" },
    { DM1_V1_D2L_D2R_F0098_OP_F0112_CEILING_PIT_PC34, 130,
      "F0112_DUNGEONVIEW_DrawCeilingPit", "DUNVIEW.C:7029", "C864_ZONE_CEILING_PIT_D2L" },
    { DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34, 140,
      "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "DUNVIEW.C:7031", "C0x3421 open-cell order" },
    { DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34, 900,
      "F0097_DUNGEONVIEW_DrawViewport", "DUNVIEW.C:8606-8610", "after all far-to-near square drawing" }
};

static const DM1_V1_D2LD2RF0098OrderStepPc34 s_d2r_pit_order[] = {
    { DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34, 10,
      "F0098_DUNGEONVIEW_DrawFloorAndCeiling", "DUNVIEW.C:8337-8338", "G0297_B_DrawFloorAndCeilingRequested" },
    { DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34, 20,
      "F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal", "DUNVIEW.C:8363/8425", "party-side parity branch" },
    { DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34, 110,
      "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:7051-7220", "C02_ELEMENT_PIT, not wall_return" },
    { DM1_V1_D2L_D2R_F0098_OP_F0104_FLOOR_PIT_OR_STAIRS_PC34, 115,
      "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally", "DUNVIEW.C:7206", "C857_ZONE_FLOORPIT_D2R" },
    { DM1_V1_D2L_D2R_F0098_OP_F0108_FLOOR_ORNAMENT_PC34, 120,
      "F0108_DUNGEONVIEW_DrawFloorOrnament", "DUNVIEW.C:7213", "BUG0_64 open-pit overdraw path" },
    { DM1_V1_D2L_D2R_F0098_OP_F0112_CEILING_PIT_PC34, 130,
      "F0112_DUNGEONVIEW_DrawCeilingPit", "DUNVIEW.C:7221", "C866_ZONE_CEILING_PIT_D2R" },
    { DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34, 140,
      "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "DUNVIEW.C:7224", "C0x4312 open-cell order" },
    { DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34, 900,
      "F0097_DUNGEONVIEW_DrawViewport", "DUNVIEW.C:8606-8610", "after all far-to-near square drawing" }
};

const DM1_V1_D2LD2RF0098FallbackSpecPc34 *
dm1_v1_viewport_d2l_d2r_f0098_fallback_spec_pc34(
    DM1_V1_D2LD2RF0098SidePc34 side)
{
    if (side != DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34 &&
        side != DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34) {
        return NULL;
    }
    s_specs[(int)side].source_lines =
        dm1_v1_viewport_d2l_d2r_f0098_fallback_source_evidence_pc34();
    return &s_specs[(int)side];
}

const DM1_V1_D2LD2RF0098OrderStepPc34 *
dm1_v1_viewport_d2l_d2r_f0098_fallback_order_pc34(
    DM1_V1_D2LD2RF0098SidePc34 side,
    DM1_V1_D2LD2RF0098ElementPc34 element,
    size_t *count)
{
    if (count) *count = 0;
    if (element == DM1_V1_D2L_D2R_F0098_ELEMENT_PIT_PC34) {
        if (side == DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34) {
            if (count) *count = sizeof(s_d2l_pit_order) / sizeof(s_d2l_pit_order[0]);
            return s_d2l_pit_order;
        }
        if (side == DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34) {
            if (count) *count = sizeof(s_d2r_pit_order) / sizeof(s_d2r_pit_order[0]);
            return s_d2r_pit_order;
        }
        return NULL;
    }
    if (element == DM1_V1_D2L_D2R_F0098_ELEMENT_CORRIDOR_PC34 ||
        element == DM1_V1_D2L_D2R_F0098_ELEMENT_TELEPORTER_PC34 ||
        element == DM1_V1_D2L_D2R_F0098_ELEMENT_STAIRS_FRONT_PC34 ||
        element == DM1_V1_D2L_D2R_F0098_ELEMENT_STAIRS_SIDE_PC34 ||
        element == DM1_V1_D2L_D2R_F0098_ELEMENT_DOOR_SIDE_PC34 ||
        element == DM1_V1_D2L_D2R_F0098_ELEMENT_DOOR_FRONT_PC34) {
        if (side == DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34) {
            if (count) *count = sizeof(s_d2l_open_order) / sizeof(s_d2l_open_order[0]);
            return s_d2l_open_order;
        }
        if (side == DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34) {
            if (count) *count = sizeof(s_d2r_open_order) / sizeof(s_d2r_open_order[0]);
            return s_d2r_open_order;
        }
    }
    return NULL;
}

bool dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34(
    bool floor_ceiling_dirty_flag)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8338 guards F0098 with G0297. */
    return floor_ceiling_dirty_flag;
}

const char *dm1_v1_viewport_d2l_d2r_f0098_fallback_source_evidence_pc34(void)
{
    return
        "pass650 contract_only=1; the real GRAPHICS.DAT identity is "
        PASS650_GRAPHICS_SHA256 ". ReDMCSB DUNVIEW.C F0098:2962-3002 "
        "DrawFloorAndCeiling refreshes G2109_Ceiling/G2108_Floor into "
        "C700_ZONE_VIEWPORT_CEILING_AREA/C701_ZONE_VIEWPORT_FLOOR_AREA "
        "and clears G0297_B_DrawFloorAndCeilingRequested. DUNVIEW.C "
        "F0128:8337-8338 calls F0098 only when the dirty flag is set; "
        "F0128:8357-8431 performs F0099/floor-ceiling flip work after "
        "F0098; F0128:8512-8517 dispatches D2L then D2R; F0128:8606-8610 "
        "calls F0097 present after the square bodies. DUNVIEW.C "
        "F0119:6900-7049 D2L non-wall path reaches F0108/F0112/F0115 "
        "without the C00 wall_return/F0107 branch; DUNVIEW.C "
        "F0120:7051-7220 D2R mirrors that path. DEFS.H:2582-2583 "
        "M604/M605; DEFS.H:2755-2757 M591/M593; DEFS.H:4040-4051 "
        "C700/C701/C710/C711; DEFS.H:4144-4213 D2L/D2R stairs, floor-pit, "
        "and ceiling-pit zones; DEFS.H:4223 C1500 floor ornament zone.";
}
