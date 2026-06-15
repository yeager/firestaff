#include "csb_v1_viewport_d2l2_wall_pc34_compat.h"

enum {
    CSB_D2L2_VIEW_SQUARE = 9,                 /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2 */
    CSB_D2L2_SQUARE_ASPECT_ELEMENT_SLOT = 0,  /* ReDMCSB DEFS.H:2534 C0_ELEMENT */
    CSB_ELEMENT_WALL = 0,                     /* ReDMCSB DEFS.H:1007 C00_ELEMENT_WALL */
    CSB_ELEMENT_TELEPORTER = 5,               /* ReDMCSB DEFS.H:1012 C05_ELEMENT_TELEPORTER */
    CSB_D2L2_WALL_ZONE = 707,                 /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    CSB_D2L2_NATIVE_WALL_INDEX_BASE = 6,      /* ReDMCSB DEFS.H:3429 C06_WALL_D2L2 */
    CSB_D2L2_PC_FIX_CODE_SIZE_DELTA = 2,      /* ReDMCSB DUNVIEW.C:6854-6856 PC_FIX_CODE_SIZE */
    CSB_D2L2_NATIVE_WALL_INDEX_PC34 = 8,      /* ReDMCSB DUNVIEW.C:6853-6858 C06_WALL_D2L2 + 2 */
    CSB_D2R2_FLIPPED_WALL_INDEX = 5,          /* ReDMCSB DEFS.H:3428 C05_WALL_D2R2 */
    CSB_ROUTE_PRESENT = 1,                    /* Contract marker for source-locked route presence */
    CSB_ROUTE_ABSENT = 0,                     /* Contract marker for source-locked route absence */
    CSB_TRANSPARENT_COLOR = 10,               /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
    CSB_D2L2_FRAME_X1 = 0,                    /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_FRAME_X2 = 37,                   /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_FRAME_Y1 = 20,                   /* ReDMCSB COORD.C:1498 C707 top y=20 */
    CSB_D2L2_FRAME_Y2 = 90,                   /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_BYTE_WIDTH = 36,                 /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_HEIGHT = 71,                     /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_SOURCE_X = 30,                   /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_SOURCE_Y = 0,                    /* ReDMCSB DUNVIEW.C:6954-6964 projected D2L reference */
    CSB_D2L2_FIELD_ASPECT_INDEX = 5,          /* ReDMCSB DUNVIEW.C:377 G2035[C09_VIEW_SQUARE_D2L2] */
    CSB_D2L2_F0128_DRAW_ORDER_INDEX = 8,      /* ReDMCSB DUNVIEW.C:8500-8504 F0128 D2L2 dispatch */
    CSB_D2L2_F0128_RELATIVE_DEPTH = 2,        /* ReDMCSB DUNVIEW.C:8501 F0150 depth */
    CSB_D2L2_F0128_RELATIVE_LATERAL = -2,     /* ReDMCSB DUNVIEW.C:8501 F0150 lateral */
    CSB_PARITY_PAIR_PWALLBITMAP_LEFT = 5,     /* CSB Viewport.cpp:2267 pWallBitmaps[5] */
    CSB_PARITY_PAIR_PWALLBITMAP_RIGHT = 6     /* CSB Viewport.cpp:2271 pWallBitmaps[6] */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; not full real-asset wall bitmap parity. "
    "ReDMCSB DUNVIEW.C:6837-6872 F0678_DrawD2L2 switches on "
    "L2488_ai_SquareAspect[C0_ELEMENT]. Wall case routes to "
    "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2 + 2], "
    "C707_ZONE_WALL_D2L2) under PC_FIX_CODE_SIZE; MEDIA709 routes to "
    "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally("
    "G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2). "
    "Teleporter case routes to F0113_DUNGEONVIEW_DrawField("
    "G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex"
    "[C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2). "
    "DUNVIEW.C:3113-3129 F0104 preserves C10_COLOR_FLESH transparency; "
    "DUNVIEW.C:3185-3204 F0105 scratch flip preserves C10_COLOR_FLESH transparency; "
    "DUNVIEW.C:5920-5923 F0128 dispatcher contract; "
    "DUNVIEW.C:8500-8504 F0128 dispatches D2L2 at relative depth 2 lateral -2. "
    "DEFS.H:2605 C09_VIEW_SQUARE_D2L2=9; DEFS.H:3429 C06_WALL_D2L2=6; "
    "DEFS.H:4047 C707_ZONE_WALL_D2L2=707; DEFS.H:3428 C05_WALL_D2R2=5; "
    "DEFS.H:2088 C10_COLOR_FLESH=10; DEFS.H:1007 C00_ELEMENT_WALL=0; "
    "DEFS.H:1012 C05_ELEMENT_TELEPORTER=5; DEFS.H:2534 C0_ELEMENT=0. "
    "CSB Viewport.cpp:2267/2271 pWallBitmaps parity-pair evidence. "
    "D2L2 wall case returns before F0107 wall ornaments and has no F0111 door "
    "route and no F0115 thing pass.";

static const CSB_V1_ViewportD2L2WallRouteSpec s_d2l2_wall_route = {
    CSB_ROUTE_PRESENT,
    CSB_D2L2_VIEW_SQUARE,
    CSB_D2L2_SQUARE_ASPECT_ELEMENT_SLOT,
    CSB_ELEMENT_WALL,
    CSB_ELEMENT_TELEPORTER,
    CSB_D2L2_WALL_ZONE,
    CSB_D2L2_NATIVE_WALL_INDEX_BASE,
    CSB_D2L2_PC_FIX_CODE_SIZE_DELTA,
    CSB_D2L2_NATIVE_WALL_INDEX_PC34,
    CSB_D2R2_FLIPPED_WALL_INDEX,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_PRESENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_ABSENT,
    CSB_ROUTE_ABSENT,
    CSB_TRANSPARENT_COLOR,
    CSB_ROUTE_PRESENT,
    CSB_D2L2_FRAME_X1,
    CSB_D2L2_FRAME_X2,
    CSB_D2L2_FRAME_Y1,
    CSB_D2L2_FRAME_Y2,
    CSB_D2L2_BYTE_WIDTH,
    CSB_D2L2_HEIGHT,
    CSB_D2L2_SOURCE_X,
    CSB_D2L2_SOURCE_Y,
    CSB_D2L2_FIELD_ASPECT_INDEX,
    CSB_D2L2_F0128_DRAW_ORDER_INDEX,
    CSB_D2L2_F0128_RELATIVE_DEPTH,
    CSB_D2L2_F0128_RELATIVE_LATERAL,
    CSB_PARITY_PAIR_PWALLBITMAP_LEFT,
    CSB_PARITY_PAIR_PWALLBITMAP_RIGHT,
    "DUNVIEW.C F0678_DrawD2L2 / F0104 / F0105 / F0113 / F0128",
    s_source_evidence
};

const CSB_V1_ViewportD2L2WallRouteSpec *
csb_v1_viewport_d2l2_wall_route_spec_pc34(void)
{
    return &s_d2l2_wall_route;
}

const char *csb_v1_viewport_d2l2_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
