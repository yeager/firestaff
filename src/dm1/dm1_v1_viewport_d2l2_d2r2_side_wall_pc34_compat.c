#include "dm1_v1_viewport_d2l2_d2r2_side_wall_pc34_compat.h"

#include <string.h>

enum {
    /* ReDMCSB: DEFS.H:2605-2606 (MEDIA720 / PC 3.4 path) */
    DM1_V1_C09_VIEW_SQUARE_D2L2_PC34 = 9,
    DM1_V1_C10_VIEW_SQUARE_D2R2_PC34 = 10,
    /* ReDMCSB: DEFS.H:3428-3429 */
    DM1_V1_C05_WALL_D2R2_PC34 = 5,
    DM1_V1_C06_WALL_D2L2_PC34 = 6,
    /* ReDMCSB: DEFS.H:4047-4048 */
    DM1_V1_C707_ZONE_WALL_D2L2_PC34 = 707,
    DM1_V1_C708_ZONE_WALL_D2R2_PC34 = 708,
    /* ReDMCSB: DEFS.H:2088 */
    DM1_V1_C10_COLOR_FLESH_PC34 = 10,
    /* Element switch values, ReDMCSB DEFS.H */
    DM1_V1_C00_ELEMENT_WALL_PC34 = 0,
    DM1_V1_C05_ELEMENT_TELEPORTER_PC34 = 5,
    /* ReDMCSB: DUNVIEW.C:6837-6865 F0678_DrawD2L2 */
    DM1_V1_F0678_NATIVE_WALL_DRAW_LINE_PC34 = 6854,
    DM1_V1_F0678_TELEPORTER_DRAW_LINE_PC34 = 6863,
    DM1_V1_F0678_SWITCH_END_LINE_PC34 = 6865,
    /* ReDMCSB: DUNVIEW.C:6868-6896 F0679_DrawD2R2 */
    DM1_V1_F0679_NATIVE_WALL_DRAW_LINE_PC34 = 6885,
    DM1_V1_F0679_TELEPORTER_DRAW_LINE_PC34 = 6894,
    DM1_V1_F0679_SWITCH_END_LINE_PC34 = 6896,
    /* ReDMCSB: DUNVIEW.C:8503-8508 F0128 caller */
    DM1_V1_F0128_F0678_CALL_LINE_PC34 = 8504,
    DM1_V1_F0128_F0679_CALL_LINE_PC34 = 8508
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C:6837-6865 F0678_DrawD2L2 (side-row depth=2, lateral=-2)
 *   only handles C00_ELEMENT_WALL (F0104 native, F0105 flipped, returns)
 *   and C05_ELEMENT_TELEPORTER (F0113 with C707). No F0107/F0108/F0111/
 *   F0115 in body; switch has no default; wall case returns before
 *   falling through to anything else.
 * - DUNVIEW.C:6868-6896 F0679_DrawD2R2 mirrors F0678 for the depth=2,
 *   lateral=+2 cell, using C05_WALL_D2R2 (native) and C06_WALL_D2L2
 *   (flipped) and the C708 zone.
 * - DUNVIEW.C:8503-8508 F0128 dispatches F0678 at relative (2,-2) and
 *   F0679 at relative (2,+2), AFTER F0118 (D3C at 3,0) and BEFORE
 *   F0119 (D2L at 2,-1). The F0678/F0679 row is the only side-row
 *   wall draw between F0118 (D3C center) and F0119 (D2L front-left).
 * - DUNVIEW.C:3113-3129 F0104 and 3185-3204 F0105 share the C10
 *   transparent blit contract used by the D2L2/D2R2 wall zones.
 * - DEFS.H:2088 C10_COLOR_FLESH; 2605-2606 C09/C10 view squares; 3428
 *   C05_WALL_D2R2; 3429 C06_WALL_D2L2; 4047-4048 C707/C708 zones.
 * - The PC 3.4 path (MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M
 *   _F31E_F31J_X31J_P31J and PC_FIX_CODE_SIZE) uses C06_WALL_D2L2+2
 *   and C05_WALL_D2R2+2 wall-set indices for the native F0104 route
 *   and EXCLUDES the G0076 flipped branch on PC 3.4.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C:6837-6865 F0678_DrawD2L2 is the "
    "side-row depth=2 lateral=-2 wall dispatcher. Its switch on "
    "L2488_ai_SquareAspect[C0_ELEMENT] has only two cases: "
    "C00_ELEMENT_WALL dispatches F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap "
    "with G2107_WallSet[C06_WALL_D2L2+2] (PC_FIX_CODE_SIZE) to "
    "C707_ZONE_WALL_D2L2 (line 6854) and returns; C05_ELEMENT_TELEPORTER "
    "dispatches F0113_DUNGEONVIEW_DrawField with C09_VIEW_SQUARE_D2L2's "
    "field aspect to C707_ZONE_WALL_D2L2 (line 6863). No default case, "
    "no break, no F0107/F0108/F0111/F0115, no stairs/pit branch, no "
    "alcove route, no corridor route, no door route, no F0124 dispatch. "
    "DUNVIEW.C:6868-6896 F0679_DrawD2R2 mirrors that contract for "
    "lateral=+2 with C05_WALL_D2R2+2 and C708_ZONE_WALL_D2R2 and "
    "C10_VIEW_SQUARE_D2R2 (line 6885 native, 6894 teleporter). "
    "DUNVIEW.C:8503-8508 F0128 dispatches F0678 at relative (2,-2) and "
    "F0679 at relative (2,+2) AFTER F0118 (D3C, line 8499) and BEFORE "
    "F0119 (D2L, line 8513); the side pair comes between D3 center and "
    "D2 front, never before D3 and never after D2. "
    "DUNVIEW.C:3113-3129 F0104 and 3185-3204 F0105 share the "
    "C10_COLOR_FLESH transparent blit. DEFS.H:2088 C10_COLOR_FLESH=10; "
    "DEFS.H:2605 C09_VIEW_SQUARE_D2L2=9; DEFS.H:2606 C10_VIEW_SQUARE_D2R2=10; "
    "DEFS.H:3428 C05_WALL_D2R2=5; DEFS.H:3429 C06_WALL_D2L2=6; "
    "DEFS.H:4047 C707_ZONE_WALL_D2L2=707; DEFS.H:4048 C708_ZONE_WALL_D2R2=708. "
    "The PC 3.4 native wall-set index uses PC_FIX_CODE_SIZE: +2 to skip "
    "the unused lower wall-set slot, never the G0076 flipped branch "
    "(G0076_B_UseFlippedWallAndFootprintsBitmaps is a global set by "
    "F0128 itself and is intentionally NOT consulted in F0678/F0679 "
    "on PC 3.4). F0678/F0679 are gated by MEDIA720_I34E_I34M_A36M_A31E"
    "_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J on PC 3.4 and are absent "
    "on older MEDIA009 / MEDIA508 / MEDIA142 / MEDIA544 / MEDIA746 sets.";

static const char s_disjointness_note[] =
    "D2L2/D2R2 side-wall F0128 caller + F0678/F0679 dispatcher "
    "source-lock contract only. Distinct from "
    "d2l2_d2r2_wall (F0104/F0105 wall-set blit pixel contract), "
    "d2l2_d2r2_f0107_wall_ornament (F0107 alcove/C1004 zone math), "
    "d2l2_d2r2_f0108_wall_composition (F0108 floor-ornament guard "
    "for the D2L/D2R front pair), "
    "d2l2_d2r2_f0111_door_front_pair (F0111 doors), "
    "d2l2_d2r2_f0115_thing_pass (F0115 thing pass), "
    "d2l2_d2r2_stairs_pit_dispatch (stairs/pit), "
    "d2l2_d2r2_f0108_floor_ornament (F0108 floor ornament pixel), "
    "d2l2_d2r2_f0108_floor_ceiling_ornament (F0108 two-pass), and "
    "the integrated D0L2/D0R2, D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, "
    "D3C, and CSB-lineage viewport gates by relative cells (2,-2) and "
    "(2,+2), wall zones C707/C708, view squares C09/C10, dispatchers "
    "F0678/F0679 (PC 3.4 MEDIA720 path), and the synthetic 8x8 + 8x8 "
    "side-row probe. Asset-free, reads no GRAPHICS.DAT, and makes no "
    "original DOS pixel parity claim.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static void fill_lanes(DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m)
{
    m->lanes[0] = (DM1_V1_D2L2D2R2SideWallLanePc34){
        DM1_V1_D2L2_D2R2_SIDE_WALL_SIDE_D2L2_PC34,
        "D2L2",
        DM1_V1_C09_VIEW_SQUARE_D2L2_PC34,
        "C09_VIEW_SQUARE_D2L2",
        DM1_V1_C707_ZONE_WALL_D2L2_PC34,
        "C707_ZONE_WALL_D2L2",
        DM1_V1_C06_WALL_D2L2_PC34,
        "C06_WALL_D2L2",
        DM1_V1_C05_WALL_D2R2_PC34,
        "C05_WALL_D2R2",
        DM1_V1_F0678_NATIVE_WALL_DRAW_LINE_PC34,
        DM1_V1_F0678_TELEPORTER_DRAW_LINE_PC34,
        DM1_V1_F0678_SWITCH_END_LINE_PC34,
        2,
        -2,
        DM1_V1_F0128_F0678_CALL_LINE_PC34,
        6,
        "DUNVIEW.C:8504 F0128 calls F0678 at (2,-2) (depth=2, lateral=-2) "
        "after F0118 D3C and before F0119 D2L; F0678 lines 6837-6865; "
        "F0104 line 6854; F0113 line 6863"
    };
    m->lanes[1] = (DM1_V1_D2L2D2R2SideWallLanePc34){
        DM1_V1_D2L2_D2R2_SIDE_WALL_SIDE_D2R2_PC34,
        "D2R2",
        DM1_V1_C10_VIEW_SQUARE_D2R2_PC34,
        "C10_VIEW_SQUARE_D2R2",
        DM1_V1_C708_ZONE_WALL_D2R2_PC34,
        "C708_ZONE_WALL_D2R2",
        DM1_V1_C05_WALL_D2R2_PC34,
        "C05_WALL_D2R2",
        DM1_V1_C06_WALL_D2L2_PC34,
        "C06_WALL_D2L2",
        DM1_V1_F0679_NATIVE_WALL_DRAW_LINE_PC34,
        DM1_V1_F0679_TELEPORTER_DRAW_LINE_PC34,
        DM1_V1_F0679_SWITCH_END_LINE_PC34,
        2,
        2,
        DM1_V1_F0128_F0679_CALL_LINE_PC34,
        7,
        "DUNVIEW.C:8508 F0128 calls F0679 at (2,+2) (depth=2, lateral=+2) "
        "after F0678 and before F0119 D2L; F0679 lines 6868-6896; "
        "F0104 line 6885; F0113 line 6894"
    };
}

static void fill_cases(DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m)
{
    int idx = 0;

    /* D2L2 wall case -> F0104 native, return */
    m->cases[idx++] = (DM1_V1_D2L2D2R2SideWallCasePc34){
        DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0678_PC34,
        "F0678_DrawD2L2",
        DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C00_WALL_PC34,
        "C00_ELEMENT_WALL",
        1, /* routes_to_f0104_native */
        0, /* routes_to_f0105_flipped */
        0, /* routes_to_f0113_teleporter */
        1, /* has_return_after_draw */
        DM1_V1_F0678_NATIVE_WALL_DRAW_LINE_PC34,
        DM1_V1_C06_WALL_D2L2_PC34,
        DM1_V1_C707_ZONE_WALL_D2L2_PC34,
        "DUNVIEW.C:6854 F0104 native, C06_WALL_D2L2+2 to C707, returns"
    };

    /* D2L2 teleporter case -> F0113, no return */
    m->cases[idx++] = (DM1_V1_D2L2D2R2SideWallCasePc34){
        DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0678_PC34,
        "F0678_DrawD2L2",
        DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C05_TELEPORTER_PC34,
        "C05_ELEMENT_TELEPORTER",
        0, /* routes_to_f0104_native */
        0, /* routes_to_f0105_flipped */
        1, /* routes_to_f0113_teleporter */
        0, /* has_return_after_draw */
        DM1_V1_F0678_TELEPORTER_DRAW_LINE_PC34,
        -1, /* no wall-set used */
        DM1_V1_C707_ZONE_WALL_D2L2_PC34,
        "DUNVIEW.C:6863 F0113 teleporter field, C09/C707"
    };

    /* D2R2 wall case -> F0104 native, return */
    m->cases[idx++] = (DM1_V1_D2L2D2R2SideWallCasePc34){
        DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0679_PC34,
        "F0679_DrawD2R2",
        DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C00_WALL_PC34,
        "C00_ELEMENT_WALL",
        1, /* routes_to_f0104_native */
        0, /* routes_to_f0105_flipped */
        0, /* routes_to_f0113_teleporter */
        1, /* has_return_after_draw */
        DM1_V1_F0679_NATIVE_WALL_DRAW_LINE_PC34,
        DM1_V1_C05_WALL_D2R2_PC34,
        DM1_V1_C708_ZONE_WALL_D2R2_PC34,
        "DUNVIEW.C:6885 F0104 native, C05_WALL_D2R2+2 to C708, returns"
    };

    /* D2R2 teleporter case -> F0113, no return */
    m->cases[idx++] = (DM1_V1_D2L2D2R2SideWallCasePc34){
        DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_F0679_PC34,
        "F0679_DrawD2R2",
        DM1_V1_D2L2_D2R2_SIDE_WALL_ELEMENT_C05_TELEPORTER_PC34,
        "C05_ELEMENT_TELEPORTER",
        0, /* routes_to_f0104_native */
        0, /* routes_to_f0105_flipped */
        1, /* routes_to_f0113_teleporter */
        0, /* has_return_after_draw */
        DM1_V1_F0679_TELEPORTER_DRAW_LINE_PC34,
        -1, /* no wall-set used */
        DM1_V1_C708_ZONE_WALL_D2R2_PC34,
        "DUNVIEW.C:6894 F0113 teleporter field, C10/C708"
    };
}

static void fill_dispatch_order(DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m)
{
    int idx = 0;

    /* F0128 dispatch sequence (PC 3.4 / MEDIA720 path, lines 8482-8542)
     * the side-row pair sits between F0118 (D3C, line 8499) and
     * F0119 (D2L, line 8513). */
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        0, "F0676_DrawD3L2 (3,-2)", 0, 8482, 0, 1,
        "DUNVIEW.C:8482 F0128 calls F0676 first (depth=3, lateral=-2)"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        1, "F0677_DrawD3R2 (3,+2)", 0, 8486, 0, 1,
        "DUNVIEW.C:8486 F0128 calls F0677 second (depth=3, lateral=+2)"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        2, "F0116_DrawD3L (3,-1)", 0, 8491, 0, 1,
        "DUNVIEW.C:8491 F0128 calls F0116 (depth=3, lateral=-1)"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        3, "F0117_DrawD3R (3,+1)", 0, 8495, 0, 1,
        "DUNVIEW.C:8495 F0128 calls F0117 (depth=3, lateral=+1)"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        4, "F0118_DrawD3C (3,0)", 0, 8499, 0, 1,
        "DUNVIEW.C:8499 F0128 calls F0118 (depth=3, lateral=0)"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        5, "F0678_DrawD2L2 (2,-2)", 0, 8504, 0, 1,
        "DUNVIEW.C:8504 F0128 calls F0678 (depth=2, lateral=-2) SIDE"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        6, "F0679_DrawD2R2 (2,+2)", 1, 8508, 0, 1,
        "DUNVIEW.C:8508 F0128 calls F0679 (depth=2, lateral=+2) SIDE"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        7, "F0119_DrawD2L (2,-1)", 0, 8513, 1, 0,
        "DUNVIEW.C:8513 F0128 calls F0119 (depth=2, lateral=-1) AFTER side"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        8, "F0120_DrawD2R (2,+1)", 1, 8517, 1, 0,
        "DUNVIEW.C:8517 F0128 calls F0120 (depth=2, lateral=+1) AFTER side"
    };
    m->dispatch_order[idx++] = (DM1_V1_D2L2D2R2SideWallDispatchOrderPc34){
        9, "F0121_DrawD2C (2,0)", 0, 8521, 1, 0,
        "DUNVIEW.C:8521 F0128 calls F0121 (depth=2, lateral=0) AFTER side"
    };
}

static void fill_sibling_rejects(DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m)
{
    m->sibling_rejects[0] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "D0L2/D0R2 F0128 caller", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 2, 7, 716,
        "DUNVIEW.C:8537-8542 F0125/F0126/F0127 D0 routes at depth=0"
    };
    m->sibling_rejects[1] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "D1L/D1R/D1C F0128 caller", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 5, 712,
        "DUNVIEW.C:8525-8533 F0122/F0123/F0124 D1 routes at depth=1"
    };
    m->sibling_rejects[2] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "D2L/D2R F0128 caller (front pair)", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 1, 10, 710,
        "DUNVIEW.C:8513-8517 F0119/F0120 D2 front pair at depth=2 lateral+-1"
    };
    m->sibling_rejects[3] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "D2C F0128 caller (center)", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 0, 11, 710,
        "DUNVIEW.C:8521 F0121 D2 center at depth=2 lateral=0"
    };
    m->sibling_rejects[4] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "D3L2/D3R2 F0676/F0677 (D3 side pair)", 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 2, 4, 702,
        "DUNVIEW.C:6226-6331 F0676/F0677 D3 side pair at depth=3 lateral+-2; "
        "the F0678/F0679 contract is for the F0128 side-row depth=2 lateral+-2"
    };
    m->sibling_rejects[5] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "D3L/D3R/D3C F0128 caller", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        3, 1, 5, 704,
        "DUNVIEW.C:8491-8499 F0116/F0117/F0118 D3 routes at depth=3"
    };
    m->sibling_rejects[6] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "CSB-lineage D2L2/D2R2 side wall", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 7, 1707,
        "CSB Viewport.cpp custom background route, not ReDMCSB DUNVIEW.C"
    };
    m->sibling_rejects[7] = (DM1_V1_D2L2D2R2SideWallSiblingRejectPc34){
        "F0678/F0679 old-media F0100_DrawWallSetBitmap", 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 7, 707,
        "Old-media F0100 (DUNVIEW.C:2080) is the F0678/F0679 D3L2/D3R2 "
        "path; on PC 3.4 the F0678/F0679 D2L2/D2R2 path uses F0104"
    };
}

static int rects_overlap(int ax, int ay, int aw, int ah,
                         int bx, int by, int bw, int bh)
{
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

static int probe_collision_count(
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *m)
{
    /* Synthetic probe rects: D2L2=8x8 at (0,20), D2R2=8x8 at (216,20).
     * Sibling reject rects are taken from the disjoint D0/D1/D2 front/D3
     * families. */
    static const int s_sibling_rects[][4] = {
        { 4, 58, 18, 74 },   /* D0L/D0R */
        { 86, 24, 46, 90 },  /* D1C */
        { 23, 42, 30, 88 },  /* D1L/D1R */
        { 54, 62, 42, 64 },  /* D2C */
        { 54, 62, 24, 55 },  /* D2L/D2R front pair */
        { 74, 76, 16, 37 },  /* D3L/D3R */
        { 98, 84, 28, 25 },  /* D3C */
        { 38, 62, 21, 29 }   /* CSB */
    };
    static const int s_lane_rects[2][4] = {
        { 0, 20, 8, 8 },     /* D2L2 probe (left side of viewport) */
        { 216, 20, 8, 8 }    /* D2R2 probe (right side of viewport) */
    };
    int collisions = 0;
    int i;
    int j;

    if (!m) return -1;
    for (i = 0; i < 2; ++i) {
        for (j = 0; j < 8; ++j) {
            collisions += rects_overlap(s_lane_rects[i][0], s_lane_rects[i][1],
                                        s_lane_rects[i][2], s_lane_rects[i][3],
                                        s_sibling_rects[j][0],
                                        s_sibling_rects[j][1],
                                        s_sibling_rects[j][2],
                                        s_sibling_rects[j][3]) ? 1 : 0;
        }
    }
    return collisions;
}

bool dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_builder_pc34(
    DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    /* F0128 caller: relative depth=2, lateral=-2/+2, call lines 8504/8508 */
    out_model->f0128_dispatches_after_d3l2 = 1; /* F0676 at 8482 */
    out_model->f0128_dispatches_after_d3r2 = 1; /* F0677 at 8486 */
    out_model->f0128_dispatches_after_d3l = 1;  /* F0116 at 8491 */
    out_model->f0128_dispatches_after_d3r = 1;  /* F0117 at 8495 */
    out_model->f0128_dispatches_after_d3c = 1;  /* F0118 at 8499 */
    out_model->f0128_dispatches_before_d2l = 1; /* F0119 at 8513 */
    out_model->f0128_dispatches_before_d2r = 1; /* F0120 at 8517 */
    out_model->f0128_dispatches_before_d2c = 1; /* F0121 at 8521 */
    out_model->f0128_dispatches_before_d1l = 1; /* F0122 at 8525 */
    out_model->f0128_dispatches_before_d1r = 1; /* F0123 at 8529 */
    out_model->f0128_dispatches_before_d1c = 1; /* F0124 at 8533 */
    out_model->f0128_dispatches_before_d0l = 1; /* F0125 at 8537 */
    out_model->f0128_dispatches_before_d0r = 1; /* F0126 at 8541 */
    out_model->f0128_dispatches_before_d0c = 1; /* F0127 at 8542 */
    out_model->side_left_of_center = 1; /* both F0678/F0679 are side, lateral != 0 */
    out_model->draws_before_sibling_right = 1; /* F0678 (D2L2) draws before F0679 (D2R2) */
    out_model->zone_pair_c707_c708 = 1;
    out_model->native_flipped_wall_set_swap = 1;
    out_model->pc_3_4_media720_path_enabled = 1;
    out_model->pc_fix_code_size_native_wall_offset = 2; /* +2 on PC 3.4 */
    out_model->flipped_wall_and_footprints_branch_excluded_pc34 = 1;
    out_model->contract_only_no_real_asset_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->no_original_dos_pixel_parity = 1;

    out_model->opaque_pixel_value = 0x77u;
    out_model->transparent_color = DM1_V1_C10_COLOR_FLESH_PC34;
    out_model->c10_transparent_preserves_destination =
        (dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(
             0xaau,
             DM1_V1_C10_COLOR_FLESH_PC34,
             DM1_V1_C10_COLOR_FLESH_PC34) == 0xaau) ? 1 : 0;
    out_model->c10_opaque_writes_source =
        (dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(
             0xaau,
             0x77u,
             DM1_V1_C10_COLOR_FLESH_PC34) == 0x77u) ? 1 : 0;

    /* F0678 contract */
    out_model->side = DM1_V1_D2L2_D2R2_SIDE_WALL_SIDE_D2L2_PC34;
    out_model->side_name = "D2L2/F0678";
    out_model->switch_dispatch_count = 2; /* C00_WALL + C05_TELEPORTER */
    out_model->wall_case_count = 1;
    out_model->teleporter_case_count = 1;
    out_model->non_wall_non_teleporter_case_count = 0;
    out_model->has_default_case = 0;
    out_model->has_break_after_wall_case = 0;
    out_model->has_return_after_wall_case = 1;
    out_model->has_break_after_teleporter_case = 0;
    out_model->has_return_after_teleporter_case = 0;
    out_model->f0107_call_count = 0;
    out_model->f0108_call_count = 0;
    out_model->f0111_call_count = 0;
    out_model->f0115_call_count = 0;
    out_model->f0104_native_call_count = 1;
    out_model->f0105_flipped_call_count = 0;
    out_model->f0113_teleporter_call_count = 1;
    out_model->f0128_caller_line = DM1_V1_F0128_F0678_CALL_LINE_PC34;
    out_model->f0128_relative_depth = 2;
    out_model->f0128_relative_lateral = -2;
    out_model->f0128_call_order = 6; /* 6th in the F0128 dispatch order */
    out_model->view_square_index = DM1_V1_C09_VIEW_SQUARE_D2L2_PC34;
    out_model->wall_zone = DM1_V1_C707_ZONE_WALL_D2L2_PC34;
    out_model->wall_set_native_index = DM1_V1_C06_WALL_D2L2_PC34;
    out_model->wall_set_flipped_index = DM1_V1_C05_WALL_D2R2_PC34;

    /* Side metadata stored in lanes for both sides */
    fill_lanes(out_model);
    fill_cases(out_model);
    fill_dispatch_order(out_model);
    fill_sibling_rejects(out_model);

    out_model->synthetic_probe_collision_count = probe_collision_count(out_model);
    out_model->synthetic_pixel_writes = 8 * 8 + 8 * 8; /* 128 opaque writes */
    out_model->synthetic_pixel_skips = 8 * 8 + 8 * 8; /* 128 transparent skips */

    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;
    out_model->deterministic_hash =
        dm1_v1_viewport_d2l2_d2r2_side_wall_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d2l2_d2r2_side_wall_hash_model_pc34(
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->switch_dispatch_count);
    h = fnv1a_u32(h, (uint32_t)model->wall_case_count);
    h = fnv1a_u32(h, (uint32_t)model->teleporter_case_count);
    h = fnv1a_u32(h, (uint32_t)model->non_wall_non_teleporter_case_count);
    h = fnv1a_u32(h, (uint32_t)model->has_default_case);
    h = fnv1a_u32(h, (uint32_t)model->has_return_after_wall_case);
    h = fnv1a_u32(h, (uint32_t)model->f0104_native_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0105_flipped_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0113_teleporter_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0107_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0108_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0111_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0115_call_count);
    h = fnv1a_u32(h, (uint32_t)model->f0128_caller_line);
    h = fnv1a_u32(h, (uint32_t)model->f0128_relative_depth);
    h = fnv1a_u32(h, (uint32_t)model->f0128_relative_lateral);
    h = fnv1a_u32(h, (uint32_t)model->f0128_call_order);
    h = fnv1a_u32(h, (uint32_t)model->view_square_index);
    h = fnv1a_u32(h, (uint32_t)model->wall_zone);
    h = fnv1a_u32(h, (uint32_t)model->wall_set_native_index);
    h = fnv1a_u32(h, (uint32_t)model->wall_set_flipped_index);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatches_after_d3c);
    h = fnv1a_u32(h, (uint32_t)model->f0128_dispatches_before_d2l);
    h = fnv1a_u32(h, (uint32_t)model->pc_fix_code_size_native_wall_offset);
    h = fnv1a_u32(h, (uint32_t)model->pc_3_4_media720_path_enabled);
    h = fnv1a_u32(h, (uint32_t)model->flipped_wall_and_footprints_branch_excluded_pc34);
    h = fnv1a_u32(h, (uint32_t)model->c10_transparent_preserves_destination);
    h = fnv1a_u32(h, (uint32_t)model->c10_opaque_writes_source);
    h = fnv1a_u32(h, (uint32_t)model->synthetic_probe_collision_count);
    h = fnv1a_u32(h, (uint32_t)model->synthetic_pixel_writes);
    h = fnv1a_u32(h, (uint32_t)model->synthetic_pixel_skips);
    for (i = 0; i < 2; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].view_square_index);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].wall_zone);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].native_wall_set_index);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].flipped_wall_set_index);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].relative_depth);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].relative_lateral);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0128_call_line);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0128_call_order_index);
    }
    for (i = 0; i < DM1_V1_D2L2_D2R2_SIDE_WALL_CASE_CAPACITY_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->cases[i].dispatcher);
        h = fnv1a_u32(h, (uint32_t)model->cases[i].element);
        h = fnv1a_u32(h, (uint32_t)model->cases[i].routes_to_f0104_native);
        h = fnv1a_u32(h, (uint32_t)model->cases[i].routes_to_f0105_flipped);
        h = fnv1a_u32(h, (uint32_t)model->cases[i].routes_to_f0113_teleporter);
        h = fnv1a_u32(h, (uint32_t)model->cases[i].has_return_after_draw);
        h = fnv1a_u32(h, (uint32_t)model->cases[i].call_line);
    }
    for (i = 0; i < DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_ORDER_CAPACITY_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->dispatch_order[i].order_index);
        h = fnv1a_u32(h, (uint32_t)model->dispatch_order[i].f0128_call_line);
    }
    for (i = 0; i < 8; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].reject_f0128_depth);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].reject_f0128_lateral);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].reject_wall_zone);
    }
    return h;
}

const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34(void)
{
    static DM1_V1_D2L2D2R2SideWallDispatchModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d2l2_d2r2_side_wall_deterministic_hash_pc34(void)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D2L2D2R2SideWallLanePc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_lane_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    return (!model || index >= 2) ? NULL : &model->lanes[index];
}

const DM1_V1_D2L2D2R2SideWallCasePc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_case_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    return (!model || index >= DM1_V1_D2L2_D2R2_SIDE_WALL_CASE_CAPACITY_PC34) ?
        NULL : &model->cases[index];
}

const DM1_V1_D2L2D2R2SideWallDispatchOrderPc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_order_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    return (!model ||
            index >= DM1_V1_D2L2_D2R2_SIDE_WALL_DISPATCH_ORDER_CAPACITY_PC34) ?
        NULL : &model->dispatch_order[index];
}

const DM1_V1_D2L2D2R2SideWallSiblingRejectPc34 *
dm1_v1_viewport_d2l2_d2r2_side_wall_sibling_reject_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2SideWallDispatchModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_side_wall_default_model_pc34();
    return (!model || index >= 8) ? NULL : &model->sibling_rejects[index];
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_switch_count_pc34(int side_index)
{
    if (side_index != 0 && side_index != 1) return -1;
    return 2; /* C00_WALL + C05_TELEPORTER only */
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_element_case_count_pc34(
    int side_index,
    int element_index)
{
    if (side_index != 0 && side_index != 1) return -1;
    if (element_index == 0) return 1; /* C00_WALL */
    if (element_index == 1) return 1; /* C05_TELEPORTER */
    return 0;
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_dispatch_call_count_pc34(
    int side_index,
    int target_function)
{
    if (side_index != 0 && side_index != 1) return -1;
    /* target_function: 0=F0104, 1=F0105, 2=F0113, 3=F0107, 4=F0108,
     * 5=F0111, 6=F0115 */
    switch (target_function) {
    case 0: return 1; /* F0104 native */
    case 1: return 0; /* F0105 flipped (PC 3.4 excludes G0076 branch) */
    case 2: return 1; /* F0113 teleporter */
    case 3: return 0; /* F0107 not called */
    case 4: return 0; /* F0108 not called */
    case 5: return 0; /* F0111 not called */
    case 6: return 0; /* F0115 not called */
    default: return -1;
    }
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_call_order_pc34(int side_index)
{
    if (side_index == 0) return 6; /* F0678 at 8504 (0-indexed 6th) */
    if (side_index == 1) return 7; /* F0679 at 8508 (0-indexed 7th) */
    return -1;
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_after_pc34(
    int side_index,
    int other_call_line)
{
    if (side_index != 0 && side_index != 1) return -1;
    /* D2L2 calls at 8504 / D2R2 at 8508. Side calls are AFTER any
     * call line < 8504 and BEFORE any call line > 8508. */
    int side_call_line = (side_index == 0) ?
        DM1_V1_F0128_F0678_CALL_LINE_PC34 :
        DM1_V1_F0128_F0679_CALL_LINE_PC34;
    return side_call_line > other_call_line ? 1 : 0;
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_f0128_dispatches_before_pc34(
    int side_index,
    int other_call_line)
{
    if (side_index != 0 && side_index != 1) return -1;
    int side_call_line = (side_index == 0) ?
        DM1_V1_F0128_F0678_CALL_LINE_PC34 :
        DM1_V1_F0128_F0679_CALL_LINE_PC34;
    return side_call_line < other_call_line ? 1 : 0;
}

uint8_t dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size)
{
    static const int s_rects[2][4] = {
        { 0, 20, 8, 8 },
        { 216, 20, 8, 8 }
    };
    int writes = 0;
    int i;
    int y;
    int x;

    if (!framebuffer ||
        framebuffer_size <
            (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 *
            (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_HEIGHT_PC34) {
        return -1;
    }
    memset(framebuffer, 0, framebuffer_size);
    for (i = 0; i < 2; ++i) {
        for (y = s_rects[i][1]; y < s_rects[i][1] + s_rects[i][3]; ++y) {
            for (x = s_rects[i][0]; x < s_rects[i][0] + s_rects[i][2]; ++x) {
                size_t offset = (size_t)y *
                    (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 +
                    (size_t)x;
                framebuffer[offset] = 0x77u;
                ++writes;
            }
        }
    }
    return writes;
}

int dm1_v1_viewport_d2l2_d2r2_side_wall_render_dispatch_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size)
{
    /* Simulate the F0128 caller sequence: blank framebuffer, mark the
     * side-row pair with a transparent color, then "dispatch" F0678/F0679
     * which write opaque pixels and consume the C10 transparent marker.
     * Returns the number of opaque pixel writes from the side pair
     * dispatcher after the C10 transparency pass. */
    static const int s_rects[2][4] = {
        { 0, 20, 8, 8 },
        { 216, 20, 8, 8 }
    };
    int writes = 0;
    int skips = 0;
    int i;
    int y;
    int x;

    if (!framebuffer ||
        framebuffer_size <
            (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 *
            (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_HEIGHT_PC34) {
        return -1;
    }
    memset(framebuffer, 0, framebuffer_size);
    /* Pass 1: F0104 native, write C10 transparent marker */
    for (i = 0; i < 2; ++i) {
        for (y = s_rects[i][1]; y < s_rects[i][1] + s_rects[i][3]; ++y) {
            for (x = s_rects[i][0]; x < s_rects[i][0] + s_rects[i][2]; ++x) {
                size_t offset = (size_t)y *
                    (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 +
                    (size_t)x;
                framebuffer[offset] = DM1_V1_C10_COLOR_FLESH_PC34;
            }
        }
    }
    /* Pass 2: C10 transparent route - blend on top, no writes */
    for (i = 0; i < 2; ++i) {
        for (y = s_rects[i][1]; y < s_rects[i][1] + s_rects[i][3]; ++y) {
            for (x = s_rects[i][0]; x < s_rects[i][0] + s_rects[i][2]; ++x) {
                size_t offset = (size_t)y *
                    (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 +
                    (size_t)x;
                /* Blend with same C10 source: preserve, count as skip */
                framebuffer[offset] = dm1_v1_viewport_d2l2_d2r2_side_wall_blend_pixel_pc34(
                    framebuffer[offset],
                    DM1_V1_C10_COLOR_FLESH_PC34,
                    DM1_V1_C10_COLOR_FLESH_PC34);
                ++skips;
            }
        }
    }
    /* Pass 3: opaque wall-set pixels actually drawn */
    for (i = 0; i < 2; ++i) {
        for (y = s_rects[i][1]; y < s_rects[i][1] + s_rects[i][3]; ++y) {
            for (x = s_rects[i][0]; x < s_rects[i][0] + s_rects[i][2]; ++x) {
                size_t offset = (size_t)y *
                    (size_t)DM1_V1_D2L2_D2R2_SIDE_WALL_VIEWPORT_WIDTH_PC34 +
                    (size_t)x;
                framebuffer[offset] = 0x77u;
                ++writes;
            }
        }
    }
    return (writes << 16) | (skips & 0xffff);
}

const char *dm1_v1_viewport_d2l2_d2r2_side_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d2l2_d2r2_side_wall_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
