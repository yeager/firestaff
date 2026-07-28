/*
 * DM1 V1 Viewport 3D Wall Rendering Pipeline — pc34 compat implementation.
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   VIEWPORT.C  — F0564_VIEWPORT_InitializeBitPlanes (line 16)
 *                 F0565_VIEWPORT_SetPalette (line 33)
 *                 F0566_VIEWPORT_BlitToScreen (line 56)
 *   DUNVIEW.C   — F0096 (line 2225), F0098 (line 2962), F0099 (line 3018),
 *                 F0100 (line 3048), F0101 (line 3065), F0102 (line 3082),
 *                 F0103 (line 3096), F0104 (line 3113), F0128 (line 8318)
 *   DRAWVIEW.C  — F0097 (platform-specific viewport blit)
 */

#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_floor_ornament_pc34_compat.h"
#include "dm1_v1_field_teleporter_effect_pc34_compat.h"
#include "dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_pc34_compat.h"
#include "dm1_v1_viewport_wall_ornament_ordinal_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include <string.h>

/* ────────────────────────────────────────────────────────────────────────────
 * Transparency color — ReDMCSB DEFS.H C10_COLOR_FLESH
 * Used by F0100_DUNGEONVIEW_DrawWallSetBitmap as the skip color.
 * ──────────────────────────────────────────────────────────────────────── */
#define COLOR_TRANSPARENT  10

/* ────────────────────────────────────────────────────────────────────────────
 * Wall Frame Table
 *
 * Derived from ReDMCSB DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8].
 * Each entry encodes the 8-byte frame descriptor for one wall position.
 * The original uses the packed format:
 *   [0]=leftX, [1]=rightX, [2]=topY, [3]=bottomY,
 *   [4]=byteWidth, [5]=height, [6]=blitX, [7]=blitY
 *
 * These values are copied directly from ReDMCSB DUNVIEW.C lines 581-594.
 * The frame array maps view squares D3L..D0R to wall bitmap positions.
 *
 * Index mapping (M600-M611 → array index):
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 *
 * Source: DUNVIEW.C lines 581-594 (G0163_aauc_Graphic558_Frame_Walls)
 * ──────────────────────────────────────────────────────────────────────── */

/* Definition of g_dm1_wall_frame_bitmaps.
 * Production builds: filled by asset loader before first draw call.
 * Test builds: NULL so door frame draw calls are no-ops until assets wired.
 * TBT-XXX: production asset system wires this from GRAPHICS.DAT. */
const uint8_t *g_dm1_wall_frame_bitmaps = NULL;

/* CSB back-wall / near-wall frame tables (4 CSB-specific positions).
 * Derived from ReDMCSB DUNVIEW.C G0711/G0712 (D3L2/D3R2, lines 579-580)
 * and DUNGEON.C G0163-equivalent (D2L2/D2R2, derived from D2L/D2R perspective).
 * These are separate from the 12-entry s_wall_frames[] array so that the
 * main DM1 draw path is undisturbed.
 *
 * D3L2 frame: G0711_auc_Graphic558_Frame_Wall_D3L2 { 0, 15, 25, 73, 8, 49, 0, 0 }
 * D3R2 frame: G0712_auc_Graphic558_Frame_Wall_D3R2 { 208, 223, 25, 73, 8, 49, 0, 0 }
 * D2L2 frame: scaled D2L (x/2, viewport-projected to D2L2 position)
 * D2R2 frame: scaled D2R (x/2, viewport-projected to D2R2 position)
 *
 * Source: DUNVIEW.C:579-580 (G0711/G0712) · DUNVIEW.C:584-585 (D2L/D2R reference)
 */
static const DM1_WallFrame s_csb_back_wall_frames[2] = {
    /* D3L2 */ {   0,  15, 25,  73,  8, 49,  0, 0 },   /* G0711, DUNVIEW.C:579 */
    /* D3R2 */ { 208, 223, 25,  73,  8, 49,  0, 0 },   /* G0712, DUNVIEW.C:580 */
};

static const DM1_WallFrame s_csb_near_wall_frames[2] = {
    /* D2L2 */ {   0,  37, 20,  90, 36, 71,  30, 0 },  /* scaled D2L left half, DUNVIEW.C:6954-6964 */
    /* D2R2 */ { 186, 223, 20,  90, 36, 71,   0, 0 },  /* scaled D2R right half, DUNVIEW.C:7105-7115 */
};

/* Standard wall frame table (12 entries, D3C..D0R).
 * Placed before csb_v1_vp_get_wall_frame to avoid forward-reference errors. */
static const DM1_WallFrame s_wall_frames[12] = {
    /* D3C */ {  74, 149, 25,  75,  64,  51,  18, 0 },
    /* D3L */ {   0,  83, 25,  75,  64,  51,  32, 0 },
    /* D3R */ { 139, 223, 25,  75,  64,  51,   0, 0 },
    /* D2C */ {  60, 163, 20,  90,  72,  71,  16, 0 },
    /* D2L */ {   0,  74, 20,  90,  72,  71,  61, 0 },
    /* D2R */ { 149, 223, 20,  90,  72,  71,   0, 0 },
    /* D1C */ {  32, 191,  9, 119, 128, 111,  48, 0 },
    /* D1L */ {   0,  63,  9, 119, 128, 111, 192, 0 },
    /* D1R */ { 160, 223,  9, 119, 128, 111,   0, 0 },
    /* D0C — unused for walls */ { 0, 223, 0, 135, 0, 0, 0, 0 },
    /* D0L */ {   0,  31,  0, 135,  16, 136,   0, 0 },
    /* D0R */ { 192, 223,  0, 135,  16, 136,   0, 0 },
};

/* ReDMCSB DUNVIEW.C:1210-1216 G0208_aaauc_Graphic558_DoorButtonCoordinateSets
 * stores the one DM1 C0_DOOR_BUTTON coordinate set in view-index order:
 * C0 D3R, C1 D3C, C2 D2C, C3 D1C.  F0110 lines 4163 and 4204-4207 select
 * one row, use source X/Y 0/0, and blit with C10 transparency. */
static const DM1_WallFrame s_door_button_frames[DM1_VIEW_DOOR_BUTTON_COUNT] = {
    /* D3R */ { 199, 204, 41, 44, 8, 4, 0, 0 },
    /* D3C */ { 136, 141, 41, 44, 8, 4, 0, 0 },
    /* D2C */ { 144, 155, 42, 47, 8, 6, 0, 0 },
    /* D1C */ { 160, 175, 44, 52, 8, 9, 0, 0 },
};

static const uint8_t s_door_button_d3_palette_remap[16] = {
    0, 0, 12, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 1, 0, 2
};

static const uint8_t s_door_button_d2_palette_remap[16] = {
    0, 12, 1, 3, 4, 3, 6, 7, 5, 9, 10, 11, 0, 2, 14, 13
};

static void dm1_viewport_3d_draw_d3_side_square(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int map_x,
    int map_y);

static void dm1_viewport_3d_draw_wall_ornament_f0107(
    DM1_Viewport3DState *state,
    int view_wall_index,
    int map_x, int map_y);

static int dm1_viewport_3d_draw_center_wall_element(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int map_x, int map_y);

static int dm1_viewport_3d_draw_side_wall_element(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int map_x, int map_y);

/* View square → wall frame table index mapping.
 * Placed before csb_v1_vp_get_wall_frame to avoid forward-reference errors. */
static int view_square_to_frame_index(DM1_ViewSquareIndex sq)
{
    switch (sq) {
        case DM1_VIEW_SQUARE_D3C: return 0;
        case DM1_VIEW_SQUARE_D3L: return 1;
        case DM1_VIEW_SQUARE_D3R: return 2;
        case DM1_VIEW_SQUARE_D2C: return 3;
        case DM1_VIEW_SQUARE_D2L: return 4;
        case DM1_VIEW_SQUARE_D2R: return 5;
        case DM1_VIEW_SQUARE_D1C: return 6;
        case DM1_VIEW_SQUARE_D1L: return 7;
        case DM1_VIEW_SQUARE_D1R: return 8;
        case DM1_VIEW_SQUARE_D0C: return 9;
        case DM1_VIEW_SQUARE_D0L: return 10;
        case DM1_VIEW_SQUARE_D0R: return 11;
        default: return -1;
    }
}

/* View square → back-wall frame table index mapping */
static int csb_back_wall_frame_index(DM1_ViewSquareIndex sq)
{
    switch (sq) {
        case DM1_VIEW_SQUARE_D3L2: return 0;
        case DM1_VIEW_SQUARE_D3R2: return 1;
        default: return -1;
    }
}

/* View square → near-wall frame table index mapping */
static int csb_near_wall_frame_index(DM1_ViewSquareIndex sq)
{
    switch (sq) {
        case DM1_VIEW_SQUARE_D2L2: return 0;
        case DM1_VIEW_SQUARE_D2R2: return 1;
        default: return -1;
    }
}

/* Extended frame lookup — covers all DM1 squares (0-12) PLUS the 4 CSB
 * back/near-wall squares (D3L2=-101, D3R2=-102, D2L2=-103, D2R2=-104).
 * Returns NULL for squares without a wall frame (D4L/D4R/D4C). */
const DM1_WallFrame *csb_v1_vp_get_wall_frame(DM1_ViewSquareIndex square)
{
    int idx = view_square_to_frame_index(square);
    if (idx >= 0 && idx < 12) return &s_wall_frames[idx];
    idx = csb_back_wall_frame_index(square);
    if (idx >= 0) return &s_csb_back_wall_frames[idx];
    idx = csb_near_wall_frame_index(square);
    if (idx >= 0) return &s_csb_near_wall_frames[idx];
    return NULL;
}

/* ReDMCSB DUNVIEW.C F0128 draw sequence, lines 8446-8542.
 * rel_depth/rel_lateral are the F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
 * arguments immediately before each draw call; D0C is the party square and does not
 * call F0150. */
static const DM1_ViewportDrawStep s_draw_order[] = {
    { DM1_VIEW_SQUARE_D4L, 4, -1, "F0115:D4L objects", "DUNVIEW.C:8466-8469" },
    { DM1_VIEW_SQUARE_D4R, 4,  1, "F0115:D4R objects", "DUNVIEW.C:8470-8473" },
    { DM1_VIEW_SQUARE_D4C, 4,  0, "F0115:D4C objects", "DUNVIEW.C:8474-8477" },
    { DM1_VIEW_SQUARE_D3L2, 3, -2, "F0676_DrawD3L2", "DUNVIEW.C:8478-8482" },
    { DM1_VIEW_SQUARE_D3R2, 3,  2, "F0677_DrawD3R2", "DUNVIEW.C:8483-8486" },
    { DM1_VIEW_SQUARE_D3L, 3, -1, "F0116_DUNGEONVIEW_DrawSquareD3L", "DUNVIEW.C:8488-8491" },
    { DM1_VIEW_SQUARE_D3R, 3,  1, "F0117_DUNGEONVIEW_DrawSquareD3R", "DUNVIEW.C:8492-8495" },
    { DM1_VIEW_SQUARE_D3C, 3,  0, "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "DUNVIEW.C:8496-8499" },
    { DM1_VIEW_SQUARE_D2L2, 2, -2, "F0678_DrawD2L2", "DUNVIEW.C:8500-8504" },
    { DM1_VIEW_SQUARE_D2R2, 2,  2, "F0679_DrawD2R2", "DUNVIEW.C:8505-8508" },
    { DM1_VIEW_SQUARE_D2L, 2, -1, "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:8510-8513" },
    { DM1_VIEW_SQUARE_D2R, 2,  1, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:8514-8517" },
    { DM1_VIEW_SQUARE_D2C, 2,  0, "F0121_DUNGEONVIEW_DrawSquareD2C", "DUNVIEW.C:8518-8521" },
    { DM1_VIEW_SQUARE_D1L, 1, -1, "F0122_DUNGEONVIEW_DrawSquareD1L", "DUNVIEW.C:8522-8525" },
    { DM1_VIEW_SQUARE_D1R, 1,  1, "F0123_DUNGEONVIEW_DrawSquareD1R", "DUNVIEW.C:8526-8529" },
    { DM1_VIEW_SQUARE_D1C, 1,  0, "F0124_DUNGEONVIEW_DrawSquareD1C", "DUNVIEW.C:8530-8533" },
    { DM1_VIEW_SQUARE_D0L, 0, -1, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8534-8537" },
    { DM1_VIEW_SQUARE_D0R, 0,  1, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8538-8541" },
    { DM1_VIEW_SQUARE_D0C, 0,  0, "F0127_DUNGEONVIEW_DrawSquareD0C", "DUNVIEW.C:8542" },
};

int dm1_viewport_3d_primary_side_wall_max_forward_pc34(
    int center_max_visible_forward)
{
    (void)center_max_visible_forward;
    /* ReDMCSB DUNVIEW.C F0128 lines 8446-8542 draws D3/D2/D1 side
     * squares before the same-depth center square, so the first side-wall
     * pass walks the full three-step envelope. */
    return 3;
}

int dm1_viewport_3d_nearest_blocking_center_depth_index_pc34(
    unsigned int blocking_depth_mask)
{
    int depth;
    /* ReDMCSB DUNVIEW.C F0128 lines 8496-8533 dispatch the center lane as
     * D3C, D2C, then D1C source squares.  Firestaff samples in party-near
     * order, so the first set bit in the D1..D3 mask is the nearest center
     * wall/door that limits same-lane visibility. */
    for (depth = 0; depth < 3; ++depth) {
        if ((blocking_depth_mask & (1u << (unsigned int)depth)) != 0u) {
            return depth;
        }
    }
    return -1;
}

int dm1_viewport_3d_nearest_blocking_center_door_depth_pc34(
    unsigned int blocking_depth_mask,
    unsigned int blocking_door_depth_mask)
{
    int nearest = dm1_viewport_3d_nearest_blocking_center_depth_index_pc34(
        blocking_depth_mask);
    if (nearest < 0) {
        return -1;
    }
    /* ReDMCSB DUNVIEW.C F0124/F0121/F0118 draws the nearest blocking
     * center square as a complete wall/door.  Door ornaments, destroyed
     * masks, and buttons belong only to that same blocking center door;
     * farther center doors behind a nearer wall/door cannot decorate over it. */
    return (blocking_door_depth_mask & (1u << (unsigned int)nearest)) != 0u
        ? nearest
        : -1;
}

int dm1_viewport_3d_max_visible_forward_from_center_pc34(
    unsigned int blocking_depth_mask)
{
    int nearest = dm1_viewport_3d_nearest_blocking_center_depth_index_pc34(
        blocking_depth_mask);
    return nearest >= 0 ? nearest + 1 : 3;
}

int dm1_viewport_3d_use_flipped_walls_pc34(
    int party_map_x,
    int party_map_y,
    int party_direction)
{
    /* ReDMCSB DUNVIEW.C F0128 line 8357 sets
     * G0076_B_UseFlippedWallAndFootprintsBitmaps from the party tuple
     * before dispatching the visible square draw order. */
    return (party_map_x + party_map_y + party_direction) & 1;
}

int dm1_viewport_3d_side_lane_clear_for_rel_pc34(int rel_forward,
                                                 int rel_side,
                                                 unsigned int open_depth_mask)
{
    int d;
    if (rel_side == 0 || rel_forward <= 0) {
        return 1;
    }
    /* Source lock: DUNVIEW.C F0128 draws complete side squares in
     * far-to-near order (D3L/D3R before D3C, then D2L/D2R before D2C,
     * then D1L/D1R before D1C; see ReDMCSB DUNVIEW.C:8488-8533).
     * A nearer non-open side square therefore occludes farther side-lane
     * floor, pit, field, content, projectile, and ornament passes. */
    for (d = 0; d < rel_forward - 1 && d < 3; ++d) {
        if ((open_depth_mask & (1u << (unsigned int)d)) == 0u) {
            return 0;
        }
    }
    return 1;
}

unsigned int dm1_viewport_3d_open_depth_mask_from_cells_pc34(
    const int open_by_depth[3])
{
    int depth;
    unsigned int mask = 0u;
    if (!open_by_depth) {
        return 0u;
    }
    for (depth = 0; depth < 3; ++depth) {
        if (open_by_depth[depth]) {
            mask |= 1u << (unsigned int)depth;
        }
    }
    return mask;
}

DM1_ViewportCenterLaneMasksPc34 dm1_viewport_3d_center_lane_masks_from_cells_pc34(
    const int valid_by_depth[3],
    const int open_by_depth[3],
    const int door_by_depth[3])
{
    int depth;
    DM1_ViewportCenterLaneMasksPc34 masks;
    masks.valid_depth_mask = 0u;
    masks.open_depth_mask = 0u;
    masks.blocking_depth_mask = 0u;
    masks.blocking_door_depth_mask = 0u;
    if (!valid_by_depth || !open_by_depth || !door_by_depth) {
        return masks;
    }
    /* ReDMCSB DUNVIEW.C F0128/F0124/F0121/F0118: center-lane depth
     * visibility is derived from the valid D1..D3 center squares; non-open
     * center squares block farther center content, and only closed center
     * doors enter the door-ornament/button path. */
    for (depth = 0; depth < 3; ++depth) {
        unsigned int bit = 1u << (unsigned int)depth;
        if (!valid_by_depth[depth]) {
            continue;
        }
        masks.valid_depth_mask |= bit;
        if (open_by_depth[depth]) {
            masks.open_depth_mask |= bit;
        } else {
            masks.blocking_depth_mask |= bit;
            if (door_by_depth[depth]) {
                masks.blocking_door_depth_mask |= bit;
            }
        }
    }
    return masks;
}

DM1_ViewportLaneVisibilityReceiptPc34 dm1_viewport_3d_lane_visibility_from_cells_pc34(
    const int center_valid_by_depth[3],
    const int center_open_by_depth[3],
    const int center_door_by_depth[3],
    const int left_open_by_depth[3],
    const int right_open_by_depth[3])
{
    DM1_ViewportLaneVisibilityReceiptPc34 receipt;
    receipt.center =
        dm1_viewport_3d_center_lane_masks_from_cells_pc34(
            center_valid_by_depth,
            center_open_by_depth,
            center_door_by_depth);
    receipt.left_open_depth_mask =
        dm1_viewport_3d_open_depth_mask_from_cells_pc34(left_open_by_depth);
    receipt.right_open_depth_mask =
        dm1_viewport_3d_open_depth_mask_from_cells_pc34(right_open_by_depth);
    receipt.max_visible_forward =
        dm1_viewport_3d_max_visible_forward_from_center_pc34(
            receipt.center.blocking_depth_mask);
    receipt.nearest_blocking_center_depth_index =
        dm1_viewport_3d_nearest_blocking_center_depth_index_pc34(
            receipt.center.blocking_depth_mask);
    receipt.nearest_blocking_center_door_depth =
        dm1_viewport_3d_nearest_blocking_center_door_depth_pc34(
            receipt.center.blocking_depth_mask,
            receipt.center.blocking_door_depth_mask);
    receipt.center_visible_depth_mask =
        dm1_viewport_3d_center_visible_depth_mask_pc34(
            receipt.center.valid_depth_mask,
            receipt.center.open_depth_mask);
    return receipt;
}

int dm1_viewport_3d_side_lane_clear_from_visibility_pc34(
    const DM1_ViewportLaneVisibilityReceiptPc34* visibility,
    int rel_forward,
    int rel_side)
{
    unsigned int mask;
    if (!visibility) {
        return 0;
    }
    if (rel_side == 0) {
        return 1;
    }
    /* ReDMCSB DUNVIEW.C F0128 draws side squares in their depth bucket
     * before the center wall/door for that bucket. A center blocker limits
     * the center corridor, but must not erase source-ordered side walls such
     * as D2L/D2R beside a D1C wall. */
    mask = rel_side < 0
        ? visibility->left_open_depth_mask
        : visibility->right_open_depth_mask;
    return dm1_viewport_3d_side_lane_clear_for_rel_pc34(rel_forward,
                                                        rel_side,
                                                        mask);
}

int dm1_viewport_3d_center_line_clear_from_visibility_pc34(
    const DM1_ViewportLaneVisibilityReceiptPc34* visibility,
    int depth_index)
{
    if (!visibility) {
        return 0;
    }
    return dm1_viewport_3d_center_line_clear_before_depth_pc34(
        depth_index,
        visibility->center.open_depth_mask);
}

int dm1_viewport_3d_center_line_clear_before_depth_pc34(
    int depth_index,
    unsigned int open_depth_mask)
{
    int d;
    if (depth_index <= 0) {
        return 1;
    }
    /* ReDMCSB DUNVIEW.C F0128 draws center squares in depth order D3C,
     * D2C, D1C. A nearer non-open center square blocks farther center
     * content and the late split side-content repair passes. */
    for (d = 0; d < depth_index && d < 3; ++d) {
        if ((open_depth_mask & (1u << (unsigned int)d)) == 0u) {
            return 0;
        }
    }
    return 1;
}

int dm1_viewport_3d_center_visible_depth_mask_pc34(
    unsigned int valid_depth_mask,
    unsigned int open_depth_mask)
{
    int depth;
    unsigned int mask = 0u;
    for (depth = 0; depth < 3; ++depth) {
        unsigned int bit = 1u << (unsigned int)depth;
        if ((valid_depth_mask & bit) == 0u ||
            (open_depth_mask & bit) == 0u) {
            break;
        }
        mask |= bit;
    }
    return (int)mask;
}

static void dm1_viewport_3d_notify_pre_square_draw(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int relative_forward,
    int relative_side)
{
    if (state && state->pre_square_draw_callback) {
        state->pre_square_draw_callback(state->pre_square_draw_user_data,
                                        square,
                                        relative_forward,
                                        relative_side);
    }
}

/* ReDMCSB DUNVIEW.C F0128 lines 8466-8477 draw D4L, D4R, then D4C by
 * resolving relative map coordinates and calling F0115 with
 * F0162_DUNGEON_GetSquareFirstObject(...), M598/M599/M597 and
 * C0x0001_CELL_ORDER_BACKLEFT.  This happens before the D3 side-wall helpers
 * at lines 8478-8499, so nearer wall panels occlude any far object pixels. */
static const DM1_ViewportFarObjectPassSpec s_far_object_pass_specs[] = {
    { DM1_VIEW_SQUARE_D4L, 4, -1, 0x0001, 17, true, "DUNVIEW.C:8466-8469; DEFS.H:2613 M598_VIEW_SQUARE_D4L" },
    { DM1_VIEW_SQUARE_D4R, 4,  1, 0x0001, 18, true, "DUNVIEW.C:8470-8473; DEFS.H:2614 M599_VIEW_SQUARE_D4R" },
    { DM1_VIEW_SQUARE_D4C, 4,  0, 0x0001, 16, true, "DUNVIEW.C:8474-8477; DEFS.H:2612 M597_VIEW_SQUARE_D4C" },
};

int dm1_viewport_3d_object_source_scale_units(int scale_index)
{
    /* ReDMCSB DUNVIEW.C:1207 G2030_auc_ObjectScales.  F0115 lines
     * 5030-5039 select one of these source units before deriving an object
     * bitmap for the current view depth/cell. */
    static const unsigned char k_object_scales[5] = { 27, 21, 18, 14, 12 };
    if (scale_index < 0) scale_index = 0;
    if (scale_index > 4) scale_index = 4;
    return (int)k_object_scales[scale_index];
}

int dm1_viewport_3d_object_source_scale_index(int depth_index,
                                              int relative_cell)
{
    int front_row = relative_cell >= 2;
    int idx;

    /* ReDMCSB DUNVIEW.C F0115: D1/native objects use the native bucket;
     * deeper view rows derive the bucket from view depth and the
     * front/back half of AL0126_i_ViewCell. */
    if (depth_index <= 0) return 0;
    idx = depth_index * 2 - (front_row ? 1 : 0);
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return idx;
}

void dm1_viewport_3d_object_pile_shift_indices(int pile_index,
                                               int *out_x_index,
                                               int *out_y_index)
{
    /* ReDMCSB DUNVIEW.C:1228-1235
     * G0217_aauc_Graphic558_ObjectPileShiftSetIndices. */
    static const unsigned char k_pile_shift_indices[16][2] = {
        { 2, 5 }, { 0, 6 }, { 5, 7 }, { 3, 0 },
        { 7, 1 }, { 1, 2 }, { 6, 3 }, { 3, 3 },
        { 5, 5 }, { 2, 6 }, { 7, 7 }, { 1, 0 },
        { 3, 1 }, { 6, 2 }, { 1, 3 }, { 5, 3 }
    };
    if (pile_index < 0) pile_index = 0;
    pile_index &= 0x0F;
    if (out_x_index) *out_x_index = (int)k_pile_shift_indices[pile_index][0];
    if (out_y_index) *out_y_index = (int)k_pile_shift_indices[pile_index][1];
}

int dm1_viewport_3d_object_source_shift_value(int shift_set,
                                              int shift_index)
{
    /* ReDMCSB DUNVIEW.C:1237-1240 G0223_aac_Graphic558_ShiftSets.
     * COORD.C F0637 applies these when the C2500 zone has
     * MASK0x8000_SHIFT_OBJECTS_AND_CREATURES. */
    static const signed char k_shift_sets[3][8] = {
        { 0, 1, 2, 3, 0, -3, -2, -1 },
        { 0, 1, 1, 2, 0, -2, -1, -1 },
        { 0, 1, 1, 1, 0, -1, -1, -1 }
    };
    if (shift_set < 0) shift_set = 0;
    if (shift_set > 2) shift_set = 2;
    if (shift_index < 0) shift_index = 0;
    if (shift_index > 7) shift_index = 7;
    return (int)k_shift_sets[shift_set][shift_index];
}

int dm1_viewport_3d_f0115_view_square_index(int rel_forward,
                                            int rel_side)
{
    /* ReDMCSB DUNVIEW.C / DEFS.H MEDIA720 visible-square order:
     * D1C/L/R = 3/4/5, D2C/L/R = 6/7/8, D3C/L/R = 11/12/13.
     * D3L2/D3R2 = 14/15. DUNVIEW.C F0115 uses this square id with
     * G2028_ac_ViewSquareIndexTo for object/projectile source rows. */
    static const signed char k_view_square[3][3] = {
        { 4,  3,  5 },
        { 7,  6,  8 },
        {12, 11, 13 }
    };
    if (rel_forward < 1 || rel_forward > 3) return -1;
    if (rel_side == -2 && rel_forward == 3) return 14;
    if (rel_side == 2 && rel_forward == 3) return 15;
    if (rel_side < -1 || rel_side > 1) return -1;
    return (int)k_view_square[rel_forward - 1][rel_side + 1];
}

int dm1_viewport_3d_f0115_c2500_c2900_row(int rel_forward,
                                          int rel_side)
{
    /* ReDMCSB DUNVIEW.C F0115 lines 4923/5075/5668-5683:
     * C2500/C2900 rows are selected as G2028[viewSquare] before
     * AL0126_i_ViewCell chooses the per-cell source zone. */
    static const signed char k_g2028_view_square_to_row[23] = {
        11, -1, -1,  8,  9, 10,  5,  6,  7, -1, -1,
         0,  1,  2,  3,  4, -1, -1, -1, -1, -1, -1, -1
    };
    int view_square = dm1_viewport_3d_f0115_view_square_index(rel_forward,
                                                              rel_side);
    if (view_square < 0 || view_square >= 23) return -1;
    return (int)k_g2028_view_square_to_row[view_square];
}

int dm1_viewport_3d_c2500_object_zone_point(int scale_index,
                                            int relative_cell,
                                            int *out_x,
                                            int *out_y)
{
    /* ReDMCSB DUNVIEW.C F0115 line 5075 binds C2500_ZONE_ plus
     * G2028 row and AL0126_i_ViewCell.  These five canonical source rows
     * match the current source-scale buckets used by the Firestaff
     * object-render path. */
    static const short k_c2500[5][4][2] = {
        {{   0,   0 }, {   0,   0 }, { 127,  70 }, {  98,  70 }},
        {{   0,   0 }, {   0,   0 }, {  62,  70 }, {  25,  70 }},
        {{   0,   0 }, {   0,   0 }, { 200,  70 }, { 162,  70 }},
        {{   0,   0 }, {   0,   0 }, {   2,  70 }, { -35,  70 }},
        {{   0,   0 }, {   0,   0 }, { 258,  70 }, { 222,  70 }}
    };
    int zx;
    int zy;
    if (scale_index < 0) scale_index = 0;
    if (scale_index > 4) scale_index = 4;
    if (relative_cell < 0 || relative_cell > 3) return 0;
    zx = (int)k_c2500[scale_index][relative_cell][0];
    zy = (int)k_c2500[scale_index][relative_cell][1];
    if (zx == 0 && zy == 0) return 0;
    if (out_x) *out_x = zx;
    if (out_y) *out_y = zy;
    return 1;
}

int dm1_viewport_3d_c2500_object_raw_zone_point(int row_index,
                                                int relative_cell,
                                                int *out_x,
                                                int *out_y)
{
    /* ReDMCSB DUNVIEW.C F0115 lines 4923 and 5075 select rows through
     * G2028_ac_ViewSquareIndexTo; COORD.C F0637/F0640 resolve layout-696
     * C2500..C2567 and apply MASK0x8000 object/creature pile shifts. */
    static const short k_c2500_raw[17][4][2] = {
        {{   0,   0 }, {   0,   0 }, { 127,  70 }, {  98,  70 }},
        {{   0,   0 }, {   0,   0 }, {  62,  70 }, {  25,  70 }},
        {{   0,   0 }, {   0,   0 }, { 200,  70 }, { 162,  70 }},
        {{   0,   0 }, {   0,   0 }, {   2,  70 }, { -35,  70 }},
        {{   0,   0 }, {   0,   0 }, { 258,  70 }, { 222,  70 }},
        {{  94,  78 }, { 131,  78 }, { 136,  88 }, {  89,  88 }},
        {{  10,  78 }, {  53,  79 }, {  41,  88 }, { -14,  89 }},
        {{ 171,  78 }, { 218,  78 }, { 236,  89 }, { 184,  88 }},
        {{  83,  99 }, { 141,  99 }, { 150, 115 }, {  76, 115 }},
        {{ -40, 101 }, {  24,  99 }, {   5, 114 }, { -79, 117 }},
        {{ 200,  99 }, { 262, 101 }, { 301, 117 }, { 220, 114 }},
        {{  66, 133 }, { 158, 133 }, {   0,   0 }, {   0,   0 }},
        {{ 113,  62 }, {  46,  61 }, { 180,  61 }, { 115,  74 }},
        {{   8,  73 }, { 220,  74 }, { 115,  92 }, { 112,  60 }},
        {{  45,  61 }, { 179,  60 }, { 114,  73 }, {   4,  73 }},
        {{ 219,  73 }, { 114,  88 }, { 113,  63 }, {  45,  62 }},
        {{ 181,  62 }, { 114,  74 }, {  11,  73 }, { 218,  74 }}
    };
    int zx;
    int zy;
    if (row_index < 0 || row_index >= 17) return 0;
    if (relative_cell < 0 || relative_cell > 3) return 0;
    zx = (int)k_c2500_raw[row_index][relative_cell][0];
    zy = (int)k_c2500_raw[row_index][relative_cell][1];
    if (zx == 0 && zy == 0) return 0;
    if (out_x) *out_x = zx;
    if (out_y) *out_y = zy;
    return 1;
}

int dm1_viewport_3d_c2548_alcove_object_zone_point(int coordinate_set,
                                                    int alcove_row,
                                                    int *out_x,
                                                    int *out_y)
{
    /* Graphic 558 / layout-696 C2548..C2568.  F0115 indexes these as
     * coordinateSet * 7 + G2029[viewSquare]; C2548 is an anchor family,
     * so it must never be substituted with C2500's four-cell floor rows. */
    static const short k_c2548[3][7][2] = {
        {{113, 62}, { 46, 61}, {180, 61}, {115, 74},
         {  8, 73}, {220, 74}, {115, 92}},
        {{112, 60}, { 45, 61}, {179, 60}, {114, 73},
         {  4, 73}, {219, 73}, {114, 88}},
        {{113, 63}, { 45, 62}, {181, 62}, {114, 74},
         { 11, 73}, {218, 74}, {114, 88}}
    };

    if (coordinate_set < 0 || coordinate_set >= 3 ||
        alcove_row < 0 || alcove_row >= 7) {
        return 0;
    }
    if (out_x) *out_x = k_c2548[coordinate_set][alcove_row][0];
    if (out_y) *out_y = k_c2548[coordinate_set][alcove_row][1];
    return 1;
}

int dm1_viewport_3d_c2900_projectile_zone_point(int scale_index,
                                                int relative_cell,
                                                int *out_x,
                                                int *out_y)
{
    /* ReDMCSB DUNVIEW.C F0115 lines 5667-5683 binds projectile placement
     * through C2900_ZONE_ plus G2028 row and AL0126_i_ViewCell.  These
     * five rows mirror Firestaff's source-scale buckets for the current
     * projectile renderer path. */
    static const short k_c2900[5][4][2] = {
        {{   0,  0 }, {   0,  0 }, { 129, 47 }, {  95, 47 }},
        {{   0,  0 }, {   0,  0 }, {  62, 47 }, {  25, 47 }},
        {{   0,  0 }, {   0,  0 }, { 200, 47 }, { 162, 47 }},
        {{   0,  0 }, {   0,  0 }, {   2, 47 }, { -35, 47 }},
        {{   0,  0 }, {   0,  0 }, { 258, 47 }, { 202, 47 }}
    };
    int zx;
    int zy;
    if (scale_index < 0) scale_index = 0;
    if (scale_index > 4) scale_index = 4;
    if (relative_cell < 0 || relative_cell > 3) return 0;
    zx = (int)k_c2900[scale_index][relative_cell][0];
    zy = (int)k_c2900[scale_index][relative_cell][1];
    if (zx == 0 && zy == 0) return 0;
    if (out_x) *out_x = zx;
    if (out_y) *out_y = zy;
    return 1;
}

int dm1_viewport_3d_c2900_projectile_raw_zone_point(int row_index,
                                                    int relative_cell,
                                                    int *out_x,
                                                    int *out_y)
{
    /* ReDMCSB DUNVIEW.C F0115 lines 5635-5683 restores the projectile
     * view cell and resolves C2900_ZONE_ through G2028 rows.  Layout-696
     * source rows C2900..C2947 are kept visible here so D1/D2/D3 side and
     * deep projectile placement cannot silently fall back to synthetic panes. */
    static const short k_c2900_raw[12][4][2] = {
        {{   0,  0 }, {   0,  0 }, { 129, 47 }, {  95, 47 }},
        {{   0,  0 }, {   0,  0 }, {  62, 47 }, {  25, 47 }},
        {{   0,  0 }, {   0,  0 }, { 200, 47 }, { 162, 47 }},
        {{   0,  0 }, {   0,  0 }, {   2, 47 }, { -35, 47 }},
        {{   0,  0 }, {   0,  0 }, { 258, 47 }, { 202, 47 }},
        {{  92, 47 }, { 132, 46 }, { 136, 47 }, {  88, 47 }},
        {{  10, 47 }, {  53, 47 }, {  41, 47 }, { -14, 47 }},
        {{ 171, 47 }, { 218, 47 }, { 236, 47 }, { 183, 47 }},
        {{  83, 47 }, { 140, 47 }, { 148, 47 }, {  76, 47 }},
        {{ -40, 47 }, {  26, 47 }, {   5, 47 }, { -79, 47 }},
        {{ 197, 47 }, { 262, 47 }, { 301, 47 }, { 220, 47 }},
        {{  66, 47 }, { 158, 47 }, {   0,  0 }, {   0,  0 }}
    };
    int zx;
    int zy;
    if (row_index < 0 || row_index >= 12) return 0;
    if (relative_cell < 0 || relative_cell > 3) return 0;
    zx = (int)k_c2900_raw[row_index][relative_cell][0];
    zy = (int)k_c2900_raw[row_index][relative_cell][1];
    if (zx == 0 && zy == 0) return 0;
    if (out_x) *out_x = zx;
    if (out_y) *out_y = zy;
    return 1;
}

static int dm1_viewport_3d_creature_front_point_index(int coord_set,
                                                      int visible_count,
                                                      int slot_index)
{
    int point_index = 4;
    if (slot_index < 0) slot_index = 0;
    if (slot_index > 3) slot_index = 3;
    if (coord_set == 1) {
        if (visible_count > 1) {
            point_index = slot_index < 2 ? slot_index : 4;
        }
    } else if (visible_count > 1) {
        point_index = slot_index;
        if (point_index > 3) point_index = 3;
    }
    return point_index;
}

int dm1_viewport_3d_c3200_creature_zone_point(int coord_set,
                                              int depth_index,
                                              int visible_count,
                                              int slot_index,
                                              int *out_x,
                                              int *out_y)
{
    /* ReDMCSB DUNVIEW.C F0115 lines 5201-5214 and 5615-5617 bind
     * creature placement through C3200_ZONE_ plus coordinate set, view
     * depth, and view cell.  These layout-696 points are viewport-local
     * center-X / bottom-Y anchors for the center lane. */
    static const short k_c3200_center[3][3][5][2] = {
        {
            {{ 83, 106 }, { 141, 106 }, { 148, 119 }, {  76, 119 }, { 112, 111 }},
            {{ 92,  83 }, { 131,  83 }, { 132,  90 }, {  91,  90 }, { 112,  85 }},
            {{ 97,  67 }, { 125,  67 }, { 129,  72 }, {  95,  72 }, { 112,  72 }}
        },
        {
            {{ 81, 119 }, { 142, 119 }, { 112, 105 }, { 112, 111 }, { 112, 119 }},
            {{ 91,  90 }, { 132,  90 }, { 112,  83 }, { 112,  85 }, { 112,  89 }},
            {{ 94,  73 }, { 128,  73 }, { 112,  70 }, { 112,  70 }, { 112,  73 }}
        },
        {
            {{ 83,  79 }, { 141,  79 }, { 148,  85 }, {  76,  85 }, { 112,  81 }},
            {{ 92,  65 }, { 131,  65 }, { 132,  67 }, {  91,  67 }, { 112,  66 }},
            {{ 95,  59 }, { 127,  59 }, { 129,  61 }, {  93,  61 }, { 112,  60 }}
        }
    };
    int point_index;
    if (coord_set < 0 || coord_set > 2) return 0;
    if (depth_index < 0) depth_index = 0;
    if (depth_index > 2) depth_index = 2;
    point_index = dm1_viewport_3d_creature_front_point_index(coord_set,
                                                             visible_count,
                                                             slot_index);
    {
        int x = (int)k_c3200_center[coord_set][depth_index][point_index][0];
        int y = (int)k_c3200_center[coord_set][depth_index][point_index][1];
        /* A zero pair is an empty source coordinate. F0115 has no generic
         * replacement placement for it. */
        if (x == 0 && y == 0) return 0;
        if (out_x) *out_x = x;
        if (out_y) *out_y = y;
    }
    return 1;
}

int dm1_viewport_3d_c3200_creature_side_zone_point(int coord_set,
                                                   int depth_index,
                                                   int side_hint,
                                                   int visible_count,
                                                   int slot_index,
                                                   int *out_x,
                                                   int *out_y)
{
    /* ReDMCSB DUNVIEW.C G0224 lines 1836-1870 stores layout-696 creature
     * coordinate rows in D3C/D3L/D3R, D2C/D2L/D2R, D1C/D1L/D1R,
     * D0L/D0R order.  Side lookup keeps that order so side-lane creatures
     * do not fall through to D0/offscreen coordinates. */
    static const short k_c3200_side[3][3][2][5][2] = {
        {
            {{{  46, 103 }, { 118, 103 }, { 101, 119 }, {   0,   0 }, {  79, 111 }},
             {{ 107, 103 }, { 177, 103 }, {   0,   0 }, { 123, 119 }, { 144, 111 }}},
            {{{  99,  81 }, { 146,  81 }, { 135,  90 }, {  80,  90 }, { 120,  85 }},
             {{  77,  81 }, { 124,  81 }, { 143,  90 }, {  89,  90 }, { 105,  85 }}},
            {{{ 131,  70 }, { 163,  70 }, { 158,  75 }, { 120,  75 }, { 145,  72 }},
             {{  59,  70 }, {  91,  70 }, { 107,  75 }, {  66,  75 }, {  79,  72 }}}
        },
        {
            {{{   0,   0 }, { 101, 119 }, {  84, 105 }, {  70, 111 }, {  77, 119 }},
             {{ 123, 119 }, {   0,   0 }, { 139, 105 }, { 153, 111 }, { 146, 119 }}},
            {{{  80,  90 }, { 135,  90 }, { 125,  83 }, { 120,  85 }, { 125,  90 }},
             {{  89,  90 }, { 143,  90 }, {  99,  83 }, { 105,  85 }, {  98,  90 }}},
            {{{ 120,  75 }, { 158,  75 }, { 149,  70 }, { 145,  72 }, { 150,  75 }},
             {{  66,  75 }, { 104,  75 }, {  75,  70 }, {  79,  72 }, {  73,  75 }}}
        },
        {
            {{{  46,  79 }, { 118,  79 }, { 101,  85 }, {   0,   0 }, {  79,  81 }},
             {{ 107,  79 }, { 177,  79 }, {   0,   0 }, { 123,  85 }, { 144,  81 }}},
            {{{  99,  65 }, { 146,  65 }, { 135,  67 }, {  80,  67 }, { 120,  66 }},
             {{  77,  65 }, { 124,  65 }, { 143,  67 }, {  89,  67 }, { 105,  66 }}},
            {{{ 131,  59 }, { 163,  59 }, { 158,  61 }, { 120,  61 }, { 145,  60 }},
             {{  59,  59 }, {  91,  59 }, { 107,  61 }, {  66,  61 }, {  79,  60 }}}
        }
    };
    int side_index;
    int point_index;
    if (coord_set < 0 || coord_set > 2) return 0;
    if (depth_index < 0) depth_index = 0;
    if (depth_index > 2) depth_index = 2;
    side_index = side_hint < 0 ? 0 : 1;
    point_index = dm1_viewport_3d_creature_front_point_index(coord_set,
                                                             visible_count,
                                                             slot_index);
    {
        int x = (int)k_c3200_side[coord_set][depth_index][side_index][point_index][0];
        int y = (int)k_c3200_side[coord_set][depth_index][side_index][point_index][1];
        /* G0224's zero pair is deliberately blank, not a pane-relative
         * fallback coordinate. */
        if (x == 0 && y == 0) return 0;
        if (out_x) *out_x = x;
        if (out_y) *out_y = y;
    }
    return 1;
}

/* ReDMCSB DUNVIEW.C F0128 lines 8488-8499 dispatch D3L, D3R, then D3C
 * after resolving their map cells through DUNGEON.C F0150 lines 1371-1421.
 * F0116 lines 6406-6437 prove D3L wall/alcove handling returns before the
 * ordinary F0115 object handoff unless the front ornament is an alcove, so a
 * read-only D3L1 target excludes D3L itself while preserving the rest of the
 * F0128 draw list. */
static const DM1_ViewSquareIndex s_d3l1_no_write_allowed_squares[] = {
    DM1_VIEW_SQUARE_D4L,
    DM1_VIEW_SQUARE_D4R,
    DM1_VIEW_SQUARE_D4C,
    DM1_VIEW_SQUARE_D3L2,
    DM1_VIEW_SQUARE_D3R2,
    DM1_VIEW_SQUARE_D3R,
    DM1_VIEW_SQUARE_D3C,
    DM1_VIEW_SQUARE_D2L2,
    DM1_VIEW_SQUARE_D2R2,
    DM1_VIEW_SQUARE_D2L,
    DM1_VIEW_SQUARE_D2R,
    DM1_VIEW_SQUARE_D2C,
    DM1_VIEW_SQUARE_D1L,
    DM1_VIEW_SQUARE_D1R,
    DM1_VIEW_SQUARE_D1C,
    DM1_VIEW_SQUARE_D0L,
    DM1_VIEW_SQUARE_D0R,
    DM1_VIEW_SQUARE_D0C,
};

static const DM1_ViewportNoWriteSpec s_d3l1_no_write_spec = {
    DM1_VIEW_SQUARE_D3L,
    s_d3l1_no_write_allowed_squares,
    sizeof(s_d3l1_no_write_allowed_squares) / sizeof(s_d3l1_no_write_allowed_squares[0]),
    "F0116_DUNGEONVIEW_DrawSquareD3L",
    "DUNVIEW.C:6361-6495 F0116; DUNVIEW.C:8488-8499 F0128 D3 dispatch; DUNGEON.C:1371-1421 F0150",
    "DUNVIEW.C:6406-6437 wall case returns before F0115 unless front alcove; DUNVIEW.C:6475-6480 open-cell F0108/F0115 path"
};


/* PC34/I34E wall bitmap selection table, source-locked to the MEDIA709/720
 * draw calls in ReDMCSB DUNVIEW.C.  These entries encode the native draw
 * bitmap and the parity draw bitmap used with F0105/F0792. */

/* F0115 per-cell z-order.  ReDMCSB explicitly scans the thing list multiple
 * times for each cell: objects first, then creatures, then projectiles; after
 * all packed cells are processed it restarts once more for explosions/fluxcage. */
static const DM1_ViewportThingLayerSpec s_thing_layers[] = {
    { DM1_VIEWPORT_THING_LAYER_OBJECTS,     "objects",     "DUNVIEW.C:4567-4571,4853-4860", true,  false },
    { DM1_VIEWPORT_THING_LAYER_CREATURES,   "creatures",   "DUNVIEW.C:4573,5195-5202",      true,  false },
    { DM1_VIEWPORT_THING_LAYER_PROJECTILES, "projectiles", "DUNVIEW.C:4575-4577,5681-5883", true,  false },
    { DM1_VIEWPORT_THING_LAYER_EXPLOSIONS,  "explosions",  "DUNVIEW.C:4579-4581,5915-5933", false, true  },
};


static const DM1_ViewportProjectileOcclusionSpec s_projectile_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D0C,   0, 0, 11, "DEFS.H:2596; DUNVIEW.C:373,5675-5676,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D1C,   3, 1,  8, "DEFS.H:2599; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D1L,   4, 1,  9, "DEFS.H:2600; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D1R,   5, 1, 10, "DEFS.H:2601; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D2C,   6, 2,  5, "DEFS.H:2602; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D2L,   7, 2,  6, "DEFS.H:2603; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D2R,   8, 2,  7, "DEFS.H:2604; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3C,  11, 3,  0, "DEFS.H:2607; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3L,  12, 3,  1, "DEFS.H:2608; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3R,  13, 3,  2, "DEFS.H:2609; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3L2, 14, 3,  3, "DEFS.H:2610; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3R2, 15, 3,  4, "DEFS.H:2611; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
};

static const DM1_ViewportExplosionOcclusionSpec s_explosion_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D0C,   0, 0, 14, 13, 11, true,  "DEFS.H:2596,3749,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6031-6071,6094-6129" },
    { DM1_VIEW_SQUARE_D0L,   1, 0, 15, 14, -1, false, "DEFS.H:2597,4234-4235; DUNVIEW.C:376-377,5920-5923,6106-6129" },
    { DM1_VIEW_SQUARE_D0R,   2, 0, 16, 15, -1, false, "DEFS.H:2598,4234-4235; DUNVIEW.C:376-377,5920-5923,6106-6129" },
    { DM1_VIEW_SQUARE_D1C,   3, 1, 11, 10,  8, false, "DEFS.H:2599,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D1L,   4, 1, 12, 11,  9, false, "DEFS.H:2600,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D1R,   5, 1, 13, 12, 10, false, "DEFS.H:2601,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D2C,   6, 2,  8,  7,  5, false, "DEFS.H:2602,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D2L,   7, 2,  9,  8,  6, false, "DEFS.H:2603,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D2R,   8, 2, 10,  9,  7, false, "DEFS.H:2604,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D3C,  11, 3,  3,  2,  0, false, "DEFS.H:2607,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D3L,  12, 3,  4,  3,  1, false, "DEFS.H:2608,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D3R,  13, 3,  5,  4,  2, false, "DEFS.H:2609,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D3L2, 14, 3,  6,  0,  3, false, "DEFS.H:2610,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D3R2, 15, 3,  7,  1,  4, false, "DEFS.H:2611,4232-4235; DUNVIEW.C:373,376-377,5920-5923,5983-6000,6094-6129" },
    { DM1_VIEW_SQUARE_D4C,  16, 4,  0, -1, -1, false, "DEFS.H:2612,4234-4235; DUNVIEW.C:376-377,5920-5923,6106-6129" },
    { DM1_VIEW_SQUARE_D4L,  17, 4,  1, -1, -1, false, "DEFS.H:2613,4234-4235; DUNVIEW.C:376-377,5920-5923,6106-6129" },
    { DM1_VIEW_SQUARE_D4R,  18, 4,  2, -1, -1, false, "DEFS.H:2614,4234-4235; DUNVIEW.C:376-377,5920-5923,6106-6129" },
};

static const DM1_ViewportDoorFrontOcclusionSpec s_door_front_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D3L2, 0x0218, 0x0349, "DUNVIEW.C:6270 floor ornament under far rear pass", "DUNVIEW.C:6271 pass1 rear cells before far door", "DUNVIEW.C:6272 no separate far frame; F0111 draws C3700_ZONE_DOOR_D3L2", NULL, "DUNVIEW.C:6272 F0111 door bitmap/ornament", "DUNVIEW.C:6273-6286 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D3R2, 0x0128, 0x0439, "DUNVIEW.C:6337 floor ornament under mirrored far rear pass", "DUNVIEW.C:6338 pass1 rear cells before far door", "DUNVIEW.C:6339 no separate far frame; F0111 draws C3710_ZONE_DOOR_D3R2", NULL, "DUNVIEW.C:6339 F0111 door bitmap/ornament", "DUNVIEW.C:6340-6353 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D3L, 0x0218, 0x0349, "DUNVIEW.C:6443 floor ornament under rear pass", "DUNVIEW.C:6444 pass1 rear cells before left frame", "DUNVIEW.C:6446-6454 left/right frame draw", NULL, "DUNVIEW.C:6457 F0111 door bitmap/ornament", "DUNVIEW.C:6459 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D3R, 0x0128, 0x0439, "DUNVIEW.C:6579 floor ornament under mirrored rear pass", "DUNVIEW.C:6580 pass1 rear cells before right frame", "DUNVIEW.C:6582-6590 mirrored frame draw", "DUNVIEW.C:6592-6593 optional button before door panel", "DUNVIEW.C:6598-6599 F0111 door bitmap/ornament", "DUNVIEW.C:6601 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D3C, 0x0218, 0x0349, "DUNVIEW.C:6722 floor ornament under rear pass", "DUNVIEW.C:6723 pass1 rear cells before frame", "DUNVIEW.C:6725-6739 side frame and button draw", "DUNVIEW.C:6737-6739 optional button before door panel", "DUNVIEW.C:6744 F0111 door bitmap/ornament", "DUNVIEW.C:6746 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D2L, 0x0218, 0x0349, "DUNVIEW.C:6988 floor ornament under rear pass", "DUNVIEW.C:6989 pass1 rear cells before top frame", "DUNVIEW.C:6991-6998 top frame draw", NULL, "DUNVIEW.C:7000-7001 F0111 door bitmap/ornament", "DUNVIEW.C:7003 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D2R, 0x0128, 0x0439, "DUNVIEW.C:7181 floor ornament under mirrored rear pass", "DUNVIEW.C:7182 pass1 rear cells before top frame", "DUNVIEW.C:7184-7191 mirrored top frame draw", NULL, "DUNVIEW.C:7193-7194 F0111 door bitmap/ornament", "DUNVIEW.C:7196 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D2C, 0x0218, 0x0349, "DUNVIEW.C:7314 floor ornament under rear pass", "DUNVIEW.C:7315 pass1 rear cells before frame", "DUNVIEW.C:7317-7333 top/side frame and button draw", "DUNVIEW.C:7332-7334 optional button before door panel", "DUNVIEW.C:7339 F0111 door bitmap/ornament", "DUNVIEW.C:7341 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D1L, 0x0028, 0x0039, "DUNVIEW.C:7493 floor ornament under D1L rear pass", "DUNVIEW.C:7494 pass1 back-right cell before top frame", "DUNVIEW.C:7496-7504 top frame draw", NULL, "DUNVIEW.C:7506 F0111 door bitmap/ornament", "DUNVIEW.C:7508-7536 pass2 front-right cell after door" },
    { DM1_VIEW_SQUARE_D1R, 0x0018, 0x0049, "DUNVIEW.C:7661 floor ornament under mirrored D1R rear pass", "DUNVIEW.C:7662 pass1 back-left cell before top frame", "DUNVIEW.C:7664-7672 mirrored top frame draw", NULL, "DUNVIEW.C:7674 F0111 door bitmap/ornament", "DUNVIEW.C:7676-7704 pass2 front-left cell after door" },
    { DM1_VIEW_SQUARE_D1C, 0x0218, 0x0349, "DUNVIEW.C:7874 floor ornament under rear pass", "DUNVIEW.C:7874-7875 pass1 rear cells before frame", "DUNVIEW.C:7877-7902 top/side frame and button draw", "DUNVIEW.C:7901-7902 optional button before door panel", "DUNVIEW.C:7905-7908 F0111 door bitmap/ornament", "DUNVIEW.C:7910-7937 pass2 front cells after door" },
};

static const DM1_ViewportSideOcclusionSpec s_side_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D3L, 0x0321, "F0116_DUNGEONVIEW_DrawSquareD3L", "DUNVIEW.C:6438-6441 door-side/stairs-side branch", "DUNVIEW.C:6478-6480 floor ornament then F0115 with C0x0321" },
    { DM1_VIEW_SQUARE_D3R, 0x0412, "F0117_DUNGEONVIEW_DrawSquareD3R", "DUNVIEW.C:6574-6577 door-side/stairs-side branch", "DUNVIEW.C:6619-6621 floor ornament then F0115 with C0x0412" },
    { DM1_VIEW_SQUARE_D2L, 0x0342, "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:6974-6986 stairs-side falls through to door-side order", "DUNVIEW.C:7017-7027 floor ornament/ceiling pit then F0115 with C0x0342" },
    { DM1_VIEW_SQUARE_D2R, 0x0431, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:7167-7179 stairs-side falls through to door-side order", "DUNVIEW.C:7209-7219 floor ornament/ceiling pit then F0115 with C0x0431" },
    { DM1_VIEW_SQUARE_D1L, 0x0032, "F0122_DUNGEONVIEW_DrawSquareD1L", "DUNVIEW.C:7461-7491 stairs-side falls through to door-side order", "DUNVIEW.C:7524-7536 floor ornament/ceiling pit then F0115 with C0x0032" },
    { DM1_VIEW_SQUARE_D1R, 0x0041, "F0123_DUNGEONVIEW_DrawSquareD1R", "DUNVIEW.C:7629-7659 stairs-side falls through to door-side order", "DUNVIEW.C:7692-7704 floor ornament/ceiling pit then F0115 with C0x0041" },
    { DM1_VIEW_SQUARE_D0L, 0x0002, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8000-8005 door-side/teleporter branch", "DUNVIEW.C:8005 F0115 with C0x0002" },
    { DM1_VIEW_SQUARE_D0R, 0x0001, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8110-8115 door-side/teleporter branch", "DUNVIEW.C:8115 F0115 with C0x0001" },
};

static const DM1_ViewportThievesEyeDoorFrameOcclusionSpec s_thieves_eye_door_frame_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D0C, 0x0021, 728, 736,
      "DUNVIEW.C:8185-8188 D0C door-side branch checks Event73Count_ThievesEye",
      "DUNVIEW.C:8199-8201 copies G2116_DoorFrameFrontD0C into temporary bitmap and initializes M711 hole graphic",
      "DUNVIEW.C:8206-8210 resolves C736_ZONE_THIEVES_EYE_HOLE_IN_DOOR_FRAME and blits the hole into the temporary frame",
      "DUNVIEW.C:8215-8216 blits temporary frame to C728_ZONE_DOOR_FRAME_D0C before D0C common F0115",
      "DUNVIEW.C:8240,8294 break then common F0115 with C0x0021" },
};

static const DM1_ViewportPostCommandRedrawSpec s_post_command_redraw = {
    true,
    true,
    true,
    "COMMAND.C:2045-2156/F0380 pops a queued command; lines 2118-2127 pop/unlock, 2150-2156 dispatch turn/move mutations",
    "GAMELOOP.C:55-90 next loop iteration redraws F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)",
    "DRAWVIEW.C:709-722 F0097_DUNGEONVIEW_DrawViewport requests the G0296 viewport blit and waits for vertical blank",
};

static const DM1_ViewportSameViewportCaptureContract s_same_viewport_capture_contract = {
    true,
    true,
    true,
    true,
    "COMMAND.C:106-114 PC34 movement/dungeon-view mouse zones feed the original command queue labels",
    "COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC pops one queued command and dispatches turn/move handlers",
    "CLIKMENU.C:142-174 F0365_COMMAND_ProcessTypes1To2_TurnParty mutates direction and stops waiting",
    "CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty resolves relative movement/result ticks",
    "DUNVIEW.C:8318-8611 F0128_DUNGEONVIEW_Draw_CPSF composes G0296 from direction/mapX/mapY and calls F0097",
    "DRAWVIEW.C:709-858 F0097_DUNGEONVIEW_DrawViewport blits G0296 through the PC34 viewport-present boundary",
    "_canonical/dm1 README hashes: GRAPHICS.DAT 2c3aa836..., DUNGEON.DAT d90b6b1c..., TITLE adc7f191...",
};

static const DM1_ViewportFloorFieldOrderSpec s_floor_field_order_specs[] = {
    { DM1_VIEW_SQUARE_D3L2, 0x3421, true, true, true, true, true, false, true,
      "F0676_DrawD3L2",
      "DUNVIEW.C:6237-6252 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6275-6278 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6282-6284 order then F0108 floor ornament",
      "DUNVIEW.C:6286 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6288-6289 teleporter field after F0115",
      "DUNVIEW.C:6253-6264 wall bitmap/ornament then return before F0115" },
    { DM1_VIEW_SQUARE_D3R2, 0x4312, true, true, true, true, true, false, true,
      "F0677_DrawD3R2",
      "DUNVIEW.C:6304-6319 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6342-6345 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6349-6351 order then F0108 floor ornament",
      "DUNVIEW.C:6353 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6355-6356 teleporter field after F0115",
      "DUNVIEW.C:6320-6331 wall bitmap/ornament then return before F0115" },
    { DM1_VIEW_SQUARE_D3L, 0x3421, true, true, true, true, true, false, true,
      "F0116_DUNGEONVIEW_DrawSquareD3L",
      "DUNVIEW.C:6375-6405 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6461-6472 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6475-6478 order then F0108 floor ornament",
      "DUNVIEW.C:6480 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6482-6495 teleporter field after F0115",
      "DUNVIEW.C:6406-6437 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D3R, 0x4312, true, true, true, true, true, false, true,
      "F0117_DUNGEONVIEW_DrawSquareD3R",
      "DUNVIEW.C:6514-6544 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6603-6614 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6617-6620 order then F0108 floor ornament",
      "DUNVIEW.C:6622 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6624-6638 teleporter field after F0115",
      "DUNVIEW.C:6545-6573 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D3C, 0x3421, true, true, true, true, true, false, true,
      "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
      "DUNVIEW.C:6666-6696 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6748-6762 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6811-6814 order then F0108 floor ornament",
      "DUNVIEW.C:6816 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6818-6831 teleporter field after F0115",
      "DUNVIEW.C:6697-6720 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D2L, 0x3421, true, true, true, true, true, false, true,
      "F0119_DUNGEONVIEW_DrawSquareD2L",
      "DUNVIEW.C:6914-6944 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:7005-7015 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:7017-7020 order then F0108 floor ornament; ceiling pit follows before F0115",
      "DUNVIEW.C:7031 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:7033-7048 teleporter field after F0115",
      "DUNVIEW.C:6945-6973 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D2R, 0x4312, true, true, true, true, true, false, true,
      "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
      "DUNVIEW.C:7065-7095 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:7198-7208 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:7210-7213 order then F0108 floor ornament; ceiling pit follows before F0115",
      "DUNVIEW.C:7224 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:7226-7240 teleporter field after F0115",
      "DUNVIEW.C:7097-7166 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D2C, 0x3421, true, true, true, true, true, false, true,
      "F0121_DUNGEONVIEW_DrawSquareD2C",
      "DUNVIEW.C:7260-7288 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:7343-7353 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:7355-7357 order then F0108 floor ornament; ceiling pit follows before F0115",
      "DUNVIEW.C:7367-7368 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:7370-7388 teleporter field after F0115",
      "DUNVIEW.C:7289-7312 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D2L2, 0x0000, false, false, false, false, false, false, true,
      "F0678_DrawD2L2",
      "DUNVIEW.C:6846-6865 no stairs branch in D2L2 helper",
      "DUNVIEW.C:6846-6865 no pit branch in D2L2 helper",
      "DUNVIEW.C:6846-6865 no floor ornament or F0115 cell pass in D2L2 helper",
      "DUNVIEW.C:6846-6865 no F0115 thing pass in D2L2 helper",
      "DUNVIEW.C:6863-6865 teleporter field draws directly in C707_ZONE_WALL_D2L2",
      "DUNVIEW.C:6848-6862 wall bitmap then return before teleporter field" },
    { DM1_VIEW_SQUARE_D2R2, 0x0000, false, false, false, false, false, false, true,
      "F0679_DrawD2R2",
      "DUNVIEW.C:6877-6896 no stairs branch in D2R2 helper",
      "DUNVIEW.C:6877-6896 no pit branch in D2R2 helper",
      "DUNVIEW.C:6877-6896 no floor ornament or F0115 cell pass in D2R2 helper",
      "DUNVIEW.C:6877-6896 no F0115 thing pass in D2R2 helper",
      "DUNVIEW.C:6894-6896 teleporter field draws directly in C708_ZONE_WALL_D2R2",
      "DUNVIEW.C:6879-6893 wall bitmap then return before teleporter field" },
    { DM1_VIEW_SQUARE_D1L, 0x0032, true, true, true, true, true, false, true,
      "F0122_DUNGEONVIEW_DrawSquareD1L",
      "DUNVIEW.C:7405-7435 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:7510-7520 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:7522-7533 order then F0108 floor ornament; ceiling pit follows before F0115",
      "DUNVIEW.C:7535-7536 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:7538-7555 teleporter field after F0115",
      "DUNVIEW.C:7436-7460 wall bitmap/ornament then return" },
    { DM1_VIEW_SQUARE_D1R, 0x0041, true, true, true, true, true, false, true,
      "F0123_DUNGEONVIEW_DrawSquareD1R",
      "DUNVIEW.C:7573-7603 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:7678-7688 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:7690-7701 order then F0108 floor ornament; ceiling pit follows before F0115",
      "DUNVIEW.C:7703-7704 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:7706-7722 teleporter field after F0115",
      "DUNVIEW.C:7604-7628 wall bitmap/ornament then return" },
    { DM1_VIEW_SQUARE_D0L, 0x0002, true, true, false, true, true, false, true,
      "F0125_DUNGEONVIEW_DrawSquareD0L",
      "DUNVIEW.C:7978-7988 stairs-side bitmap returns before common F0115",
      "DUNVIEW.C:7989-7998 pit bitmap falls through before F0115",
      "DUNVIEW.C:7999-8005 no floor-ornament call; ceiling pit precedes F0115",
      "DUNVIEW.C:8005 F0115 object/creature/projectile/explosion handoff with C0x0002",
      "DUNVIEW.C:8050-8059 teleporter field after F0115",
      "DUNVIEW.C:8007-8038 wall bitmap then return before F0115/field" },
    { DM1_VIEW_SQUARE_D0R, 0x0001, true, true, false, true, true, false, true,
      "F0126_DUNGEONVIEW_DrawSquareD0R",
      "DUNVIEW.C:8082-8092 stairs-side bitmap returns before common F0115",
      "DUNVIEW.C:8093-8102 pit bitmap falls through before F0115",
      "DUNVIEW.C:8103-8115 no floor-ornament call; ceiling pit precedes F0115",
      "DUNVIEW.C:8115 F0115 object/creature/projectile/explosion handoff with C0x0001",
      "DUNVIEW.C:8150-8159 teleporter field after F0115",
      "DUNVIEW.C:8117-8144 wall bitmap then return before F0115/field" },
    { DM1_VIEW_SQUARE_D0C, 0x0021, true, true, false, true, true, true, false,
      "F0127_DUNGEONVIEW_DrawSquareD0C",
      "DUNVIEW.C:8241-8273 stairs front bitmap draws and breaks before F0115",
      "DUNVIEW.C:8274-8292 pit floor/ceiling bitmap before F0115",
      "DUNVIEW.C:8284-8294 no D0C floor-ornament call in this branch; ceiling pit precedes F0115",
      "DUNVIEW.C:8294 F0115 object/creature/projectile/explosion handoff with C0x0021",
      "DUNVIEW.C:8295-8308 teleporter field after F0115",
      "DUNVIEW.C:8185-8240 door-side case breaks before common F0115; no wall case in D0C" },
};

/* Pass760: D0L2/D0R2 pair composition source lock.
 * ReDMCSB anchors: DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament:3940-4011
 * (floor-ornament ordinal and MASK0x8000_FOOTPRINTS keepout);
 * DUNVIEW.C F0107:3502-3938 (wall-ornament ordinal/coordinate set);
 * DUNVIEW.C F0098:2962-3002 (floor/ceiling base);
 * DUNVIEW.C F0115:4547-4581,5180-5188,5211-5214,5668-5671
 * (thing-pass cell ordering); DUNVIEW.C:6432-6600
 * M575_VIEW_WALL_D3L_RIGHT; DEFS.H:2088,2596-2611,2668-2677,
 * 2698-2702,4045-4046; DRAWVIEW.C F0097:1-50 wall-side dispatch
 * plus DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 C10 blits. */
static const DM1_ViewportD0L2D0R2F0108CompositionSpec
s_d0l2_d0r2_f0108_composition_specs[] = {
    {
        DM1_VIEW_SQUARE_D0L2, DM1_VIEW_SQUARE_D0L,
        DM1_WALL_D0L, DM1_WALL_D0R,
        DM1_PC34_ZONE_WALL_D0L, DM1_PC34_ZONE_WALL_D3L,
        0, -2, 1, 8, 2, 4,
        0x0002, 0x0000, 0x8000u, 1500, 11,
        true, true, true, true, true, true, true, true,
        true, true, true,
        "DUNVIEW.C:6432-6480 wall ornament/order path; DUNVIEW.C:8016-8038 D0L wall return",
        "DUNVIEW.C:3940-4011 F0108 floor ornament MASK0x8000 keepout",
        "DUNVIEW.C:3502-3938 F0107 wall ornament ordinal/coordinateSet",
        "DUNVIEW.C:2962-3002 F0098 floor+ceiling base",
        "DUNVIEW.C:4547-4581,5180-5188,5211-5214,5668-5671 F0115 thing-pass ordering",
        "DEFS.H:2088 C10; DEFS.H:2596-2611 view-square ordinals; DEFS.H:2668-2677/2698-2702 cell/view-wall ordinals; DEFS.H:4045-4046 C705/C706",
        "DRAWVIEW.C F0097:1-50 wall-side dispatch; DUNVIEW.C F0104:3113-3156; DUNVIEW.C F0105:3185-3247"
    },
    {
        DM1_VIEW_SQUARE_D0R2, DM1_VIEW_SQUARE_D0R,
        DM1_WALL_D0R, DM1_WALL_D0L,
        DM1_PC34_ZONE_WALL_D0R, DM1_PC34_ZONE_WALL_D3R,
        0, 2, 2, 10, 3, 6,
        0x0001, 0x0000, 0x8000u, 1500, 11,
        true, true, true, true, true, true, true, true,
        true, true, true,
        "DUNVIEW.C:6545-6600 wall ornament/order path; DUNVIEW.C:8126-8144 D0R wall return",
        "DUNVIEW.C:3940-4011 F0108 floor ornament MASK0x8000 keepout",
        "DUNVIEW.C:3502-3938 F0107 wall ornament ordinal/coordinateSet",
        "DUNVIEW.C:2962-3002 F0098 floor+ceiling base",
        "DUNVIEW.C:4547-4581,5180-5188,5211-5214,5668-5671 F0115 thing-pass ordering",
        "DEFS.H:2088 C10; DEFS.H:2596-2611 view-square ordinals; DEFS.H:2668-2677/2698-2702 cell/view-wall ordinals; DEFS.H:4045-4046 C705/C706",
        "DRAWVIEW.C F0097:1-50 wall-side dispatch; DUNVIEW.C F0104:3113-3156; DUNVIEW.C F0105:3185-3247"
    },
};

static const DM1_ViewportWallDrawSpec s_wall_draw_specs[] = {
    { DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, true,  false, DM1_PC34_ZONE_WALL_D3L2, true,  false, 3, -2, 0,   25, 44,  49,  "F0676_DrawD3L2",                  "DUNVIEW.C:6254-6260", "DUNVIEW.C:6263-6264 wall ornament then return" },
    { DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, true,  false, DM1_PC34_ZONE_WALL_D3R2, true,  false, 3,  2, 180, 25, 44,  49,  "F0677_DrawD3R2",                  "DUNVIEW.C:6321-6327", "DUNVIEW.C:6330-6331 wall ornament then return" },
    { DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  true,  false, DM1_PC34_ZONE_WALL_D3L,  true,  true,  3, -1, 7,   25, 83,  49,  "F0116_DUNGEONVIEW_DrawSquareD3L", "DUNVIEW.C:6421-6427", "DUNVIEW.C:6432-6437 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  true,  false, DM1_PC34_ZONE_WALL_D3R,  true,  true,  3,  1, 134, 25, 83,  49,  "F0117_DUNGEONVIEW_DrawSquareD3R", "DUNVIEW.C:6554-6564", "DUNVIEW.C:6568-6573 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C,  true,  true,  DM1_PC34_ZONE_WALL_D3C,  true,  true,  3,  0, 77,  25, 70,  49,  "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "DUNVIEW.C:6707-6714", "DUNVIEW.C:6716-6720 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, true,  false, DM1_PC34_ZONE_WALL_D2L2, true,  false, 2, -2, 0,   24, 8,   52,  "F0678_DrawD2L2",                  "DUNVIEW.C:6849-6858", "DUNVIEW.C:6848-6862 wall case returns" },
    { DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, true,  false, DM1_PC34_ZONE_WALL_D2R2, true,  false, 2,  2, 216, 24, 8,   52,  "F0679_DrawD2R2",                  "DUNVIEW.C:6880-6889", "DUNVIEW.C:6882-6893 wall case returns" },
    { DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R,  true,  false, DM1_PC34_ZONE_WALL_D2L,  true,  true,  2, -1, 0,   20, 75,  71,  "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:6954-6964", "DUNVIEW.C:6968-6973 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L,  true,  false, DM1_PC34_ZONE_WALL_D2R,  true,  true,  2,  1, 149, 20, 75,  71,  "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:7105-7115", "DUNVIEW.C:7119-7123 front alcove branch; DUNVIEW.C:7166 blocker return" },
    { DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  true,  true,  DM1_PC34_ZONE_WALL_D2C,  true,  true,  2,  0, 59,  19, 106, 74,  "F0121_DUNGEONVIEW_DrawSquareD2C", "DUNVIEW.C:7299-7306", "DUNVIEW.C:7308-7312 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  true,  false, DM1_PC34_ZONE_WALL_D1L,  true,  false, 1, -1, 0,   9,  60,  111, "F0122_DUNGEONVIEW_DrawSquareD1L", "DUNVIEW.C:7445-7455", "DUNVIEW.C:7459-7460 side ornament then return" },
    { DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  true,  false, DM1_PC34_ZONE_WALL_D1R,  true,  false, 1,  1, 164, 9,  60,  111, "F0123_DUNGEONVIEW_DrawSquareD1R", "DUNVIEW.C:7613-7623", "DUNVIEW.C:7627-7628 side ornament then return" },
    { DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C,  true,  true,  DM1_PC34_ZONE_WALL_D1C,  false, true,  1,  0, 32,  9,  160, 111, "F0124_DUNGEONVIEW_DrawSquareD1C", "DUNVIEW.C:7833-7840", "DUNVIEW.C:7842-7843 front alcove draws F0115; no side cells behind D1C" },
    { DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  true,  false, DM1_PC34_ZONE_WALL_D0L,  true,  false, 0, -1, 0,   0,  33,  136, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8016-8033", "DUNVIEW.C:8036-8038 wall case returns" },
    { DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  true,  false, DM1_PC34_ZONE_WALL_D0R,  true,  false, 0,  1, 191, 0,  33,  136, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8126-8139", "DUNVIEW.C:8142-8144 wall case returns" },
};

static const uint8_t *dm1_viewport_3d_selected_wall_bitmap(const DM1_Viewport3DState *state,
                                                           const uint8_t *bm_base,
                                                           DM1_WallSetIndex selected_wall)
{
    const uint8_t *pixels = NULL;
    int width = 0;
    int height = 0;

    if (!state || selected_wall < 0 || selected_wall >= DM1_WALL_SET_COUNT) return NULL;

    /* CSB stores G2107's 15 cells as individual C093..C107 IMG3 records.
     * The DM1 atlas is only a legacy fallback and is never live-CSB input. */
    if (state->graphic_provider_callback &&
        state->graphic_provider_callback(
            state->graphic_provider_user_data,
            dm1_v1_graphic_wallset0_index_pc34((int)selected_wall),
            &pixels, &width, &height) &&
        pixels && width > 0 && height > 0) {
        return pixels;
    }
    if (!bm_base) return NULL;

    /* ReDMCSB PC34/I34E selects the G2107_WallSet[] entry first, then
     * F0105 flips that selected native bitmap horizontally when parity is
     * active.  Do not index G3048/G3071 here after selecting the opposite
     * wall, or side lanes such as F0676 D3L2 can double-swap back to the
     * original wall.  Source: DUNVIEW.C:6254-6260,6321-6327,6849-6889;
     * F0128 restores G3071 only after the whole draw in lines 8577-8579. */
    return bm_base + (int)state->wall_set_native[selected_wall] * DM1_VIEWPORT_BYTE_WIDTH;
}

static int dm1_viewport_3d_classify_grid_cell(int cell)
{
    /* ReDMCSB: DEFS.H M034_SQUARE_TYPE is square >> 5; DUNGEON.C F0172
     * derives closed fakewalls/door-stair orientation from that raw byte.
     * Some legacy Firestaff callers pass normalized element IDs 0..6, so
     * keep those stable while making raw DUNGEON.DAT bytes render correctly.
     * The grid must not pass F0172 extended aspects 16..19; 0x10 is a valid
     * raw wall byte carrying MASK0x0010_THING_LIST_PRESENT. */
    if (cell >= DM1_VP_ELEMENT_WALL && cell <= DM1_VP_ELEMENT_FAKEWALL) {
        return cell;
    }
    return (((unsigned int)cell) & 0xFFu) >> 5;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_init
 *
 * Source: VIEWPORT.C F0564_VIEWPORT_InitializeBitPlanes (line 16)
 *   Sets up source/destination bitplane pointers for the 224×136 viewport.
 *   Amiga: 4 bitplanes, each M091_BITPLANE_SIZE(224, 136) bytes.
 *   PC34: single chunky 8-bit buffer, stride = width.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_init(DM1_Viewport3DState *state,
                          uint8_t *viewport_pixels,
                          int viewport_stride)
{
    memset(state, 0, sizeof(*state));
    state->viewport_pixels = viewport_pixels;
    state->viewport_stride = viewport_stride > 0 ? viewport_stride
                                                  : DM1_VIEWPORT_WIDTH;
    state->floor_area = viewport_pixels +
                        DM1_VIEWPORT_FLOOR_Y * state->viewport_stride;
    state->floor_graphic  = -1;
    state->ceiling_graphic = -2;
    state->floor_ceiling_dirty = true;

    /* Default wall set indices (ReDMCSB DUNVIEW.C G2107, I34E section).
     * Negative values = derived bitmap offset from wall set base. */
    static const int16_t default_wall_set[DM1_WALL_SET_COUNT] = {
        -17, -16, -15, -14, -13,   /* D0R, D0L, D1R, D1L, D1C */
         -9,  -8, -12, -11, -10,   /* D2R2, D2L2, D2R, D2L, D2C */
         -4,  -3,  -7,  -6,  -5    /* D3R2, D3L2, D3R, D3L, D3C */
    };
    memcpy(state->wall_set, default_wall_set, sizeof(default_wall_set));
    memcpy(state->wall_set_native, default_wall_set, sizeof(default_wall_set));

    /* Default door frame indices (DUNVIEW.C G2110-G2120, I34E) */
    static const int16_t default_door_frames[DM1_DOOR_FRAME_COUNT] = {
        -35, -33, -34, -32, -30, -31,  /* Top: D1R,D1L,D1LCR,D2R,D2L,D2LCR */
        -29, -28, -27, -26, -25, -24   /* Front D0C, Right D1C, Left D1C..D3L */
    };
    memcpy(state->door_frames, default_door_frames, sizeof(default_door_frames));
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_load_wall_set
 *
 * Source: DUNVIEW.C F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF (line 2225)
 *   Loads wall set bitmaps based on map's wall set index.
 *   Creates flipped variants via F0099.
 *   Sets up door frame bitmaps, floor/ceiling from floor set.
 *
 * In the original, this loads actual bitmap data from GRAPHICS.DAT.
 * Our implementation sets the index offsets; actual bitmap loading
 * is deferred to the asset system.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_load_wall_set(DM1_Viewport3DState *state,
                                   int wall_set_index,
                                   int floor_set_index)
{
    (void)wall_set_index; /* Index used by asset loader, not stored here */

    /* Floor/ceiling graphic indices.
     * ReDMCSB DUNVIEW.C line 126-127: G2108_Floor = -1, G2109_Ceiling = -2
     * These are derived bitmap indices relative to the floor set base. */
    state->floor_graphic  = -1;
    state->ceiling_graphic = -2;
    (void)floor_set_index;

    /* Copy native wall set as backup for parity restore.
     * ReDMCSB F0128 line 8575: restores G3071_WallSetNotFlipped → G2107 */
    memcpy(state->wall_set_native, state->wall_set, sizeof(state->wall_set));

    /* Build flipped wall set.
     * ReDMCSB DUNVIEW.C G3048_WallSetFlipped[15] (I34E, line ~230):
     * Mirrors L↔R within each depth group. */
    static const int flip_map[DM1_WALL_SET_COUNT] = {
        1,  0,  3,  2,  4,   /* D0R↔D0L, D1R↔D1L, D1C stays */
        6,  5,  8,  7,  9,   /* D2R2↔D2L2, D2R↔D2L, D2C stays */
       11, 10, 13, 12, 14    /* D3R2↔D3L2, D3R↔D3L, D3C stays */
    };
    for (int i = 0; i < DM1_WALL_SET_COUNT; i++) {
        state->wall_set_flipped[i] = state->wall_set_native[flip_map[i]];
    }

    state->floor_ceiling_dirty = true;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_floor_ceiling
 *
 * Source: DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling (line 2962)
 *   Amiga A20E path:
 *     1. Clear black area (37 lines × 4 bitplanes)
 *     2. Copy ceiling bitmap → viewport top (29 lines)
 *     3. Copy floor bitmap → viewport floor area (70 lines)
 *   PC34 (F20E/I34E) path:
 *     1. STARTUP2.C:621-622 lays out C079 (224×39) followed by C078
 *        (224×97) as the complete 224×136 aperture.
 *     2. F0674_F0128_sub: copies that cached ceiling/floor pair via
 *        GetBitmapPointer; there is no Amiga black inter-band.
 *
 * The legacy 29/70 path retains the source's non-PC geometry.  The PC3.4
 * path consumes the complete cached pair. Do not manufacture floor/ceiling
 * pixels when a GRAPHICS.DAT provider cannot supply the selected source
 * graphics.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_floor_ceiling(DM1_Viewport3DState *state)
{
    uint8_t *vp = state->viewport_pixels;
    int stride = state->viewport_stride;
    const uint8_t *ceiling_pixels = NULL;
    const uint8_t *floor_pixels = NULL;
    int ceiling_width = 0;
    int ceiling_height = 0;
    int floor_width = 0;
    int floor_height = 0;

    if (state->graphic_provider_callback &&
        state->graphic_provider_callback(
            state->graphic_provider_user_data, state->ceiling_graphic,
            &ceiling_pixels, &ceiling_width, &ceiling_height) &&
        state->graphic_provider_callback(
            state->graphic_provider_user_data, state->floor_graphic,
            &floor_pixels, &floor_width, &floor_height) &&
        ceiling_pixels && floor_pixels &&
        ceiling_width >= DM1_VIEWPORT_WIDTH &&
        floor_width >= DM1_VIEWPORT_WIDTH &&
        ceiling_height >= DM1_PC34_VIEWPORT_CEILING_H &&
        floor_height >= DM1_PC34_VIEWPORT_FLOOR_H) {
        /* PC 3.4: STARTUP2.C:621-622 derives G0086/G0087 from C079/C078's
         * actual byte counts. F0674 therefore copies the complete 39-row
         * ceiling and 97-row floor pair with no Amiga black inter-band. */
        for (int y = 0; y < DM1_PC34_VIEWPORT_CEILING_H; y++) {
            memcpy(vp + y * stride,
                   ceiling_pixels + y * ceiling_width,
                   DM1_VIEWPORT_WIDTH);
        }
        for (int y = 0; y < DM1_PC34_VIEWPORT_FLOOR_H; y++) {
            memcpy(vp + (DM1_PC34_VIEWPORT_FLOOR_Y + y) * stride,
                   floor_pixels + y * floor_width,
                   DM1_VIEWPORT_WIDTH);
        }
        state->floor_ceiling_dirty = false;
        return;
    }

    ceiling_pixels = NULL;
    floor_pixels = NULL;
    ceiling_width = ceiling_height = 0;
    floor_width = floor_height = 0;

    /* Clear viewport black area: lines 0..36 (37 lines).
     * ReDMCSB F0098 Amiga: F0008_MAIN_ClearBytes for each bitplane.
     * PC34: F0008_MAIN_ClearBytes(ViewportBlackArea, BlackAreaByteCount). */
    for (int y = 0; y < DM1_VIEWPORT_BLACK_AREA_H; y++) {
        memset(vp + y * stride, 0, (size_t)DM1_VIEWPORT_WIDTH);
    }

    if (state->graphic_provider_callback &&
        state->graphic_provider_callback(
            state->graphic_provider_user_data, state->ceiling_graphic,
            &ceiling_pixels, &ceiling_width, &ceiling_height) &&
        ceiling_pixels && ceiling_width >= DM1_VIEWPORT_WIDTH &&
        ceiling_height >= DM1_VIEWPORT_CEILING_H) {
        for (int y = 0; y < DM1_VIEWPORT_CEILING_H; y++) {
            memcpy(vp + y * stride,
                   ceiling_pixels + y * ceiling_width,
                   DM1_VIEWPORT_WIDTH);
        }
    }

    if (state->graphic_provider_callback &&
        state->graphic_provider_callback(
            state->graphic_provider_user_data, state->floor_graphic,
            &floor_pixels, &floor_width, &floor_height) &&
        floor_pixels && floor_width >= DM1_VIEWPORT_WIDTH &&
        floor_height >= DM1_VIEWPORT_FLOOR_H) {
        for (int y = 0; y < DM1_VIEWPORT_FLOOR_H; y++) {
            memcpy(vp + (DM1_VIEWPORT_FLOOR_Y + y) * stride,
                   floor_pixels + y * floor_width,
                   DM1_VIEWPORT_WIDTH);
        }
    }

    state->floor_ceiling_dirty = false;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_copy_and_flip_h
 *
 * Source: DUNVIEW.C F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal (line 3018)
 *   Amiga (A20E) path:
 *     F0007_MAIN_CopyBytes(src, dst, byteWidth * height);
 *     F0130_VIDEO_FlipHorizontal(dst, byteWidth, height);
 *   PC34 (I34E) path:
 *     F0655_CopyBitmapAndFlip(src, dst, MASK0x0001_FLIP_HORIZONTAL);
 *
 * Creates a horizontally mirrored copy. For each row, pixels are
 * reversed left-to-right.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_copy_and_flip_h(const uint8_t *src, uint8_t *dst,
                                     int width, int height)
{
    if (width <= 0 || height <= 0 || !src || !dst) return;

    for (int y = 0; y < height; y++) {
        const uint8_t *src_row = src + y * width;
        uint8_t *dst_row = dst + y * width;
        for (int x = 0; x < width; x++) {
            dst_row[x] = src_row[width - 1 - x];
        }
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_wall
 *
 * Source: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap (line 3048)
 *   Amiga (A20E):
 *     if (frame[C4_BYTE_WIDTH]) {
 *       F0132_VIDEO_Blit(bitmap, viewport, frame, frame[C6_X], frame[C7_Y],
 *                        frame[C4_BYTE_WIDTH], C112_BYTE_WIDTH_VIEWPORT,
 *                        C10_COLOR_FLESH, frame[C5_HEIGHT], C136_HEIGHT_VIEWPORT);
 *     }
 *   Blits with transparency (skips pixels matching color 10).
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_wall(DM1_Viewport3DState *state,
                               const uint8_t *wall_bitmap,
                               const DM1_WallFrame *frame)
{
    if (!state || !frame || frame->byte_width == 0 || frame->height == 0 || !wall_bitmap) return;

    DM1_ViewportBlitClipGate gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    if (!gate.visible) return;

    uint8_t *vp = state->viewport_pixels;
    int vp_stride = state->viewport_stride;
    int bw = frame->byte_width;

    for (int y = 0; y < gate.height; y++) {
        const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw + gate.src_x;
        uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride + gate.dst_x;

        for (int x = 0; x < gate.width; x++) {
            uint8_t pixel = src_row[x];
            if (pixel != COLOR_TRANSPARENT) {
                dst_row[x] = pixel;
            }
        }
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_wall_opaque
 *
 * Source: DUNVIEW.C F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency (line 3065)
 *   CHANGE7_15_OPTIMIZATION: no transparency check for center walls.
 *   Amiga (A20E):
 *     F0132_VIDEO_Blit(..., CM1_COLOR_NO_TRANSPARENCY, ...);
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_wall_opaque(DM1_Viewport3DState *state,
                                      const uint8_t *wall_bitmap,
                                      const DM1_WallFrame *frame)
{
    if (!state || !frame || frame->byte_width == 0 || frame->height == 0 || !wall_bitmap) return;

    DM1_ViewportBlitClipGate gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    if (!gate.visible) return;

    uint8_t *vp = state->viewport_pixels;
    int vp_stride = state->viewport_stride;
    int bw = frame->byte_width;

    for (int y = 0; y < gate.height; y++) {
        const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw + gate.src_x;
        uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride + gate.dst_x;
        memcpy(dst_row, src_row, (size_t)gate.width);
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_door
 *
 * Source: DUNVIEW.C F0102_DUNGEONVIEW_DrawDoorBitmap (line 3082)
 *   Draws from G0074_puc_Bitmap_Temporary (our state->temp_bitmap).
 *   Amiga (A20E):
 *     F0132_VIDEO_Blit(G0074_puc_Bitmap_Temporary, viewport, frame, ...
 *                      C10_COLOR_FLESH, ...);
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_door(DM1_Viewport3DState *state,
                               const DM1_WallFrame *frame)
{
    if (!state || !state->temp_bitmap) return;
    /* F0102 passes F0128's G0074 through the same C10 route as F0104. */
    dm1_viewport_3d_draw_floor_pit_or_stairs_bitmap(
        state, state->temp_bitmap, frame);
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_door_frame_flipped
 *
 * Source: DUNVIEW.C F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally (line 3096)
 *   1. F0130_VIDEO_FlipHorizontal(bitmap, frame[C4_BYTE_WIDTH], frame[C5_HEIGHT])
 *   2. F0132_VIDEO_Blit(bitmap, viewport, frame, ..., C10_COLOR_FLESH, ...)
 *
 * Flips the source bitmap in-place, then blits with transparency.
 * Note: modifies the source bitmap (same as original).
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_door_frame_flipped(DM1_Viewport3DState *state,
                                             const uint8_t *frame_bitmap,
                                             const DM1_WallFrame *frame)
{
    /* F0103's post-flip C10 blit has the same source-owned scratch contract
     * as F0105. Source: DUNVIEW.C:3096-3108, 3185-3204. */
    dm1_viewport_3d_draw_floor_pit_or_stairs_bitmap_flipped(
        state, frame_bitmap, frame);
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_wall_parity_mirrored
 *
 * Source: DUNVIEW.C:8016-8038 (F0125 D0L), 8126-8139 (F0126 D0R),
 *         6849-6858 (F0678 D2L2), 6880-6889 (F0679 D2R2),
 *         6240-6264 (F0676 D3L2), 6304-6331 (F0677 D3R2),
 *         3185-3204 (F0105) + 3018-3045 (F0099).
 * The parity side-wall route selects the opposite lane's native bitmap
 * (G2107_WallSet[C00/C01 swap]) and mirrors it horizontally into the
 * caller's own zone.  G0074 is NOT admitted on this route, so the mirror
 * is produced by sampling the source columns in reverse through the same
 * C10-transparent clip route as F0104 — pixel-identical to the
 * F0099 -> G0074 -> F0104 chain (flipped scratch column src_x + x holds
 * source column byteWidth - 1 - (src_x + x)) with no scratch span.
 * ──────────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_wall_parity_mirrored(DM1_Viewport3DState *state,
                                               const uint8_t *wall_bitmap,
                                               const DM1_WallFrame *frame)
{
    if (!state || !frame || frame->byte_width == 0 || frame->height == 0 ||
        !wall_bitmap) {
        return;
    }

    {
        int bw = frame->byte_width;
        DM1_ViewportBlitClipGate gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(
            frame, bw, frame->height);
        if (gate.visible) {
            uint8_t *vp = state->viewport_pixels;
            int vp_stride = state->viewport_stride;
            int x, y;
            for (y = 0; y < gate.height; ++y) {
                const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw;
                uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride + gate.dst_x;
                for (x = 0; x < gate.width; ++x) {
                    uint8_t pixel = src_row[bw - 1 - (gate.src_x + x)];
                    if (pixel != COLOR_TRANSPARENT) {
                        dst_row[x] = pixel;
                    }
                }
            }
        }
    }
}

void dm1_viewport_3d_draw_floor_pit_or_stairs_bitmap(
    DM1_Viewport3DState *state,
    const uint8_t *bitmap,
    const DM1_WallFrame *frame)
{
    /* ReDMCSB F0104 is a C10-transparent bitmap-to-viewport blit. Its
     * caller owns graphic selection and G0163 destination geometry. */
    dm1_viewport_3d_draw_wall(state, bitmap, frame);
}

void dm1_viewport_3d_draw_floor_pit_or_stairs_bitmap_flipped(
    DM1_Viewport3DState *state,
    const uint8_t *bitmap,
    const DM1_WallFrame *frame)
{
    if (!state || !frame || frame->byte_width == 0 || frame->height == 0 ||
        !bitmap) {
        return;
    }

    int bw = frame->byte_width;
    int bh = frame->height;
    size_t needed = (size_t)bw * (size_t)bh;

    /* F0128 allocates and owns G0074_puc_Bitmap_Temporary before dispatching
     * F0103/F0105. A host allocation here can make an unadmitted viewport
     * look valid, so a missing or undersized source scratch span is a strict
     * no-draw. The parity side-wall route does not pass through this helper;
     * it mirrors through dm1_viewport_3d_draw_wall_parity_mirrored instead.
     * Source: DUNVIEW.C:8318-8335, 3096-3108, 3185-3204. */
    if (!state->temp_bitmap || state->temp_bitmap_size < (int)needed) return;

    /* DUNVIEW.C:3197-3204 F0105: F0099 -> G0074, then F0104. */
    dm1_viewport_3d_copy_and_flip_h(bitmap, state->temp_bitmap, bw, bh);
    dm1_viewport_3d_draw_floor_pit_or_stairs_bitmap(
        state, state->temp_bitmap, frame);
}

int dm1_v1_viewport_draw_door_button_pc34(uint8_t *dst,
                                          int dst_width,
                                          int dst_height,
                                          int dst_stride,
                                          int door_button_ordinal,
                                          DM1_ViewDoorButtonIndex view_index,
                                          const DM1_DoorButtonBitmapSpan *spans,
                                          size_t span_count)
{
    if (!dst || dst_width <= 0 || dst_height <= 0 || dst_stride < dst_width) return 0;
    if (door_button_ordinal <= 0) return 0;
    if (view_index < 0 || view_index >= DM1_VIEW_DOOR_BUTTON_COUNT) return 0;

    /* ReDMCSB F0110 lines 4159-4163 converts the ordinal to a zero-based
     * door-button index, then selects G0208[coordinateSet][viewIndex]. */
    size_t span_index = (size_t)(door_button_ordinal - 1) *
                        (size_t)DM1_VIEW_DOOR_BUTTON_COUNT +
                        (size_t)view_index;
    if (!spans || span_index >= span_count) return 0;

    const DM1_DoorButtonBitmapSpan *span = &spans[span_index];
    const DM1_WallFrame *frame = &span->frame;
    if (!span->pixels || span->source_width <= 0 || span->source_height <= 0) return 0;

    int src_x = frame->blit_x;
    int src_y = frame->blit_y;
    int dst_x = frame->left_x;
    int dst_y = frame->top_y;
    int width = (int)frame->right_x - (int)frame->left_x + 1;
    int height = (int)frame->bottom_y - (int)frame->top_y + 1;
    if (width <= 0 || height <= 0) return 0;

    if (dst_x < 0) { src_x -= dst_x; width += dst_x; dst_x = 0; }
    if (dst_y < 0) { src_y -= dst_y; height += dst_y; dst_y = 0; }
    if (src_x < 0) { dst_x -= src_x; width += src_x; src_x = 0; }
    if (src_y < 0) { dst_y -= src_y; height += src_y; src_y = 0; }
    if (dst_x + width > dst_width) width = dst_width - dst_x;
    if (dst_y + height > dst_height) height = dst_height - dst_y;
    if (src_x + width > span->source_width) width = span->source_width - src_x;
    if (src_y + height > span->source_height) height = span->source_height - src_y;
    if (width <= 0 || height <= 0) return 0;

    int written = 0;
    for (int y = 0; y < height; ++y) {
        const uint8_t *src_row = span->pixels + (src_y + y) * span->source_width + src_x;
        uint8_t *dst_row = dst + (dst_y + y) * dst_stride + dst_x;
        for (int x = 0; x < width; ++x) {
            uint8_t pixel = src_row[x];
            /* ReDMCSB F0110 lines 4204-4207 delegates to F0132 with
             * C10_COLOR_FLESH, so source pixel 10 is transparent. */
            if (pixel != COLOR_TRANSPARENT) {
                dst_row[x] = pixel;
                ++written;
            }
        }
    }
    return written;
}

const DM1_WallFrame *dm1_v1_viewport_get_door_button_frame_pc34(int door_button_ordinal,
                                                                 DM1_ViewDoorButtonIndex view_index)
{
    if (door_button_ordinal != 1) return NULL;
    if (view_index < 0 || view_index >= DM1_VIEW_DOOR_BUTTON_COUNT) return NULL;
    return &s_door_button_frames[view_index];
}

const uint8_t *dm1_v1_viewport_get_door_button_palette_remap_pc34(
    DM1_ViewDoorButtonIndex view_index)
{
    switch (view_index) {
        case DM1_VIEW_DOOR_BUTTON_D3R:
        case DM1_VIEW_DOOR_BUTTON_D3C:
            return s_door_button_d3_palette_remap;
        case DM1_VIEW_DOOR_BUTTON_D2C:
            return s_door_button_d2_palette_remap;
        case DM1_VIEW_DOOR_BUTTON_D1C:
            return NULL;
        default:
            return NULL;
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_frame
 *
 * Source: DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF (line 8318)
 *
 * Main rendering entry point. The draw order is:
 *   1. Check if floor/ceiling needs redraw → F0098
 *   2. Allocate temp bitmap (largest = 160×111)
 *   3. Compute parity: G0076 = (mapX + mapY + direction) & 1
 *   4. If parity: flip floor, swap to flipped wall set
 *   5. Draw D4L, D4R, D4C far background objects
 *   6. Draw D3L2, D3R2 far-side PC34/I34E wall lanes
 *   7. Draw D3L → D3R → D3C (depth 3, all lanes)
 *   8. Draw D2L2, D2R2 (if applicable)
 *   9. Draw D2L → D2R → D2C (depth 2)
 *  10. Draw D1L → D1R → D1C (depth 1)
 *  11. Draw D0L → D0R → D0C (depth 0 = party square)
 *  12. Restore native wall set if parity was set
 *  13. Free temp bitmap
 *  14. Call F0097 to blit viewport to screen
 *  15. Anticipate next frame: draw floor/ceiling
 *
 * Each DrawSquare function (F0116-F0127):
 *   1. F0172_DUNGEON_SetSquareAspect → gets element type, ornaments, etc.
 *   2. switch (element):
 *      - WALL: draw wall bitmap, draw wall ornaments, check alcoves
 *      - CORRIDOR/PIT/TELEPORTER: draw floor ornament, items/creatures
 *      - DOOR_FRONT: draw floor ornament, door pass 1, door frame,
 *                    door bitmap, door pass 2
 *      - STAIRS_FRONT: draw stair bitmap
 *      - DOOR_SIDE/STAIRS_SIDE: draw as corridor
 *   3. If teleporter: draw field effect
 *
 * Our implementation provides the structural framework; actual square
 * aspect queries depend on the dungeon data module.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_frame(DM1_Viewport3DState *state,
                                int direction, int map_x, int map_y)
{
    /* Store party state for coordinate transforms */
    state->party_direction = (int16_t)direction;
    state->party_map_x = (int16_t)map_x;
    state->party_map_y = (int16_t)map_y;

    /* Step 1: Draw floor and ceiling if dirty.
     * ReDMCSB F0128 line 8340:
     *   if (G0297_B_DrawFloorAndCeilingRequested)
     *     F0098_DUNGEONVIEW_DrawFloorAndCeiling(); */
    if (state->floor_ceiling_dirty) {
        dm1_viewport_3d_draw_floor_ceiling(state);
    }

    /* Step 2: Compute parity flip.
     * ReDMCSB F0128 line 8357:
     *   G0076 = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001 */
    state->parity_flip = ((map_x + map_y + direction) & 1) != 0;

    /* Step 3: PC34/I34E parity is applied per draw call.
     * ReDMCSB DUNVIEW.C lines 6254-6260, 6321-6327, 6421-6427, etc.:
     *   side walls select the opposite G2107_WallSet[] entry and call F0105
     *   to flip horizontally; center walls pass G0076 to F0792.  Keep the
     *   native G2107 order stable in state->wall_set and use
     *   dm1_viewport_3d_select_wall_bitmap() at integration points. */

    /* Steps 4-11: Draw all visible squares back-to-front.
     *
     * ReDMCSB F0128 lines 8435-8542:
     *   For each square position, compute map coordinates using
     *   F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement,
     *   then call the appropriate DrawSquare function.
     *
     * The draw order ensures correct depth occlusion:
     *   Farthest squares drawn first, nearest last.
     *   Within each depth: left, right, then center.
     *
     * Each DrawSquare function queries the dungeon map for the
     * square type at the computed coordinates, then draws the
     * appropriate wall/door/stairs/floor artwork.
     *
     * Our structural framework calls the wall drawing primitives
     * for each depth position. The actual square type queries
     * require integration with the dungeon data module
     * (dm1_v1_dungeon_square_structs_pc34_compat). */

    /* ------------------------------------------------------------------
     * Door frame draw dispatch -- ReDMCSB DUNVIEW.C per-square F0104/F0105.
     *
     * Pattern (e.g. D3C, DUNVIEW.C:6725-6739 MEDIA720_I34E):
     *   LEFT  frame: F0104(G2119, C722) -> dm1_viewport_3d_draw_wall()
     *   RIGHT frame: F0105(G2119, C723) -> dm1_viewport_3d_draw_door_frame_flipped()
     *
     * F0104 draws the native bitmap; F0105 copies the same bitmap to the
     * temp buffer, flips it horizontally (dm1_viewport_3d_copy_and_flip_h),
     * then blits with transparency -- horizontal mirror for the right side.
     *
     * G21xx values from ReDMCSB DEFS.H:4040-4057 (I34E media set):
     *   G2110 = DoorFrameTopD1R   (D1L/D1R shared top bar)
     *   G2112 = DoorFrameTopD1LCR (D1C top bar)
     *   G2113 = DoorFrameTopD2R   (D2R top bar)
     *   G2114 = DoorFrameTopD2L   (D2L top bar)
     *   G2115 = DoorFrameTopD2LCR (D2C top bar, shared with D2L/D2R)
     *   G2117 = DoorFrameLeftD1C  (D1C left frame)
     *   G2118 = DoorFrameLeftD2C  (D2C left frame)
     *   G2119 = DoorFrameLeftD3C  (D3C frame + D3R right = G2119 mirrored)
     *   G2120 = DoorFrameLeftD3L  (D3L left frame + D3R right = G2120 mirrored)
     *
     * Zone constants (DEFS.H:4076-4093, MEDIA720):
     *   C718=ZONE_DOOR_FRAME_LEFT_D3L  C719=ZONE_DOOR_FRAME_RIGHT_D3L
     *   C722=ZONE_DOOR_FRAME_LEFT_D3C  C723=ZONE_DOOR_FRAME_RIGHT_D3C
     *   C724=ZONE_DOOR_FRAME_LEFT_D2C  C725=ZONE_DOOR_FRAME_RIGHT_D2C
     *   C729=ZONE_DOOR_FRAME_TOP_D2L   C731=ZONE_DOOR_FRAME_TOP_D2R
     *   C726=ZONE_DOOR_FRAME_LEFT_D1C C727=ZONE_DOOR_FRAME_RIGHT_D1C
     *   C733=ZONE_DOOR_FRAME_TOP_D1C   C734=ZONE_DOOR_FRAME_TOP_D1R
     *
     * Bitmap source: state->door_frame_bitmaps[g21xx] (asset system, TBT-XXX).
     * Guard draw calls with bitmap != NULL until asset system is wired.
     * ------------------------------------------------------------------ */

    /* Wall frame bitmap base pointer -- wired by asset system (TBT-XXX).
     * NULL means assets not yet loaded; draw calls are no-ops until then.
     * Each bitmap is DM1_VIEWPORT_BYTE_WIDTH (224) bytes wide.
     * Indexed by G21xx ordinal (absolute value, 14..22 range). */
    extern const uint8_t *g_dm1_wall_frame_bitmaps;
    const uint8_t *bm_base = g_dm1_wall_frame_bitmaps;
    const int BMP_STRIDE = DM1_VIEWPORT_BYTE_WIDTH; /* 224 bytes/row */

    /* -- Depth 3 door frames -- */

    /* F0116/F0117 own the complete D3L/D3R element switch.  In particular,
     * their door frames are only drawn by C17_ELEMENT_DOOR_FRONT, not before
     * F0172 has identified the square. */
    {
        int16_t d3l_x = 0, d3l_y = 0, d3r_x = 0, d3r_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 3, -1, map_x, map_y,
                                                 &d3l_x, &d3l_y);
        dm1_viewport_3d_resolve_relative_map_xy(direction, 3, 1, map_x, map_y,
                                                 &d3r_x, &d3r_y);
        dm1_viewport_3d_draw_d3_side_square(state, DM1_VIEW_SQUARE_D3L,
                                             d3l_x, d3l_y);
        dm1_viewport_3d_draw_d3_side_square(state, DM1_VIEW_SQUARE_D3R,
                                             d3r_x, d3r_y);
    }

    /* D3C -- center square at depth 3.
     * DUNVIEW.C:6707-6714 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:6725-6739 door-front case draws left + right pair from G2119. */
    {
        int16_t d3c_x = 0, d3c_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 3, 0, map_x, map_y,
                                                 &d3c_x, &d3c_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D3C, 3, 0);
        if (!dm1_viewport_3d_draw_center_wall_element(
                state, DM1_VIEW_SQUARE_D3C, (int)d3c_x, (int)d3c_y)) {
            const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D3C);
            if (fr && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 19 * BMP_STRIDE, fr);
                dm1_viewport_3d_draw_door_frame_flipped(state, bm_base + 19 * BMP_STRIDE, fr);
            }
        }
    }

    /* -- Depth 2 door frames -- */

    /* D2L -- side square at depth 2 left.
     * DUNVIEW.C:6954-6964 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:6991-6998 door-front case draws top frame via G2114. */
    {
        int16_t d2l_x = 0, d2l_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 2, -1, map_x, map_y,
                                                 &d2l_x, &d2l_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D2L, 2, -1);
        if (!dm1_viewport_3d_draw_side_wall_element(
                state, DM1_VIEW_SQUARE_D2L, (int)d2l_x, (int)d2l_y)) {
            const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2L);
            if (fr && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 18 * BMP_STRIDE, fr);
            }
        }
    }

    /* D2R -- side square at depth 2 right.
     * DUNVIEW.C:7105-7115 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:7184-7191 door-front case draws top frame via G2113. */
    {
        int16_t d2r_x = 0, d2r_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 2, 1, map_x, map_y,
                                                 &d2r_x, &d2r_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D2R, 2, 1);
        if (!dm1_viewport_3d_draw_side_wall_element(
                state, DM1_VIEW_SQUARE_D2R, (int)d2r_x, (int)d2r_y)) {
            const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2R);
            if (fr && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 17 * BMP_STRIDE, fr);
            }
        }
    }

    /* D2C -- center square at depth 2.
     * DUNVIEW.C:7299-7306 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:7317-7333 door-front case draws top bar (G2115) + left/right (G2118). */
    {
        int16_t d2c_x = 0, d2c_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 2, 0, map_x, map_y,
                                                 &d2c_x, &d2c_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D2C, 2, 0);
        if (!dm1_viewport_3d_draw_center_wall_element(
                state, DM1_VIEW_SQUARE_D2C, (int)d2c_x, (int)d2c_y)) {
            const DM1_WallFrame *fr_top  = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2C);
            const DM1_WallFrame *fr_side = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2L);
            if (fr_top && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 19 * BMP_STRIDE, fr_top);
            }
            if (fr_side && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 22 * BMP_STRIDE, fr_side);
                dm1_viewport_3d_draw_door_frame_flipped(state, bm_base + 22 * BMP_STRIDE, fr_side);
            }
        }
    }

    /* -- Depth 1 door frames -- */

    /* D1L -- side square at depth 1 left.
     * DUNVIEW.C:7445-7455 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:7496-7504 door-front case draws top frame via G2110. */
    {
        int16_t d1l_x = 0, d1l_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 1, -1, map_x, map_y,
                                                 &d1l_x, &d1l_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D1L, 1, -1);
        if (!dm1_viewport_3d_draw_side_wall_element(
                state, DM1_VIEW_SQUARE_D1L, (int)d1l_x, (int)d1l_y)) {
            const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1L);
            if (fr && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 14 * BMP_STRIDE, fr);
            }
        }
    }

    /* D1R -- side square at depth 1 right.
     * DUNVIEW.C:7613-7623 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:7664-7672 door-front case draws top frame via G2110. */
    {
        int16_t d1r_x = 0, d1r_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 1, 1, map_x, map_y,
                                                 &d1r_x, &d1r_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D1R, 1, 1);
        if (!dm1_viewport_3d_draw_side_wall_element(
                state, DM1_VIEW_SQUARE_D1R, (int)d1r_x, (int)d1r_y)) {
            const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1R);
            if (fr && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 14 * BMP_STRIDE, fr);
            }
        }
    }

    /* D1C -- center square at depth 1.
     * DUNVIEW.C:7833-7840 wall case draws wall + F0107 ornament and returns.
     * DUNVIEW.C:7877-7902 door-front case draws top bar (G2112) + left/right (G2117). */
    {
        int16_t d1c_x = 0, d1c_y = 0;
        dm1_viewport_3d_resolve_relative_map_xy(direction, 1, 0, map_x, map_y,
                                                 &d1c_x, &d1c_y);
        dm1_viewport_3d_notify_pre_square_draw(
            state, DM1_VIEW_SQUARE_D1C, 1, 0);
        if (!dm1_viewport_3d_draw_center_wall_element(
                state, DM1_VIEW_SQUARE_D1C, (int)d1c_x, (int)d1c_y)) {
            const DM1_WallFrame *fr_top   = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1C);
            const DM1_WallFrame *fr_side = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1L);
            if (fr_top && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 16 * BMP_STRIDE, fr_top);
            }
            if (fr_side && bm_base) {
                dm1_viewport_3d_draw_wall(state, bm_base + 21 * BMP_STRIDE, fr_side);
                dm1_viewport_3d_draw_door_frame_flipped(state, bm_base + 21 * BMP_STRIDE, fr_side);
            }
        }
    }


    /*
     * Wall draw loop -- regular (non-door) wall panels
     * ReDMCSB DUNVIEW.C F0128: each DrawSquare function (F0116-F0127)
     * queries the dungeon element at the computed map coordinate and
     * branches on C00_ELEMENT_WALL to call F0100/F0104 (native) or
     * F0105 (flipped).  The parity flag (state->parity_flip) selects
     * which G2107_WallSet[] entry is used.
     *
     * Source citations (DUNVIEW.C):
     *   D3L2: 6254-6260  D3R2: 6321-6327  D3L:  6421-6427
     *   D3R:  6554-6564  D3C:  6707-6714  D2L2: 6849-6858
     *   D2R2: 6880-6889  D2L:  6954-6964  D2R:  7105-7115
     *   D2C:  7299-7306  D1L:  7445-7455  D1R:  7613-7623
     *   D1C:  7833-7840  D0L:  8016-8033  D0R:  8126-8139
     *
     * The door frame section (steps 4-11) already covers D3L/D3R/D3C/
     * D2L/D2R/D2C/D1L/D1R/D1C.  This loop handles the remaining positions
     * that have no door frame: D3L2, D3R2, D2L2, D2R2, D0L, D0R, D0C.
     * Skip D4L/D4R/D4C (object-only) and the door-covered squares.
     */

    for (size_t i = 0; i < dm1_viewport_3d_draw_order_count(); ++i) {
        const DM1_ViewportDrawStep *step = &s_draw_order[i];

        /* Skip object-only positions */
        if (step->square == DM1_VIEW_SQUARE_D4L ||
            step->square == DM1_VIEW_SQUARE_D4R ||
            step->square == DM1_VIEW_SQUARE_D4C) {
            continue;
        }

        /* Skip squares already handled by the door frame section above.
         * Door frame covers: D3L, D3R, D3C, D2L, D2R, D2C, D1L, D1R, D1C */
        switch (step->square) {
        case DM1_VIEW_SQUARE_D3L:  case DM1_VIEW_SQUARE_D3R:
        case DM1_VIEW_SQUARE_D3C:  case DM1_VIEW_SQUARE_D2L:
        case DM1_VIEW_SQUARE_D2R:  case DM1_VIEW_SQUARE_D2C:
        case DM1_VIEW_SQUARE_D1L:  case DM1_VIEW_SQUARE_D1R:
        case DM1_VIEW_SQUARE_D1C:
            continue;
        default:
            break;
        }

        const DM1_ViewportWallDrawSpec *spec =
            dm1_viewport_3d_get_wall_draw_spec_for_square(step->square);
        if (!spec) continue;

        dm1_viewport_3d_notify_pre_square_draw(
            state, step->square, step->rel_depth, step->rel_lateral);
        if (!bm_base) continue;

        /* ── CSB-specific squares (D3L2, D3R2, D2L2, D2R2) ──────────────────
         * These squares use element-specific routing via F0676/F0677/F0678/F0679.
         * The dungeon element is looked up at the computed map position, and
         * the appropriate draw function is called for the element type
         * (wall, door, stairs, pit, teleporter, corridor).
         *
         * For D3L2/D3R2: dm1_viewport_3d_draw_csb_back_wall handles all
         * element types (WALL, TELEPORTER, STAIRS_FRONT, PIT, CORRIDOR,
         * DOOR_SIDE, DOOR_FRONT, STAIRS_SIDE).
         *
         * For D2L2/D2R2: dm1_viewport_3d_draw_csb_near_wall handles
         * WALL and TELEPORTER only (no stairs, pits, floor ornaments,
         * creatures, items, or projectiles in these squares).
         *
         * For WALL elements, the CSB functions perform parity-aware bitmap
         * selection matching F0676/F0677/F0678/F0679.
         *
         * Source: ReDMCSB DUNVIEW.C:6226-6353 F0676/F0677 (D3L2/D3R2);
         *   6837-6896 F0678/F0679 (D2L2/D2R2)
         * ─────────────────────────────────────────────────────────────────── */
        if (step->square == DM1_VIEW_SQUARE_D3L2 ||
            step->square == DM1_VIEW_SQUARE_D3R2) {
            /* Resolve map coords for D3L2/D3R2 via F0150 */
            int16_t sq_map_x = 0, sq_map_y = 0;
            dm1_viewport_3d_resolve_relative_map_xy(
                state->party_direction,
                step->rel_depth,
                step->rel_lateral,
                state->party_map_x,
                state->party_map_y,
                &sq_map_x,
                &sq_map_y);
            dm1_viewport_3d_draw_csb_back_wall(
                state, step->square,
                state->party_direction,
                (int)sq_map_x, (int)sq_map_y);
            continue; /* CSB function handles full element routing */
        }
        if (step->square == DM1_VIEW_SQUARE_D2L2 ||
            step->square == DM1_VIEW_SQUARE_D2R2) {
            /* Resolve map coords for D2L2/D2R2 via F0150 */
            int16_t sq_map_x = 0, sq_map_y = 0;
            dm1_viewport_3d_resolve_relative_map_xy(
                state->party_direction,
                step->rel_depth,
                step->rel_lateral,
                state->party_map_x,
                state->party_map_y,
                &sq_map_x,
                &sq_map_y);
            dm1_viewport_3d_draw_csb_near_wall(
                state, step->square,
                state->party_direction,
                (int)sq_map_x, (int)sq_map_y);
            continue; /* CSB function handles full element routing */
        }

        /* Parity-driven bitmap selection.
         * DUNVIEW.C:6354-6365, 6492-6503, 6625-6636, 6767-6778,
         * 7360-7371, 7505-7516, 7673-7684, 7900-7911,
         * 8039-8050, 8145-8156 */
        bool flip_h = false;
        DM1_WallSetIndex wall_idx = dm1_viewport_3d_select_wall_bitmap(
            spec, state->parity_flip, &flip_h);

        /* Compute bitmap pointer from wall_idx.
         * G2107_WallSet[wall_idx] is negative (e.g. -3 for D3L2),
         * bm_base is the base of the wall bitmap array in G2107 ordinal order.
         * state->wall_set[] / state->wall_set_flipped[] hold these indices.
         * For D0L/D0R the wall set entry already encodes the correct bitmap
         * offset; for other positions we use the G2107 entry directly.
         *
         * D0L/D0R don't use G2107 door frame bitmaps -- they use the regular
         * wall set, with D0L using C0x0002 order (DUNVIEW.C:8005) and D0R
         * using C0x0001 (DUNVIEW.C:8115).  They use wall_set_flipped/native
         * per the parity flag (DUNVIEW.C:8016-8038 D0L, 8126-8139 D0R).
         *
         * Reference: ReDMCSB DEFS.H G2107 zone (MEDIA720_I34E):
         *   D0R=-17, D0L=-16, D1R=-15, D1L=-14, D1C=-13,
         *   D2R2=-9, D2L2=-8, D2R=-12, D2L=-11, D2C=-10,
         *   D3R2=-4, D3L2=-3, D3R=-7, D3L=-6, D3C=-5
         */
        const uint8_t *wall_bmp = NULL;
        if (step->square == DM1_VIEW_SQUARE_D0L || step->square == DM1_VIEW_SQUARE_D0R) {
            /* D0L/D0R: wall_idx from select_wall_bitmap() already encodes
             * the native or opposite G2107 entry.  F0105 handles the
             * horizontal flip, so the selected native bitmap is used here.
             * ReDMCSB DUNVIEW.C:8016-8033 (D0L), 8126-8139 (D0R). */
            wall_bmp = dm1_viewport_3d_selected_wall_bitmap(state, bm_base, wall_idx);
        } else {
            /* D3L2/D3R2/D2L2/D2R2: use the selected G2107-derived bitmap
             * offset.  Parity has already swapped the selected WallSetIndex;
             * F0105 handles any horizontal flip. */
            wall_bmp = dm1_viewport_3d_selected_wall_bitmap(state, bm_base, wall_idx);
        }

        const DM1_WallFrame *fr = dm1_viewport_3d_get_wall_frame(step->square);
        if (!fr || !wall_bmp) continue;

        /* F0105 (flipped) -> copy+flip to temp then blit
         * F0100/F0104 (native) -> direct blit */
        if (flip_h) {
            dm1_viewport_3d_draw_wall_parity_mirrored(state, wall_bmp, fr);
        } else {
            dm1_viewport_3d_draw_wall(state, wall_bmp, fr);
        }
    }

    /* Step 12: Native wall set remains stable on PC34/I34E.
     * Step 13: Mark floor/ceiling dirty for next frame.
     * ReDMCSB F0128 lines 8607-8609 */
    state->floor_ceiling_dirty = true;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_present
 *
 * Source: DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport (platform-specific)
 *         VIEWPORT.C F0565_VIEWPORT_SetPalette (line 33)
 *         VIEWPORT.C F0566_VIEWPORT_BlitToScreen (line 56)
 *
 * Amiga (A20E) path:
 *   1. F0510_AMIGA_WaitBottomOfViewPort — wait for safe blit region
 *   2. Handle palette switching (inventory vs dungeon palette)
 *   3. F0565: WaitBlit, OwnBlitter, Forbid, wait vblank
 *   4. F0508: Build copper list for palette switch at scan line
 *   5. F0566: Blitter copy 4 bitplanes, 224×136 → screen at line 33
 *      custom.bltcon0 = SRCA|DEST|A_TO_D
 *      custom.bltamod = 0 (source is exactly viewport width)
 *      custom.bltdmod = (320-224)/8 = 12 (skip right margin)
 *      custom.bltsize = M092_BLITSIZE(224/16, 136)
 *   6. DisownBlitter, Permit
 *
 * PC34 implementation:
 *   Copy 224×136 viewport buffer into screen buffer at (0, 33),
 *   matching the original's viewport-to-screen placement.
 * ──────────────────────────────────────────────────────────────────────── */
bool dm1_viewport_3d_present_pc34(const uint8_t *viewport_pixels,
                                  int viewport_stride,
                                  uint8_t *screen_pixels,
                                  int screen_width,
                                  int screen_height,
                                  int screen_stride,
                                  int palette_switching,
                                  int dungeon_palette_index,
                                  int party_map_index,
                                  int entrance_map_index,
                                  int mouse_x,
                                  int mouse_y,
                                  int *cached_palette_index,
                                  DM1_ViewportPresentReceiptPc34 *out_receipt)
{
    DM1_ViewportPresentReceiptPc34 receipt;
    int y;

    memset(&receipt, 0, sizeof(receipt));
    receipt.palette_switch_request = (int16_t)palette_switching;
    receipt.dungeon_palette_index = (int16_t)dungeon_palette_index;
    receipt.cached_palette_index = cached_palette_index
        ? (int16_t)*cached_palette_index : -1;
    receipt.source_x = 0;
    receipt.source_y = 0;
    receipt.destination_x = DM1_VIEWPORT_SCREEN_X;
    receipt.destination_y = DM1_VIEWPORT_SCREEN_Y;
    receipt.width = DM1_VIEWPORT_WIDTH;
    receipt.height = DM1_VIEWPORT_HEIGHT;

    if (!viewport_pixels || !screen_pixels || !cached_palette_index ||
        viewport_stride < DM1_VIEWPORT_WIDTH ||
        screen_width < DM1_VIEWPORT_SCREEN_WIDTH ||
        screen_height < DM1_VIEWPORT_SCREEN_HEIGHT ||
        screen_stride < screen_width) {
        if (out_receipt) *out_receipt = receipt;
        return false;
    }

    /* DRAWVIEW.C:827-833. The PC path hides the pointer only when it is
     * outside C007, then restores it after VIDRV_09_BlitViewPort. */
    if (mouse_y > 168 || mouse_x > 223 || mouse_y < 15) {
        receipt.mouse_hidden = true;
    } else {
        receipt.mouse_screen_update_enabled = true;
        receipt.mouse_screen_update_disabled = true;
    }

    /* DRAWVIEW.C:835-848. This records the exact source palette selection;
     * actual RGB6 bytes are owned by the host's verified palette consumer. */
    if (palette_switching == 1) {
        if (dungeon_palette_index != *cached_palette_index) {
            receipt.palette_changed = true;
            receipt.palette_action = DM1_VIEWPORT_PRESENT_PALETTE_DUNGEON_PC34;
            *cached_palette_index = dungeon_palette_index;
        }
    } else if (palette_switching == 0) {
        receipt.palette_changed = true;
        receipt.palette_action = party_map_index != entrance_map_index
            ? DM1_VIEWPORT_PRESENT_PALETTE_INVENTORY_PC34
            : DM1_VIEWPORT_PRESENT_PALETTE_LIGHT0_PC34;
        *cached_palette_index = -1;
    }
    receipt.cached_palette_index = (int16_t)*cached_palette_index;

    /* DRAWVIEW.C:878-893, VIDRV_09_BlitViewPort(G0296, C007). */
    for (y = 0; y < DM1_VIEWPORT_HEIGHT; ++y) {
        memmove(screen_pixels +
                    (DM1_VIEWPORT_SCREEN_Y + y) * screen_stride +
                    DM1_VIEWPORT_SCREEN_X,
                viewport_pixels + y * viewport_stride,
                (size_t)DM1_VIEWPORT_WIDTH);
    }
    receipt.valid = true;
    if (out_receipt) *out_receipt = receipt;
    return true;
}

void dm1_viewport_3d_present(DM1_Viewport3DState *state,
                             uint8_t *screen_pixels,
                             int screen_stride,
                             int palette_switching)
{
    int cached_palette_index;
    if (!state) return;
    cached_palette_index = state->palette_index;
    (void)dm1_viewport_3d_present_pc34(state->viewport_pixels,
                                        state->viewport_stride,
                                        screen_pixels,
                                        DM1_VIEWPORT_SCREEN_WIDTH,
                                        DM1_VIEWPORT_SCREEN_HEIGHT,
                                        screen_stride,
                                        palette_switching,
                                        state->palette_index,
                                        0,
                                        255,
                                        -1,
                                        -1,
                                        &cached_palette_index,
                                        NULL);
    state->palette_index = (int16_t)cached_palette_index;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_get_wall_frame
 *
 * Source: DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8] · G0711 · G0712
 *   Returns the frame descriptor for the given view square position.
 *   Extended to cover CSB D3L2/D3R2/D2L2/D2R2 via csb_v1_vp_get_wall_frame().
 * ──────────────────────────────────────────────────────────────────────── */
const DM1_WallFrame *dm1_viewport_3d_get_wall_frame(DM1_ViewSquareIndex square)
{
    return csb_v1_vp_get_wall_frame(square);
}

DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate(const DM1_WallFrame *frame,
                                                                      int source_width,
                                                                      int source_height)
{
    DM1_ViewportBlitClipGate gate;
    memset(&gate, 0, sizeof(gate));
    gate.source_lines = "DUNVIEW.C:3053-3058,3198-3204; COORD.C:2390-2409; IMAGE3.C:866-889";

    if (!frame || source_width <= 0 || source_height <= 0) return gate;

    int dst_x = frame->left_x;
    int dst_y = frame->top_y;
    int src_x = frame->blit_x;
    int src_y = frame->blit_y;
    int width = (int)frame->right_x - (int)frame->left_x + 1;
    int height = (int)frame->bottom_y - (int)frame->top_y + 1;

    if (width <= 0 || height <= 0) return gate;
    if (src_x >= source_width || src_y >= source_height) return gate;

    if (dst_x < 0) { src_x -= dst_x; width += dst_x; dst_x = 0; }
    if (dst_y < 0) { src_y -= dst_y; height += dst_y; dst_y = 0; }
    if (dst_x + width > DM1_VIEWPORT_WIDTH) width = DM1_VIEWPORT_WIDTH - dst_x;
    if (dst_y + height > DM1_VIEWPORT_HEIGHT) height = DM1_VIEWPORT_HEIGHT - dst_y;

    if (src_x < 0) { dst_x -= src_x; width += src_x; src_x = 0; }
    if (src_y < 0) { dst_y -= src_y; height += src_y; src_y = 0; }
    if (src_x + width > source_width) width = source_width - src_x;
    if (src_y + height > source_height) height = source_height - src_y;

    if (width <= 0 || height <= 0) return gate;

    gate.visible = true;
    gate.src_x = (int16_t)src_x;
    gate.src_y = (int16_t)src_y;
    gate.dst_x = (int16_t)dst_x;
    gate.dst_y = (int16_t)dst_y;
    gate.width = (int16_t)width;
    gate.height = (int16_t)height;
    return gate;
}

size_t dm1_viewport_3d_draw_order_count(void)
{
    return sizeof(s_draw_order) / sizeof(s_draw_order[0]);
}

const DM1_ViewportDrawStep *dm1_viewport_3d_get_draw_order_step(size_t index)
{
    if (index >= dm1_viewport_3d_draw_order_count()) return NULL;
    return &s_draw_order[index];
}

int dm1_v1_viewport_base_graphic_pc34(int layer,
                                      int* outGraphic,
                                      int* outX,
                                      int* outY,
                                      int* outW,
                                      int* outH)
{
    switch (layer) {
        case 0:
            /* ReDMCSB DUNVIEW.C F0098: ceiling bitmap C079, 224x39. */
            if (outGraphic) *outGraphic = 79;
            if (outX) *outX = 0;
            if (outY) *outY = 0;
            if (outW) *outW = 224;
            if (outH) *outH = 39;
            return 1;
        case 1:
            /* ReDMCSB DUNVIEW.C F0098: floor bitmap C078 below ceiling. */
            if (outGraphic) *outGraphic = 78;
            if (outX) *outX = 0;
            if (outY) *outY = 39;
            if (outW) *outW = 224;
            if (outH) *outH = 97;
            return 1;
        default:
            return 0;
    }
}

int dm1_v1_viewport_source_composition_order_count_pc34(void)
{
    return 16;
}

int dm1_v1_viewport_source_composition_order_step_pc34(int ordinal)
{
    static const int kCompositionOrder[] = {
        1,  /* floor/ceiling base */
        2,  /* pits */
        3,  /* floor ornaments */
        4,  /* side walls */
        5,  /* front walls */
        6,  /* wall ornaments */
        7,  /* stairs */
        8,  /* teleporter fields */
        9,  /* side doors */
        10, /* side door ornaments */
        11, /* side destroyed-door masks */
        12, /* center doors */
        13, /* center door ornaments */
        14, /* center destroyed-door masks */
        15, /* center door buttons */
        16  /* D3R door button */
    };
    if (ordinal < 0 ||
        ordinal >= (int)(sizeof(kCompositionOrder) / sizeof(kCompositionOrder[0]))) {
        return 0;
    }
    return kCompositionOrder[ordinal];
}

size_t dm1_viewport_3d_far_object_pass_spec_count(void)
{
    return sizeof(s_far_object_pass_specs) / sizeof(s_far_object_pass_specs[0]);
}

const DM1_ViewportFarObjectPassSpec *dm1_viewport_3d_get_far_object_pass_spec(size_t index)
{
    if (index >= dm1_viewport_3d_far_object_pass_spec_count()) return NULL;
    return &s_far_object_pass_specs[index];
}

const DM1_ViewportFarObjectPassSpec *dm1_viewport_3d_get_far_object_pass_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_far_object_pass_spec_count(); ++i) {
        if (s_far_object_pass_specs[i].square == square) return &s_far_object_pass_specs[i];
    }
    return NULL;
}

int dm1_viewport_3d_resolve_relative_map_xy(int direction,
                                            int rel_depth,
                                            int rel_lateral,
                                            int origin_x,
                                            int origin_y,
                                            int16_t *out_x,
                                            int16_t *out_y)
{
    int normalized;
    int forward_dx;
    int forward_dy;
    int right_dx;
    int right_dy;
    int x;
    int y;

    if (!out_x || !out_y) return 0;

    /* Source lock: ReDMCSB DUNGEON.C:1371-1421
     * F0150 applies the forward vector for P0254, then simulates a right
     * turn and applies P0255. DUNVIEW.C:8466-8542 calls F0150 for each F0128
     * visible square offset before dispatching the corresponding draw helper.
     */
    normalized = direction & 0x0003;
    switch (normalized) {
    case 0:
        forward_dx = 0;
        forward_dy = -1;
        right_dx = 1;
        right_dy = 0;
        break;
    case 1:
        forward_dx = 1;
        forward_dy = 0;
        right_dx = 0;
        right_dy = 1;
        break;
    case 2:
        forward_dx = 0;
        forward_dy = 1;
        right_dx = -1;
        right_dy = 0;
        break;
    default:
        forward_dx = -1;
        forward_dy = 0;
        right_dx = 0;
        right_dy = -1;
        break;
    }

    x = origin_x + rel_depth * forward_dx + rel_lateral * right_dx;
    y = origin_y + rel_depth * forward_dy + rel_lateral * right_dy;
    *out_x = (int16_t)x;
    *out_y = (int16_t)y;
    return 1;
}

int dm1_viewport_3d_resolve_draw_order_step(size_t index,
                                            int direction,
                                            int origin_x,
                                            int origin_y,
                                            DM1_ViewportResolvedDrawStep *out_step)
{
    const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(index);
    if (!step || !out_step) return 0;
    memset(out_step, 0, sizeof(*out_step));
    out_step->square = step->square;
    out_step->rel_depth = step->rel_depth;
    out_step->rel_lateral = step->rel_lateral;
    out_step->redmcsb_function = step->redmcsb_function;
    out_step->source_lines = "DUNGEON.C:1371-1421; DUNVIEW.C:8466-8542";
    return dm1_viewport_3d_resolve_relative_map_xy(
        direction, step->rel_depth, step->rel_lateral, origin_x, origin_y,
        &out_step->map_x, &out_step->map_y);
}

const DM1_ViewportNoWriteSpec *dm1_viewport_3d_get_d3l1_no_write_spec(void)
{
    return &s_d3l1_no_write_spec;
}


size_t dm1_viewport_3d_wall_draw_spec_count(void)
{
    return sizeof(s_wall_draw_specs) / sizeof(s_wall_draw_specs[0]);
}

const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_wall_draw_spec(size_t index)
{
    if (index >= dm1_viewport_3d_wall_draw_spec_count()) return NULL;
    return &s_wall_draw_specs[index];
}

const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_wall_draw_spec_count(); ++i) {
        if (s_wall_draw_specs[i].square == square) return &s_wall_draw_specs[i];
    }
    return NULL;
}

const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_side_wall_draw_spec_for_rel(int rel_forward,
                                                                                int rel_side)
{
    size_t i;
    for (i = 0; i < dm1_viewport_3d_wall_draw_spec_count(); ++i) {
        const DM1_ViewportWallDrawSpec *spec = &s_wall_draw_specs[i];
        if (spec->center_wall) {
            continue;
        }
        if (spec->runtime_rel_forward == rel_forward &&
            spec->runtime_rel_side == rel_side) {
            return spec;
        }
    }
    return NULL;
}

DM1_WallSetIndex dm1_viewport_3d_select_wall_bitmap(const DM1_ViewportWallDrawSpec *spec,
                                                    bool parity_flip,
                                                    bool *flip_horizontally)
{
    if (flip_horizontally) *flip_horizontally = false;
    if (!spec) return DM1_WALL_SET_COUNT;
    if (parity_flip) {
        if (flip_horizontally) *flip_horizontally = spec->parity_flips_horizontally;
        return spec->parity_wall;
    }
    return spec->native_wall;
}

bool dm1_viewport_3d_wall_occludes_floor_items(const DM1_ViewportWallDrawSpec *spec, bool front_alcove)
{
    if (!spec) return true;
    if (!front_alcove) return true;
    return !spec->front_alcove_reveals_contents;
}

uint16_t dm1_viewport_3d_wall_item_cell_order(const DM1_ViewportWallDrawSpec *spec, bool front_alcove)
{
    return dm1_viewport_3d_wall_occludes_floor_items(spec, front_alcove)
        ? 0xffffu
        : 0x0000u;
}

int dm1_v1_graphic_wallset0_index_pc34(int wall_index)
{
    if (wall_index < 0 || wall_index >= DM1_WALL_SET_COUNT) {
        return -1;
    }
    return 93 + wall_index;
}

int dm1_viewport_3d_wall_host_material_receipt_pc34(
    int map_wall_set,
    int graphic_index,
    int transparent_color,
    bool flip_horizontally,
    int expected_width,
    int expected_height,
    DM1_ViewportWallHostMaterialReceiptPc34 *out_receipt)
{
    DM1_ViewportWallHostMaterialReceiptPc34 receipt;
    if (!out_receipt || graphic_index < 0 ||
        expected_width <= 0 || expected_height <= 0) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = true;
    receipt.map_wall_set = map_wall_set;
    receipt.graphic_index = graphic_index;
    receipt.transparent_color = transparent_color;
    receipt.flip_horizontally = flip_horizontally;
    receipt.expected_width = expected_width;
    receipt.expected_height = expected_height;
    *out_receipt = receipt;
    return 1;
}

int dm1_viewport_3d_build_d3_side_wall_host_handoff_pc34(
    DM1_ViewSquareIndex square,
    bool parity_flip,
    bool wall_like,
    bool front_alcove,
    DM1_ViewportD3SideWallHostHandoffPc34 *out_handoff)
{
    const DM1_ViewportWallDrawSpec *spec;
    DM1_ViewportD3SideWallHostHandoffPc34 handoff;

    if (!out_handoff ||
        (square != DM1_VIEW_SQUARE_D3L2 && square != DM1_VIEW_SQUARE_D3R2 &&
         square != DM1_VIEW_SQUARE_D3L && square != DM1_VIEW_SQUARE_D3R)) {
        return 0;
    }
    spec = dm1_viewport_3d_get_wall_draw_spec_for_square(square);
    if (!spec) {
        return 0;
    }
    memset(&handoff, 0, sizeof(handoff));
    handoff.handled = true;
    handoff.draw_wall = wall_like;
    handoff.falls_through_to_f0115 =
        !wall_like || (front_alcove && spec->front_alcove_reveals_contents);
    handoff.selected_wall = dm1_viewport_3d_select_wall_bitmap(
        spec, parity_flip, &handoff.flip_horizontally);
    handoff.pc34_zone = spec->pc34_zone;
    handoff.dst_x = spec->runtime_dst_x;
    handoff.dst_y = spec->runtime_dst_y;
    handoff.width = spec->runtime_width;
    handoff.height = spec->runtime_height;
    handoff.transparent_color = 10;
    handoff.redmcsb_function = spec->redmcsb_function;
    handoff.source_lines = spec->source_lines;
    /* ReDMCSB DUNVIEW.C F0676:6254-6289 / F0677:6321-6356 select
     * C702/C703, and F0116:6421-6437 / F0117:6554-6573 select C705/C706.
     * All four use C10-transparent wall material, return for ordinary
     * walls, and retain their source-defined F0115 fallthrough. */
    *out_handoff = handoff;
    return 1;
}

int dm1_viewport_3d_build_side_wall_host_receipt_pc34(
    DM1_ViewSquareIndex square,
    int map_wall_set,
    bool parity_flip,
    bool wall_like,
    bool front_alcove,
    int max_visible_forward,
    const DM1_ViewportLaneVisibilityReceiptPc34 *visibility,
    DM1_ViewportSideWallHostReceiptPc34 *out_receipt)
{
    const DM1_ViewportWallDrawSpec *spec;
    DM1_ViewportSideWallHostReceiptPc34 receipt;
    DM1_WallSetIndex selected_wall;
    bool flip_horizontally = false;
    int wallset0_graphic_index;

    if (!out_receipt || !visibility) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    spec = dm1_viewport_3d_get_wall_draw_spec_for_square(square);
    /* ReDMCSB DUNVIEW.C F0128:8466-8477 has D4 F0115 calls but no D4
     * wall-zone case. F0115 exits on depth > 3 before choosing C702..C717,
     * so an absent wall spec is deliberately a no-receipt/no-draw result. */
    if (!spec || spec->center_wall) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.handled = true;
    receipt.square = square;
    receipt.pc34_zone = spec->pc34_zone;
    receipt.dst_x = spec->runtime_dst_x;
    receipt.dst_y = spec->runtime_dst_y;
    receipt.width = spec->runtime_width;
    receipt.height = spec->runtime_height;
    receipt.redmcsb_function = spec->redmcsb_function;
    receipt.source_lines = spec->source_lines;

    /* ReDMCSB DUNVIEW.C F0128:8478-8533 invokes each side square before
     * its center square. Side-wall material is not pre-culled by nearer
     * same-lane occupancy; the nearer source wall panel overpaints the farther
     * one in the same far-to-near wall pass. Lane masks remain for later
     * floor/content/effect passes, not for F0116/F0117/F0119/F0120 walls. */
    if (!wall_like || spec->runtime_rel_forward > max_visible_forward) {
        receipt.falls_through_to_f0115 =
            !wall_like || (front_alcove && spec->front_alcove_reveals_contents);
        *out_receipt = receipt;
        return 1;
    }

    selected_wall = dm1_viewport_3d_select_wall_bitmap(
        spec, parity_flip, &flip_horizontally);
    if (selected_wall >= DM1_WALL_SET_COUNT) {
        *out_receipt = receipt;
        return 1;
    }
    wallset0_graphic_index =
        dm1_v1_graphic_wallset0_index_pc34((int)selected_wall);
    if (!dm1_viewport_3d_wall_host_material_receipt_pc34(
            map_wall_set, wallset0_graphic_index, 10, flip_horizontally,
            receipt.width, receipt.height, &receipt.material)) {
        *out_receipt = receipt;
        return 1;
    }
    /* ReDMCSB DUNVIEW.C G0163 rows 585-586 describe C710/C711 as
     * 75x71 destination zones, while F0119/F0120 consume the complete
     * 78x74 Graphic558 backing bitmap. The host validates that real backing
     * surface, then crops its copy to receipt.width/height. */
    if (receipt.pc34_zone == DM1_PC34_ZONE_WALL_D2L ||
        receipt.pc34_zone == DM1_PC34_ZONE_WALL_D2R) {
        receipt.material.expected_width = 78;
        receipt.material.expected_height = 74;
    }
    receipt.draw_wall = true;
    *out_receipt = receipt;
    return 1;
}

int dm1_viewport_3d_center_door_button_host_plan_pc34(
    int depth_index,
    DM1_ViewportCenterDoorButtonHostPlanPc34 *out_plan)
{
    DM1_ViewportCenterDoorButtonHostPlanPc34 plan;
    if (!out_plan || depth_index < 0 || depth_index > 2) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.view_index = depth_index == 0 ? DM1_VIEW_DOOR_BUTTON_D1C :
        (depth_index == 1 ? DM1_VIEW_DOOR_BUTTON_D2C : DM1_VIEW_DOOR_BUTTON_D3C);
    plan.frame = dm1_v1_viewport_get_door_button_frame_pc34(1, plan.view_index);
    if (!plan.frame) return 0;
    /* ReDMCSB DUNVIEW.C F0110:4163,4204-4210: G0208 selects the view
     * frame, then F0132/F0791 blit it with C10 and D2/D3 palette changes. */
    plan.palette_remap =
        dm1_v1_viewport_get_door_button_palette_remap_pc34(plan.view_index);
    plan.transparent_color = 10;
    plan.valid = true;
    *out_plan = plan;
    return 1;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_set_wall_frame_bitmaps
 *
 * Wires wall-frame bitmaps into the V1 viewport drawing pipeline.
 * This is the asset-population step: the M11 engine calls this after
 * loading GRAPHICS.DAT wall-set bitmaps via its own asset system.
 *
 * Source: DUNVIEW.C F0096 (line 2225) · DEFS.H G2107/G2110-G2120 (I34E)
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_set_wall_frame_bitmaps(const uint8_t *bitmap)
{
    g_dm1_wall_frame_bitmaps = bitmap;
}

DM1_ViewportCellOrder dm1_viewport_3d_decode_cell_order(uint16_t order)
{
    DM1_ViewportCellOrder out;
    memset(&out, 0, sizeof(out));
    out.cell_order = order;

    uint16_t remaining = order;
    uint16_t first = remaining & 0x000f;
    if (first == 0) {
        out.alcove = true;
        return out;
    }
    if (first & 0x0008) {
        out.door_pass = (unsigned char)((first & 0x0007) + 1);
        remaining >>= 4;
    }
    for (int i = 0; i < 4; ++i) {
        uint16_t cell = remaining & 0x000f;
        if (cell == 0) break;
        out.cells[out.cell_count++] = (unsigned char)cell;
        remaining >>= 4;
    }
    return out;
}

size_t dm1_viewport_3d_thing_layer_spec_count(void)
{
    return sizeof(s_thing_layers) / sizeof(s_thing_layers[0]);
}

const DM1_ViewportThingLayerSpec *dm1_viewport_3d_get_thing_layer_spec(size_t index)
{
    if (index >= dm1_viewport_3d_thing_layer_spec_count()) return NULL;
    return &s_thing_layers[index];
}


size_t dm1_viewport_3d_projectile_occlusion_spec_count(void)
{
    return sizeof(s_projectile_occlusion_specs) / sizeof(s_projectile_occlusion_specs[0]);
}

const DM1_ViewportProjectileOcclusionSpec *dm1_viewport_3d_get_projectile_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_projectile_occlusion_spec_count()) return NULL;
    return &s_projectile_occlusion_specs[index];
}

const DM1_ViewportProjectileOcclusionSpec *dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_projectile_occlusion_spec_count(); ++i) {
        if (s_projectile_occlusion_specs[i].square == square) return &s_projectile_occlusion_specs[i];
    }
    return NULL;
}

int dm1_viewport_3d_projectile_zone_for_cell(const DM1_ViewportProjectileOcclusionSpec *spec, unsigned char view_cell)
{
    if (!spec || view_cell > 3 || spec->g2028_row < 0) return -1;
    if ((spec->view_depth == 3) && (view_cell <= 1)) return -1;
    if ((spec->view_depth == 0) && (view_cell >= 2)) return -1;
    return 2900 + ((int)spec->g2028_row * 4) + view_cell;
}

int dm1_viewport_3d_projectile_scale_index_for_cell(const DM1_ViewportProjectileOcclusionSpec *spec, unsigned char view_cell)
{
    if (!spec || view_cell > 3) return -1;
    if (dm1_viewport_3d_projectile_zone_for_cell(spec, view_cell) < 0) return -1;
    return (spec->view_depth << 1) - (view_cell >> 1);
}

bool dm1_viewport_3d_projectile_visible_after_wall_case(const DM1_ViewportWallDrawSpec *wall,
                                                        bool front_alcove)
{
    if (!wall) return true;
    if (!wall->wall_case_returns) return true;
    return front_alcove && wall->front_alcove_reveals_contents;
}

size_t dm1_viewport_3d_explosion_occlusion_spec_count(void)
{
    return sizeof(s_explosion_occlusion_specs) / sizeof(s_explosion_occlusion_specs[0]);
}

const DM1_ViewportExplosionOcclusionSpec *dm1_viewport_3d_get_explosion_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_explosion_occlusion_spec_count()) return NULL;
    return &s_explosion_occlusion_specs[index];
}

const DM1_ViewportExplosionOcclusionSpec *dm1_viewport_3d_get_explosion_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_explosion_occlusion_spec_count(); ++i) {
        if (s_explosion_occlusion_specs[i].square == square) return &s_explosion_occlusion_specs[i];
    }
    return NULL;
}

int dm1_viewport_3d_explosion_d0c_pattern_zone(const DM1_ViewportExplosionOcclusionSpec *spec)
{
    if (!spec || !spec->d0c_pattern_zone) return -1;
    return 4;
}

int dm1_viewport_3d_explosion_centered_zone(const DM1_ViewportExplosionOcclusionSpec *spec)
{
    if (!spec || spec->d0c_pattern_zone || spec->g2034_row < 0) return -1;
    return 3014 + spec->g2034_row;
}

int dm1_viewport_3d_explosion_two_cell_zone(const DM1_ViewportExplosionOcclusionSpec *spec, unsigned char front_cell)
{
    if (!spec || spec->d0c_pattern_zone || spec->g2034_row < 0 || front_cell > 1) return -1;
    return 3031 + ((int)spec->g2034_row * 2) + front_cell;
}

int dm1_viewport_3d_explosion_rebirth_step1_zone(const DM1_ViewportExplosionOcclusionSpec *spec)
{
    if (!spec || spec->rebirth_row < 0) return -1;
    return 3000 + spec->rebirth_row;
}

int dm1_viewport_3d_explosion_rebirth_step2_zone(const DM1_ViewportExplosionOcclusionSpec *spec)
{
    if (!spec || spec->d0c_pattern_zone || spec->rebirth_row < 0) return -1;
    return 3007 + spec->rebirth_row;
}

size_t dm1_viewport_3d_door_front_occlusion_spec_count(void)
{
    return sizeof(s_door_front_occlusion_specs) / sizeof(s_door_front_occlusion_specs[0]);
}

const DM1_ViewportDoorFrontOcclusionSpec *dm1_viewport_3d_get_door_front_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_door_front_occlusion_spec_count()) return NULL;
    return &s_door_front_occlusion_specs[index];
}

const DM1_ViewportDoorFrontOcclusionSpec *dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_door_front_occlusion_spec_count(); ++i) {
        if (s_door_front_occlusion_specs[i].square == square) return &s_door_front_occlusion_specs[i];
    }
    return NULL;
}

DM1_ViewportD3BackWallRuntimeReceipt dm1_viewport_3d_build_d3_back_wall_runtime_receipt(
    DM1_ViewSquareIndex square,
    int element)
{
    DM1_ViewportD3BackWallRuntimeReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.square = square;
    receipt.element = element;

    const DM1_ViewportFloorFieldOrderSpec *order =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(square);
    const DM1_ViewportDoorFrontOcclusionSpec *door =
        dm1_viewport_3d_get_door_front_occlusion_spec_for_square(square);

    if (order) {
        receipt.wall_case_returns_before_things =
            order->wall_case_returns_before_things && element == DM1_VP_ELEMENT_WALL;
        receipt.field_after_thing_passes =
            order->field_after_things && element == DM1_VP_ELEMENT_TELEPORTER;
        receipt.field_source_lines = order->field_source_lines;
    }

    if (element == DM1_VP_ELEMENT_DOOR_FRONT && door) {
        receipt.rear_cell_order = door->rear_cell_order;
        receipt.front_cell_order = door->front_cell_order;
        receipt.thing_pass_count = 2;
        receipt.floor_ornament_before_rear_pass = true;
        receipt.door_front_between_passes = true;
        receipt.rear_pass_source_lines = door->rear_pass_source_lines;
        receipt.door_source_lines = door->door_source_lines;
        receipt.front_pass_source_lines = door->front_pass_source_lines;
    } else if (order && !receipt.wall_case_returns_before_things) {
        receipt.rear_cell_order = order->cell_order;
        receipt.thing_pass_count =
            order->objects_creatures_projectiles_before_explosions ? 1 : 0;
        receipt.floor_ornament_before_rear_pass =
            order->floor_ornament_before_things;
        receipt.rear_pass_source_lines = order->things_source_lines;
    }

    return receipt;
}

static int dm1_viewport_3d_d3_back_wall_side_slot(DM1_ViewSquareIndex square)
{
    if (square == DM1_VIEW_SQUARE_D3L2) return 0;
    if (square == DM1_VIEW_SQUARE_D3R2) return 1;
    return -1;
}

static int dm1_viewport_3d_d3_door_front_temp_bytes(
    const DM1_V1_D3L2D3R2F0111DoorFrontMaterialPlanPc34 *plan)
{
    if (!plan || plan->source_stride_bytes <= 0 || plan->height <= 0) {
        return 0;
    }
    return plan->source_stride_bytes * plan->height;
}

static int dm1_viewport_3d_d3_door_front_packed_source_ready(
    const DM1_Viewport3DState *state,
    int slot,
    const DM1_V1_D3L2D3R2F0111DoorFrontMaterialPlanPc34 *plan)
{
    if (!state || !plan || slot < 0 || slot >= 2) return 0;
    return state->door_front_d3_packed_source[slot] &&
           state->door_front_d3_packed_stride_bytes[slot] >= plan->source_stride_bytes &&
           state->door_front_d3_packed_height[slot] >= plan->height;
}

static int dm1_viewport_3d_d3_door_front_temp_ready(
    const DM1_Viewport3DState *state,
    const DM1_V1_D3L2D3R2F0111DoorFrontMaterialPlanPc34 *plan)
{
    int required = dm1_viewport_3d_d3_door_front_temp_bytes(plan);
    return state && state->temp_bitmap && required > 0 &&
           state->temp_bitmap_size >= required;
}

static int dm1_viewport_3d_floor_ornament_side_for_square(
    DM1_ViewSquareIndex square);

DM1_ViewportD3BackWallRuntimeReceipt dm1_viewport_3d_build_d3_back_wall_runtime_asset_receipt(
    const DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int element)
{
    DM1_ViewportD3BackWallRuntimeReceipt receipt =
        dm1_viewport_3d_build_d3_back_wall_runtime_receipt(square, element);
    int slot = dm1_viewport_3d_d3_back_wall_side_slot(square);

    if (!state || slot < 0) return receipt;

    if (receipt.floor_ornament_before_rear_pass) {
        DM1_FloorOrnamentRenderPlanPc34 plan;
        int rel_side = dm1_viewport_3d_floor_ornament_side_for_square(square);
        receipt.floor_ornament_asset_index = state->floor_ornament_indices[slot];
        receipt.floor_ornament_asset_bound = receipt.floor_ornament_asset_index != 0;
        if (rel_side != 0 &&
            dm1_v1_floor_ornament_source_zone_pc34(3, rel_side, &plan)) {
            const uint8_t *pixels = NULL;
            int width = 0;
            int height = 0;
            receipt.floor_ornament_blit_plan_bound = true;
            receipt.floor_ornament_graphic_index =
                receipt.floor_ornament_asset_index;
            receipt.floor_ornament_dst_x = (int16_t)plan.blit.dstX;
            receipt.floor_ornament_dst_y = (int16_t)plan.blit.dstY;
            receipt.floor_ornament_width = (int16_t)plan.blit.width;
            receipt.floor_ornament_height = (int16_t)plan.blit.height;
            if (receipt.floor_ornament_asset_bound &&
                state->graphic_provider_callback &&
                state->graphic_provider_callback(
                    state->graphic_provider_user_data,
                    receipt.floor_ornament_asset_index,
                    &pixels,
                    &width,
                    &height) &&
                pixels &&
                width >= plan.blit.width &&
                height >= plan.blit.height) {
                receipt.floor_ornament_graphics_dat_bound = true;
                receipt.floor_ornament_graphics_dat_width = (int16_t)width;
                receipt.floor_ornament_graphics_dat_height = (int16_t)height;
            }
            /* F0108 has no host-colored substitute: absent original pixels
             * remain an unmaterialized source receipt and draw nothing. */
            receipt.floor_ornament_used_bounded_fallback = false;
        }
    }

    if (receipt.door_front_between_passes) {
        const DM1_V1_D3L2D3R2F0111DoorFrontSpecPc34 *spec =
            dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_for_side_pc34(
                slot == 0 ? DM1_V1_D3L2_D3R2_F0111_SIDE_D3L2_PC34
                          : DM1_V1_D3L2_D3R2_F0111_SIDE_D3R2_PC34);
        DM1_V1_D3L2D3R2F0111DoorFrontMaterialPlanPc34 plan;
        receipt.door_front_asset_index = state->door_front_d3[slot];
        receipt.door_front_asset_bound = receipt.door_front_asset_index != 0;
        if (spec &&
            dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_material_plan_pc34(
                spec, &plan)) {
            const uint8_t *pixels = NULL;
            int width = 0;
            int height = 0;
            receipt.door_front_blit_plan_bound = true;
            receipt.door_front_graphic_index = receipt.door_front_asset_index;
            receipt.door_front_zone_index = (int16_t)plan.door_zone;
            receipt.door_front_dst_x = (int16_t)plan.x;
            receipt.door_front_dst_y = (int16_t)plan.y;
            receipt.door_front_width = (int16_t)plan.width;
            receipt.door_front_height = (int16_t)plan.height;
            if (receipt.door_front_asset_bound &&
                state->graphic_provider_callback &&
                state->graphic_provider_callback(
                    state->graphic_provider_user_data,
                    receipt.door_front_asset_index,
                    &pixels,
                    &width,
                    &height) &&
                pixels &&
                width >= plan.width &&
                height >= plan.height) {
                receipt.door_front_graphics_dat_bound = true;
                receipt.door_front_graphics_dat_width = (int16_t)width;
                receipt.door_front_graphics_dat_height = (int16_t)height;
            }
            receipt.door_front_graphics_dat_packed_bound =
                dm1_viewport_3d_d3_door_front_packed_source_ready(
                    state, slot, &plan) ? true : false;
            receipt.door_front_temp_bitmap_bound =
                dm1_viewport_3d_d3_door_front_temp_ready(state, &plan)
                    ? true : false;
            /* F0111 has no indexed-color fallback: G0693 must be supplied
             * by F0489/GRAPHICS.DAT (directly or through G0074). */
            receipt.door_front_used_bounded_fallback = false;
            if (receipt.door_front_graphics_dat_packed_bound) {
                receipt.door_front_packed_stride_bytes =
                    state->door_front_d3_packed_stride_bytes[slot];
            }
        }
    }

    return receipt;
}

size_t dm1_viewport_3d_side_occlusion_spec_count(void)
{
    return sizeof(s_side_occlusion_specs) / sizeof(s_side_occlusion_specs[0]);
}

const DM1_ViewportSideOcclusionSpec *dm1_viewport_3d_get_side_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_side_occlusion_spec_count()) return NULL;
    return &s_side_occlusion_specs[index];
}

const DM1_ViewportSideOcclusionSpec *dm1_viewport_3d_get_side_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_side_occlusion_spec_count(); ++i) {
        if (s_side_occlusion_specs[i].square == square) return &s_side_occlusion_specs[i];
    }
    return NULL;
}

size_t dm1_viewport_3d_thieves_eye_door_frame_occlusion_spec_count(void)
{
    return sizeof(s_thieves_eye_door_frame_occlusion_specs) / sizeof(s_thieves_eye_door_frame_occlusion_specs[0]);
}

const DM1_ViewportThievesEyeDoorFrameOcclusionSpec *dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_thieves_eye_door_frame_occlusion_spec_count()) return NULL;
    return &s_thieves_eye_door_frame_occlusion_specs[index];
}

const DM1_ViewportThievesEyeDoorFrameOcclusionSpec *dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_thieves_eye_door_frame_occlusion_spec_count(); ++i) {
        if (s_thieves_eye_door_frame_occlusion_specs[i].square == square) return &s_thieves_eye_door_frame_occlusion_specs[i];
    }
    return NULL;
}

size_t dm1_viewport_3d_floor_field_order_spec_count(void)
{
    return sizeof(s_floor_field_order_specs) / sizeof(s_floor_field_order_specs[0]);
}

const DM1_ViewportFloorFieldOrderSpec *dm1_viewport_3d_get_floor_field_order_spec(size_t index)
{
    if (index >= dm1_viewport_3d_floor_field_order_spec_count()) return NULL;
    return &s_floor_field_order_specs[index];
}

const DM1_ViewportFloorFieldOrderSpec *dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_floor_field_order_spec_count(); ++i) {
        if (s_floor_field_order_specs[i].square == square) return &s_floor_field_order_specs[i];
    }
    return NULL;
}

size_t dm1_viewport_3d_d0l2_d0r2_f0108_composition_spec_count(void)
{
    return sizeof(s_d0l2_d0r2_f0108_composition_specs) /
           sizeof(s_d0l2_d0r2_f0108_composition_specs[0]);
}

const DM1_ViewportD0L2D0R2F0108CompositionSpec *
dm1_viewport_3d_get_d0l2_d0r2_f0108_composition_spec(size_t index)
{
    if (index >= dm1_viewport_3d_d0l2_d0r2_f0108_composition_spec_count()) return NULL;
    return &s_d0l2_d0r2_f0108_composition_specs[index];
}

const DM1_ViewportD0L2D0R2F0108CompositionSpec *
dm1_viewport_3d_get_d0l2_d0r2_f0108_composition_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_d0l2_d0r2_f0108_composition_spec_count(); ++i) {
        if (s_d0l2_d0r2_f0108_composition_specs[i].square == square) {
            return &s_d0l2_d0r2_f0108_composition_specs[i];
        }
    }
    return NULL;
}

const DM1_ViewportPostCommandRedrawSpec *dm1_viewport_3d_post_command_redraw_spec(void)
{
    return &s_post_command_redraw;
}

const DM1_ViewportSameViewportCaptureContract *dm1_viewport_3d_same_viewport_capture_contract(void)
{
    return &s_same_viewport_capture_contract;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_get_dungeon_element
 *
 * Returns the raw dungeon.dat cell value at (map_x, map_y).
 * When dungeon_grid is NULL (DM1 mode), returns 0 (WALL).
 *
 * The raw cell is a 16-bit value stored in the dungeon grid as a byte.
 * Low 5 bits = M034_SQUARE_TYPE = element type:
 *   0=WALL, 1=CORRIDOR, 2=PIT, 3=STAIRS, 4=DOOR,
 *   5=TELEPORTER, 6=FAKEWALL
 *
 * Extended aspect types (16-19) are derived by F0172 from raw type
 * plus wall configuration context.  Callers that already resolved those
 * aspects may provide dungeon_aspect_grid; raw bytes still use sq>>5.
 *
 * Source: ReDMCSB DUNGEON.C:1371-1421 F0150; DEFS.H M034_SQUARE_TYPE (sq>>5)
 * ──────────────────────────────────────────────────────────────────────── */
int dm1_viewport_3d_get_dungeon_element(const DM1_Viewport3DState *state,
                                        int map_x, int map_y)
{
    if (!state || !state->dungeon_grid) return 0;
    if (map_x < 0 || map_x >= state->dungeon_width) return 0;
    if (map_y < 0 || map_y >= state->dungeon_height) return 0;
    if (state->dungeon_aspect_grid) {
        return (int)(state->dungeon_aspect_grid[
            (unsigned)map_y * (unsigned)state->dungeon_width + (unsigned)map_x]);
    }
    /* Dungeon grid is row-major: [y * width + x]
     * Note: CSB uses column-major dungeon.dat layout, but the firestaff
     * dungeon loader converts to row-major when populating the grid. */
    return (int)(state->dungeon_grid[(unsigned)map_y * (unsigned)state->dungeon_width + (unsigned)map_x]);
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_field
 *
 * Draw a field effect (teleporter/fluxcage) at the viewport position
 * corresponding to the relative view square.
 *
 * This still fills with one color until the field bitmap decoder is wired,
 * but the destination rectangle comes from the shared F0113 source-locked
 * render plan rather than an ad hoc wall-frame rectangle.
 *
 * field_color: VGA palette index for the field effect (default 0x1C=cyan).
 *
 * Source: ReDMCSB DUNVIEW.C F0113_DUNGEONVIEW_DrawField (line 4119)
 * ──────────────────────────────────────────────────────────────────────── */
static void dm1_viewport_3d_draw_field(DM1_Viewport3DState *state,
                                        int rel_forward,
                                        int rel_side,
                                        int field_color)
{
    DM1_FieldRenderPlanPc34 plan;
    if (!state || !state->viewport_pixels) return;

    if (!dm1_v1_field_render_plan_for_relative_pc34(rel_forward, rel_side, &plan)) {
        return;
    }

    int dst_x = plan.dstX;
    int dst_y = plan.dstY;
    int width = plan.dstW;
    int height = plan.dstH;

    if (dst_x < 0) { width += dst_x; dst_x = 0; }
    if (dst_y < 0) { height += dst_y; dst_y = 0; }
    if (dst_x + width > DM1_VIEWPORT_WIDTH) width = DM1_VIEWPORT_WIDTH - dst_x;
    if (dst_y + height > DM1_VIEWPORT_HEIGHT) height = DM1_VIEWPORT_HEIGHT - dst_y;
    if (width <= 0 || height <= 0) return;

    /* Draw solid field color rectangle */
    uint8_t *vp = state->viewport_pixels;
    int stride = state->viewport_stride;
    for (int y = 0; y < height; y++) {
        uint8_t *row = vp + (dst_y + y) * stride + dst_x;
        for (int x = 0; x < width; x++) {
            row[x] = (uint8_t)field_color;
        }
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_floor_ornament_simple
 *
 * Renders a floor ornament at the appropriate viewport position for
 * CSB back-wall squares (D3L2/D3R2) and near-wall squares (D2L2/D2R2).
 *
 * This is a bounded implementation of ReDMCSB F0108_DUNGEONVIEW_DrawFloorOrnament.
 * When a GRAPHICS.DAT provider is available, it blits the expanded source
 * bitmap through the source-locked G0206 zone with C10 transparency. Without
 * provider pixels it falls back to a flat diagnostic fill for tests.
 *
 * View floor index mapping (ReDMCSB DEFS.H C00_VIEW_FLOOR_D3L2/C01_VIEW_FLOOR_D3R2):
 *   D3L2 (square=-101): floor_index=0 (C00_VIEW_FLOOR_D3L2)
 *   D3R2 (square=-102): floor_index=1 (C01_VIEW_FLOOR_D3R2)
 *
 * Source: ReDMCSB DUNVIEW.C:3940-4015 F0108_DrawFloorOrnament;
 *   6270-6271 (D3L2 corridor/teleporter) · 6337-6351 (D3R2 corridor/teleporter);
 *   6849-6865 F0678 (D2L2) · 6877-6896 F0679 (D2R2)
 * ──────────────────────────────────────────────────────────────────────── */
static int dm1_viewport_3d_floor_ornament_side_for_square(
    DM1_ViewSquareIndex square)
{
    if (square == DM1_VIEW_SQUARE_D3L2 || square == DM1_VIEW_SQUARE_D3L) return -1;
    if (square == DM1_VIEW_SQUARE_D3R2 || square == DM1_VIEW_SQUARE_D3R) return 1;
    return 0;
}

int dm1_viewport_3d_draw_floor_ornament(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int floor_ornament_index)
{
    DM1_FloorOrnamentRenderPlanPc34 plan;
    int rel_side;
    int max_x;
    int max_y;
    uint8_t *vp;
    int stride;
    const uint8_t *provider_pixels = NULL;
    int provider_width = 0;
    int provider_height = 0;

    if (!state || !state->viewport_pixels) return 0;
    if (floor_ornament_index <= 0) return 0;

    rel_side = dm1_viewport_3d_floor_ornament_side_for_square(square);
    if (rel_side == 0 ||
        !dm1_v1_floor_ornament_source_zone_pc34(3, rel_side, &plan)) {
        return 0;
    }

    max_x = plan.blit.dstX + plan.blit.width;
    max_y = plan.blit.dstY + plan.blit.height;
    if (plan.blit.dstX < 0 || plan.blit.dstY < 0 ||
        max_x > DM1_VIEWPORT_WIDTH || max_y > DM1_VIEWPORT_HEIGHT ||
        plan.blit.width <= 0 || plan.blit.height <= 0) {
        return 0;
    }

    vp = state->viewport_pixels;
    stride = state->viewport_stride;

    /*
     * ReDMCSB F0108 DUNVIEW.C:3965-3998 obtains the native floor-ornament
     * bitmap through F0489, resolves the G0206 coordinate set, and blits with
     * C10 transparency. M11 supplies already-expanded GRAPHICS.DAT pixels via
     * the provider callback; DM1 owns the zone and transparency decision here.
     */
    if (state->graphic_provider_callback &&
        state->graphic_provider_callback(
            state->graphic_provider_user_data,
            floor_ornament_index,
            &provider_pixels,
            &provider_width,
            &provider_height) &&
        provider_pixels &&
        provider_width >= plan.blit.width &&
        provider_height >= plan.blit.height) {
        for (int y = 0; y < plan.blit.height; ++y) {
            const uint8_t *src = provider_pixels + y * provider_width;
            uint8_t *row = vp + (plan.blit.dstY + y) * stride + plan.blit.dstX;
            for (int x = 0; x < plan.blit.width; ++x) {
                uint8_t pixel = src[x];
                if (pixel != COLOR_TRANSPARENT) {
                    row[x] = pixel;
                }
            }
        }
        return 1;
    }

    /* F0108 calls F0489 for the original ornament graphic. Without that
     * caller-owned GRAPHICS.DAT span, there is no source pixel to present. */
    return 0;
}

static int dm1_viewport_3d_draw_d3_door_front_plan(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int door_front_index)
{
    const DM1_V1_D3L2D3R2F0111DoorFrontSpecPc34 *spec;
    DM1_V1_D3L2D3R2F0111DoorFrontMaterialPlanPc34 plan;
    int side;
    int slot;

    if (!state || !state->viewport_pixels) return 0;
    if (door_front_index <= 0) return 0;

    if (square == DM1_VIEW_SQUARE_D3L2) {
        side = DM1_V1_D3L2_D3R2_F0111_SIDE_D3L2_PC34;
        slot = 0;
    } else if (square == DM1_VIEW_SQUARE_D3R2) {
        side = DM1_V1_D3L2_D3R2_F0111_SIDE_D3R2_PC34;
        slot = 1;
    } else {
        return 0;
    }

    spec = dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_for_side_pc34(side);
    if (!spec ||
        !dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_material_plan_pc34(
            spec, &plan)) {
        return 0;
    }
    if (plan.x < 0 || plan.y < 0 ||
        plan.x + plan.width > DM1_VIEWPORT_WIDTH ||
        plan.y + plan.height > DM1_VIEWPORT_HEIGHT ||
        plan.width <= 0 || plan.height <= 0) {
        return 0;
    }

    /* ReDMCSB F0676/F0677 call F0111 between the two F0115 passes
     * (DUNVIEW.C:6272,6339). F0111:4257-4334 copies G0693 to temporary
     * bitmap storage and draws it through C3700/C3710 with C10 transparency.
     * Use the existing material plan so real GRAPHICS.DAT bytes can replace
     * this bounded route without changing the runtime order. */
    {
        const uint8_t *provider_pixels = NULL;
        int provider_width = 0;
        int provider_height = 0;
        if (state->graphic_provider_callback &&
            state->graphic_provider_callback(
                state->graphic_provider_user_data,
                door_front_index,
                &provider_pixels,
                &provider_width,
                &provider_height) &&
            provider_pixels &&
            provider_width >= plan.width &&
            provider_height >= plan.height) {
            for (int y = 0; y < plan.height; ++y) {
                const uint8_t *src = provider_pixels + y * provider_width;
                uint8_t *dst =
                    state->viewport_pixels + (plan.y + y) * state->viewport_stride + plan.x;
                for (int x = 0; x < plan.width; ++x) {
                    uint8_t pixel = src[x];
                    if (pixel != COLOR_TRANSPARENT) {
                        dst[x] = pixel;
                    }
                }
            }
            return 1;
        }
    }

    /* ReDMCSB F0111 obtains G0693 through F0489 and copies native packed
     * rows into G0074_puc_Bitmap_Temporary before the C3700/C3710 blit.
     * Source: DUNVIEW.C:4218-4263, 4333-4334; F0676/F0677:6272,6339. */
    if (dm1_viewport_3d_d3_door_front_packed_source_ready(
            state, slot, &plan) &&
        dm1_viewport_3d_d3_door_front_temp_ready(state, &plan)) {
        const uint8_t *source = state->door_front_d3_packed_source[slot];
        int source_stride = state->door_front_d3_packed_stride_bytes[slot];
        for (int y = 0; y < plan.height; ++y) {
            memcpy(state->temp_bitmap + y * plan.source_stride_bytes,
                   source + y * source_stride,
                   (size_t)plan.source_stride_bytes);
        }
    }

    if (!dm1_viewport_3d_d3_door_front_temp_ready(state, &plan) &&
        !dm1_viewport_3d_d3_door_front_packed_source_ready(state, slot, &plan)) {
        /* F0111's C3700/C3710 blit consumes G0693/G0074 source pixels.
         * Missing PC34 media is unavailable, not a host-colored door. */
        return 0;
    }

    for (int y = 0; y < plan.height; ++y) {
        uint8_t expanded[DM1_V1_D3L2_D3R2_F0111_DOOR_FRONT_VIEWPORT_WIDTH_PC34];
        const uint8_t *src = NULL;
        uint8_t *dst = state->viewport_pixels + (plan.y + y) * state->viewport_stride + plan.x;
        int have_expanded = 0;

        if (dm1_viewport_3d_d3_door_front_temp_ready(state, &plan)) {
            src = state->temp_bitmap + y * plan.source_stride_bytes;
            have_expanded =
                dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_materialize_row_pc34(
                    &plan, src, (size_t)plan.source_stride_bytes,
                    expanded, sizeof(expanded)) ? 1 : 0;
        } else if (dm1_viewport_3d_d3_door_front_packed_source_ready(
                       state, slot, &plan)) {
            src = state->door_front_d3_packed_source[slot] +
                  y * state->door_front_d3_packed_stride_bytes[slot];
            have_expanded =
                dm1_v1_viewport_d3l2_d3r2_f0111_door_front_pair_materialize_row_pc34(
                    &plan, src,
                    (size_t)state->door_front_d3_packed_stride_bytes[slot],
                    expanded, sizeof(expanded)) ? 1 : 0;
        }

        if (!have_expanded) {
            return 0;
        }
        for (int x = 0; x < plan.width; ++x) {
            uint8_t pixel = expanded[x];
            if (pixel != COLOR_TRANSPARENT) {
                dst[x] = pixel;
            }
        }
    }
    return 1;
}

static int dm1_viewport_3d_draw_d3_side_graphic(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int graphic_index,
    int flipped)
{
    const DM1_WallFrame *frame;
    const uint8_t *pixels = NULL;
    int width = 0;
    int height = 0;

    if (!state || !state->viewport_pixels || graphic_index <= 0) return 0;
    frame = dm1_viewport_3d_get_wall_frame(square);
    if (!frame || !state->graphic_provider_callback ||
        !state->graphic_provider_callback(state->graphic_provider_user_data,
                                          graphic_index, &pixels, &width, &height) ||
        !pixels || width < frame->byte_width || height < frame->height) {
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0676/F0677 use F0104 for D3L2 and F0105 for
     * D3R2. F0105 reverses source X while retaining the D3-side frame. */
    for (int y = 0; y < frame->height; ++y) {
        const uint8_t *src = pixels + y * width;
        uint8_t *dst = state->viewport_pixels +
                       (frame->top_y + y) * state->viewport_stride + frame->left_x;
        for (int x = 0; x < frame->byte_width; ++x) {
            uint8_t pixel = src[flipped ? frame->byte_width - 1 - x : x];
            if (pixel != COLOR_TRANSPARENT) dst[x] = pixel;
        }
    }
    return 1;
}

/* ReDMCSB: DUNVIEW.C F0116:6387-6494 and F0117:6500-6634.  These are the
 * ordinary depth-three side lanes, distinct from the MEDIA720-only D3L2/D3R2
 * helpers below.  F0115 material emission remains at the host integration
 * boundary; this DM1 route owns the source-selected structural pixels and
 * their ordering around that boundary. */
static void dm1_viewport_3d_draw_d3_side_square(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int map_x,
    int map_y)
{
    const DM1_WallFrame *frame;
    const uint8_t *wall_base = g_dm1_wall_frame_bitmaps;
    int cell;
    int element;
    int right;
    size_t map_cell;

    if (!state ||
        (square != DM1_VIEW_SQUARE_D3L && square != DM1_VIEW_SQUARE_D3R)) {
        return;
    }
    frame = dm1_viewport_3d_get_wall_frame(square);
    if (!frame) return;
    cell = dm1_viewport_3d_get_dungeon_element(state, map_x, map_y);
    element = state->dungeon_aspect_grid ? cell
                                         : dm1_viewport_3d_classify_grid_cell(cell);
    right = square == DM1_VIEW_SQUARE_D3R;
    map_cell = (size_t)map_y * (size_t)state->dungeon_width + (size_t)map_x;

    if (element == DM1_VP_ELEMENT_WALL) {
        const uint8_t *wall = dm1_viewport_3d_selected_wall_bitmap(
            state, wall_base, right ? DM1_WALL_D3R : DM1_WALL_D3L);
        if (state->parity_flip) {
            dm1_viewport_3d_draw_wall_parity_mirrored(state, wall, frame);
        } else {
            dm1_viewport_3d_draw_wall(state, wall, frame);
        }
        dm1_viewport_3d_draw_wall_ornament_f0107(state,
            right ? DM1_V1_VIEW_WALL_D3R_LEFT_PC34
                  : DM1_V1_VIEW_WALL_D3L_RIGHT_PC34,
            map_x, map_y);
        return;
    }

    if (element == DM1_VP_ELEMENT_STAIRS_FRONT || element == DM1_VP_ELEMENT_PIT) {
        int graphic = element == DM1_VP_ELEMENT_PIT ? 49 :
            state->stairs_indices[(state->dungeon_stairs_up_grid &&
                                    state->dungeon_stairs_up_grid[map_cell]) ? 0 : 7];
        int invisible = element == DM1_VP_ELEMENT_PIT &&
            state->dungeon_pit_invisible_grid && state->dungeon_pit_invisible_grid[map_cell];
        if (!invisible) {
            (void)dm1_viewport_3d_draw_d3_side_graphic(state, square, graphic, right);
        }
    }

    /* F0116/F0117 deliberately fall through from front stairs and pits to
     * F0108 then F0115.  The source-zone plan preserves BUG0_64: a floor
     * ornament is still drawn over a visible pit. */
    if (element == DM1_VP_ELEMENT_CORRIDOR || element == DM1_VP_ELEMENT_PIT ||
        element == DM1_VP_ELEMENT_TELEPORTER ||
        element == DM1_VP_ELEMENT_STAIRS_FRONT ||
        element == DM1_VP_ELEMENT_STAIRS_SIDE ||
        element == DM1_VP_ELEMENT_DOOR_SIDE ||
        element == DM1_VP_ELEMENT_DOOR_FRONT) {
        int ornament_slot = right ? 3 : 2;
        (void)dm1_viewport_3d_draw_floor_ornament(
            state, square, state->floor_ornament_indices[ornament_slot]);
    }

    if (element == DM1_VP_ELEMENT_DOOR_FRONT) {
        /* ReDMCSB F0116:6446-6454 and F0117:6582-6590 place these frames
         * between F0115's rear and front thing passes. */
        if (right) {
            dm1_viewport_3d_draw_door_frame_flipped(state,
                                                     wall_base + 20 * DM1_VIEWPORT_BYTE_WIDTH,
                                                     frame);
        } else {
            dm1_viewport_3d_draw_wall(state,
                                      wall_base + 20 * DM1_VIEWPORT_BYTE_WIDTH,
                                      frame);
            dm1_viewport_3d_draw_door_frame_flipped(state,
                                                     wall_base + 20 * DM1_VIEWPORT_BYTE_WIDTH,
                                                     frame);
        }
    }
    if (element == DM1_VP_ELEMENT_TELEPORTER) {
        dm1_viewport_3d_draw_field(state, 3, right ? 1 : -1, 0x1c);
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_center_wall_element
 *
 * Element routing for center squares D3C/D2C/D1C.
 * Returns:
 *   1 = WALL drawn (wall bitmap + F0107 ornament); skip door frame
 *   2 = open cell (corridor/pit/stairs/teleporter); skip door frame
 *   0 = DOOR_FRONT element; caller should draw door frame
 *
 * Source: DUNVIEW.C:6707-6714 (D3C), 7299-7306 (D2C), 7833-7840 (D1C)
 * ──────────────────────────────────────────────────────────────────────── */
static int dm1_viewport_3d_draw_center_wall_element(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int map_x, int map_y)
{
    const uint8_t *bm_base = g_dm1_wall_frame_bitmaps;
    int cell, element;
    DM1_WallSetIndex native_wall;
    const DM1_WallFrame *fr;
    int vwi;

    if (!state) return 0;
    if (square != DM1_VIEW_SQUARE_D3C &&
        square != DM1_VIEW_SQUARE_D2C &&
        square != DM1_VIEW_SQUARE_D1C) {
        return 0;
    }

    cell = dm1_viewport_3d_get_dungeon_element(state, map_x, map_y);
    element = state->dungeon_aspect_grid ? cell
                                         : dm1_viewport_3d_classify_grid_cell(cell);

    if (element != DM1_VP_ELEMENT_WALL) {
        if (element == DM1_VP_ELEMENT_DOOR_FRONT ||
            element == DM1_VP_ELEMENT_DOOR ||
            element == DM1_VP_ELEMENT_DOOR_SIDE) {
            return 0;
        }
        return 2;
    }

    if (square == DM1_VIEW_SQUARE_D3C) {
        native_wall = DM1_WALL_D3C;
        vwi = DM1_V1_VIEW_WALL_D3C_FRONT_PC34;
    } else if (square == DM1_VIEW_SQUARE_D2C) {
        native_wall = DM1_WALL_D2C;
        vwi = DM1_V1_VIEW_WALL_D2C_FRONT_PC34;
    } else {
        native_wall = DM1_WALL_D1C;
        vwi = DM1_V1_VIEW_WALL_D1C_FRONT_PC34;
    }

    fr = dm1_viewport_3d_get_wall_frame(square);
    if (!fr) return 1;

    {
        const uint8_t *wall = dm1_viewport_3d_selected_wall_bitmap(
            state, bm_base, native_wall);
        if (state->parity_flip) {
            dm1_viewport_3d_draw_wall_parity_mirrored(state, wall, fr);
        } else {
            dm1_viewport_3d_draw_wall(state, wall, fr);
        }
    }

    dm1_viewport_3d_draw_wall_ornament_f0107(state, vwi, map_x, map_y);
    return 1;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_side_wall_element
 *
 * Element routing for side squares D2L/D2R/D1L/D1R.
 * Returns:
 *   1 = WALL drawn (wall bitmap + F0107 ornament); skip door frame
 *   2 = open cell (corridor/pit/stairs/teleporter); skip door frame
 *   0 = DOOR_FRONT element; caller should draw door frame
 *
 * Source: DUNVIEW.C:6954-6964 (D2L), 7105-7115 (D2R),
 *         7445-7455 (D1L), 7613-7623 (D1R)
 * ──────────────────────────────────────────────────────────────────────── */
static int dm1_viewport_3d_draw_side_wall_element(
    DM1_Viewport3DState *state,
    DM1_ViewSquareIndex square,
    int map_x, int map_y)
{
    const uint8_t *bm_base = g_dm1_wall_frame_bitmaps;
    int cell, element;
    DM1_WallSetIndex native_wall, parity_wall;
    const DM1_WallFrame *fr;
    int vwi;

    if (!state) return 0;

    cell = dm1_viewport_3d_get_dungeon_element(state, map_x, map_y);
    element = state->dungeon_aspect_grid ? cell
                                         : dm1_viewport_3d_classify_grid_cell(cell);
    if (element != DM1_VP_ELEMENT_WALL) {
        if (element == DM1_VP_ELEMENT_DOOR_FRONT ||
            element == DM1_VP_ELEMENT_DOOR ||
            element == DM1_VP_ELEMENT_DOOR_SIDE) {
            return 0;
        }
        return 2;
    }

    switch (square) {
    case DM1_VIEW_SQUARE_D2L:
        native_wall = DM1_WALL_D2L; parity_wall = DM1_WALL_D2R;
        vwi = DM1_V1_VIEW_WALL_D2L_FRONT_PC34; break;
    case DM1_VIEW_SQUARE_D2R:
        native_wall = DM1_WALL_D2R; parity_wall = DM1_WALL_D2L;
        vwi = DM1_V1_VIEW_WALL_D2R_FRONT_PC34; break;
    case DM1_VIEW_SQUARE_D1L:
        native_wall = DM1_WALL_D1L; parity_wall = DM1_WALL_D1R;
        vwi = DM1_V1_VIEW_WALL_D1L_RIGHT_PC34; break;
    case DM1_VIEW_SQUARE_D1R:
        native_wall = DM1_WALL_D1R; parity_wall = DM1_WALL_D1L;
        vwi = DM1_V1_VIEW_WALL_D1R_LEFT_PC34; break;
    default: return 0;
    }

    fr = dm1_viewport_3d_get_wall_frame(square);
    if (!fr) return 1;

    {
        bool flip_h = state->parity_flip;
        DM1_WallSetIndex wall_idx = flip_h ? parity_wall : native_wall;
        const uint8_t *wall = dm1_viewport_3d_selected_wall_bitmap(
            state, bm_base, wall_idx);
        if (flip_h) {
            dm1_viewport_3d_draw_wall_parity_mirrored(state, wall, fr);
        } else {
            dm1_viewport_3d_draw_wall(state, wall, fr);
        }
    }

    dm1_viewport_3d_draw_wall_ornament_f0107(state, vwi, map_x, map_y);
    return 1;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_csb_back_wall
 *
 * Draw a CSB back-wall square (D3L2 or D3R2) with full element routing.
 *
 * This implements ReDMCSB F0676_DrawD3L2 (line 6226) and F0677_DrawD3R2
 * (line 6293).  These functions are CSB-specific: they render the back
 * wall of a corridor when viewed from a perpendicular direction.
 *
     * Element routing:
     *   WALL        → draw wall bitmap (parity-aware) + wall ornament → return
     *   TELEPORTER  → draw field effect at wall zone → return
     *   STAIRS_FRONT→ draw stairs up/down bitmap → return
     *   PIT         → draw pit bitmap (if not visible) → fall through
     *   CORRIDOR    → draw floor ornament + F0115 receipt → return
     *   DOOR_SIDE   → draw floor ornament + F0115 receipt → return
     *   STAIRS_SIDE → draw floor ornament + F0115 receipt → return
     *   DOOR_FRONT  → floor ornament + F0115 pass1 receipt + door panel receipt +
     *                  F0115 pass2 receipt → return
 *
 * Source: ReDMCSB DUNVIEW.C F0676 (line 6226) · F0677 (line 6293)
 * ──────────────────────────────────────────────────────────────────────── */

/* F0107 wall ornament overlay.  Resolves the ornament ordinal for (map_x,map_y)
 * via the host callback, then uses the F0107 resolver to get the render plan
 * and blits the ornament bitmap via the graphic provider.
 * Source: DUNVIEW.C:3502-3938 F0107_IsDrawnWallOrnamentAnAlcove_CPSF */
static void dm1_viewport_3d_draw_wall_ornament_f0107(
    DM1_Viewport3DState *state,
    int view_wall_index,
    int map_x, int map_y)
{
    int ordinal;
    DM1_V1_WallOrnamentOrdinalInputPc34 input;
    DM1_V1_WallOrnamentOrdinalResultPc34 result;
    int coord_set;
    DM1_WallOrnamentRenderPlanPc34 plan;

    if (!state || !state->wall_ornament_ordinal_callback ||
        !state->graphic_provider_callback) {
        return;
    }

    ordinal = state->wall_ornament_ordinal_callback(
        state->wall_ornament_ordinal_user_data, map_x, map_y);
    if (ordinal < 0) return;

    coord_set = dm1_v1_wall_ornament_coord_set_index_pc34(ordinal);

    memset(&input, 0, sizeof(input));
    input.wall_ornament_ordinal = ordinal;
    input.view_wall_index = view_wall_index;
    input.coordinate_set = coord_set;
    input.native_bitmap_index = -1;
    input.is_alcove = dm1_v1_wall_ornament_is_alcove_local_ordinal_pc34(ordinal) != 0;

    if (!dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(&input, &result)) {
        return;
    }
    if (!result.draws_ornament) return;

    if (dm1_v1_wall_ornament_render_plan_pc34(
            result.wall_ornament_index, view_wall_index, 136, &plan) && plan.graphicIndex >= 0) {
        const uint8_t *pixels = NULL;
        int gfx_w = 0, gfx_h = 0;
        if (state->graphic_provider_callback(
                state->graphic_provider_user_data,
                plan.graphicIndex, &pixels, &gfx_w, &gfx_h) && pixels) {
            int sx = plan.srcX;
            int sy = plan.srcY;
            int dx = plan.dstX;
            int dy = plan.dstY;
            int w = plan.width;
            int h = plan.height;
            int x, y;
            if (sx < 0) sx = 0;
            if (sy < 0) sy = 0;
            if (w > gfx_w - sx) w = gfx_w - sx;
            if (h > gfx_h - sy) h = gfx_h - sy;
            if (w <= 0 || h <= 0) return;
            if (dx < 0) { sx -= dx; w += dx; dx = 0; }
            if (dy < 0) { sy -= dy; h += dy; dy = 0; }
            if (dx + w > DM1_VIEWPORT_WIDTH) w = DM1_VIEWPORT_WIDTH - dx;
            if (dy + h > DM1_VIEWPORT_HEIGHT) h = DM1_VIEWPORT_HEIGHT - dy;
            if (w <= 0 || h <= 0) return;
            for (y = 0; y < h; ++y) {
                for (x = 0; x < w; ++x) {
                    uint8_t px = pixels[(sy + y) * gfx_w + (sx + x)];
                    if (px != (uint8_t)plan.transparentColor) {
                        state->viewport_pixels[(dy + y) * state->viewport_stride + (dx + x)] = px;
                    }
                }
            }
        }
    }
}

void dm1_viewport_3d_draw_csb_back_wall(DM1_Viewport3DState *state,
                                         DM1_ViewSquareIndex square,
                                         int direction,
                                         int map_x, int map_y)
{
    if (!state) return;

    int cell = dm1_viewport_3d_get_dungeon_element(state, map_x, map_y);
    int element = state->dungeon_aspect_grid ? cell
                                             : dm1_viewport_3d_classify_grid_cell(cell);
    state->last_d3_back_wall_receipt =
        dm1_viewport_3d_build_d3_back_wall_runtime_asset_receipt(state, square, element);

    const uint8_t *bm_base = g_dm1_wall_frame_bitmaps;

    /* D3L2 uses D3L2 frame; D3R2 uses D3R2 frame */
    const DM1_WallFrame *fr = NULL;
    int wall_zone = 0;
    int rel_forward = 3;
    int rel_side = 0;
    DM1_WallSetIndex native_wall = DM1_WALL_D3L2;
    DM1_WallSetIndex parity_wall = DM1_WALL_D3R2;

    if (square == DM1_VIEW_SQUARE_D3L2) {
        fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D3L2);
        wall_zone = DM1_PC34_ZONE_WALL_D3L2; /* 702 */
        rel_side = -2;
        native_wall = DM1_WALL_D3L2;
        parity_wall = DM1_WALL_D3R2;
    } else if (square == DM1_VIEW_SQUARE_D3R2) {
        fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D3R2);
        wall_zone = DM1_PC34_ZONE_WALL_D3R2; /* 703 */
        rel_side = 2;
        native_wall = DM1_WALL_D3R2;
        parity_wall = DM1_WALL_D3L2;
    } else {
        return; /* Not a back-wall square */
    }

    /* ── WALL case: draw wall bitmap + wall ornament, then return ──
     * ReDMCSB F0676 lines 6240-6264 / F0677 lines 6304-6331.
     * Uses G2107_WallSet[parity ? parity_wall : native_wall] bitmap.
     * Then calls F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF
     * for the appropriate wall ornament position. */
    if (element == DM1_VP_ELEMENT_WALL) { /* C00_ELEMENT_WALL */
        bool flip_h = state->parity_flip;
        DM1_WallSetIndex wall_idx = flip_h ? parity_wall : native_wall;
        const uint8_t *wall_bmp = dm1_viewport_3d_selected_wall_bitmap(state, bm_base, wall_idx);

        if (flip_h) {
            dm1_viewport_3d_draw_wall_parity_mirrored(state, wall_bmp, fr);
        } else {
            dm1_viewport_3d_draw_wall(state, wall_bmp, fr);
        }

        /* F0107 wall ornament overlay.
         * Source: DUNVIEW.C:6263-6264 (D3L2) · DUNVIEW.C:6330-6331 (D3R2) */
        {
            int vwi = (square == DM1_VIEW_SQUARE_D3L2)
                ? DM1_V1_VIEW_WALL_D3L2_RIGHT_PC34
                : DM1_V1_VIEW_WALL_D3R2_LEFT_PC34;
            (void)wall_zone;
            dm1_viewport_3d_draw_wall_ornament_f0107(state, vwi, map_x, map_y);
        }
        return;
    }

    /* ── TELEPORTER case: draw field effect, then return ──
     * ReDMCSB F0676 line 6290 / F0677 line 6356.
     * F0113_DUNGEONVIEW_DrawField draws the teleporter swirl effect
     * at the wall zone. */
    if (element == DM1_VP_ELEMENT_TELEPORTER) { /* C05_ELEMENT_TELEPORTER */
        dm1_viewport_3d_draw_field(state, rel_forward, rel_side, 0x1C);
        return;
    }

    /* ── STAIRS_FRONT case: draw source-selected stairs bitmap, then common tail ──
     * ReDMCSB F0676 lines 6237-6251 / F0677 lines 6304-6319.
     * M555_STAIRS_UP in aspect[2] (M555) determines up vs down.
     * Uses zone C800/C801 (up) or C813/C814 (down) for the stairs bitmap.
     *
     * Stairs bitmap indices (relative to GRAPHICS.DAT base):
     *   M714_NEGGRAPHIC_STAIRS_UP_D3L2   — stairs up, D3L2, index 0
     *   M715_NEGGRAPHIC_STAIRS_UP_D3R2   — stairs up, D3R2, index 1
     *   M716_NEGGRAPHIC_STAIRS_DOWN_D3L2 — stairs down, D3L2, index 2
     *   M717_NEGGRAPHIC_STAIRS_DOWN_D3R2 — stairs down, D3R2, index 3
     *
     * Both side lanes use the D3L source index: C00 for up and C07 for down.
     * Source: DUNVIEW.C:6237-6251 (F0676) · DUNVIEW.C:6304-6319 (F0677) */
    if (element == DM1_VP_ELEMENT_STAIRS_FRONT) {
        size_t cell = (size_t)map_y * (size_t)state->dungeon_width + (size_t)map_x;
        int stairs_up = state->dungeon_stairs_up_grid && state->dungeon_stairs_up_grid[cell];
        int graphic_index = state->stairs_indices[stairs_up ? 0 : 7];
        int flipped = square == DM1_VIEW_SQUARE_D3R2;
        state->last_d3_back_wall_receipt.stairs_front_drawn = true;
        state->last_d3_back_wall_receipt.stairs_front_up = stairs_up != 0;
        state->last_d3_back_wall_receipt.stairs_front_flipped = flipped != 0;
        state->last_d3_back_wall_receipt.stairs_front_graphic_index = (int16_t)graphic_index;
        state->last_d3_back_wall_receipt.stairs_front_zone_index = (int16_t)(
            stairs_up ? (flipped ? DM1_PC34_ZONE_STAIRS_UP_FRONT_D3R2 : DM1_PC34_ZONE_STAIRS_UP_FRONT_D3L2)
                      : (flipped ? DM1_PC34_ZONE_STAIRS_DOWN_FRONT_D3R2 : DM1_PC34_ZONE_STAIRS_DOWN_FRONT_D3L2));
        state->last_d3_back_wall_receipt.stairs_front_graphics_dat_bound =
            dm1_viewport_3d_draw_d3_side_graphic(state, square, graphic_index, flipped) != 0;
    }

    /* ── PIT case: draw pit bitmap if not visible, then fall through ──
     * ReDMCSB F0676 line 6276 / F0677 line 6342.
     * M554_PIT_OR_TELEPORTER_VISIBLE in aspect[4] (M554) determines if
     * the pit is invisible (masked).  If visible, no pit bitmap is drawn.
     * If invisible (M554==1), the pit graphic is NOT drawn.
     *
     * The F0172 M554 input is supplied by dungeon_pit_invisible_grid; C049
     * is blitted through C850/C851 only when that input is clear.
     * Source: DUNVIEW.C:6275-6278 (F0676) · DUNVIEW.C:6342-6345 (F0677) */
    if (element == DM1_VP_ELEMENT_PIT) { /* C02_ELEMENT_PIT */
        size_t cell = (size_t)map_y * (size_t)state->dungeon_width + (size_t)map_x;
        int invisible = state->dungeon_pit_invisible_grid && state->dungeon_pit_invisible_grid[cell];
        int flipped = square == DM1_VIEW_SQUARE_D3R2;
        state->last_d3_back_wall_receipt.pit_invisible = invisible != 0;
        state->last_d3_back_wall_receipt.pit_flipped = flipped != 0;
        state->last_d3_back_wall_receipt.pit_graphic_index = 49;
        state->last_d3_back_wall_receipt.pit_zone_index = (int16_t)(
            flipped ? DM1_PC34_ZONE_FLOORPIT_D3R2 : DM1_PC34_ZONE_FLOORPIT_D3L2);
        if (!invisible) {
            state->last_d3_back_wall_receipt.pit_drawn = true;
            state->last_d3_back_wall_receipt.pit_graphics_dat_bound =
                dm1_viewport_3d_draw_d3_side_graphic(state, square, 49, flipped) != 0;
        }
        /* ReDMCSB DUNVIEW.C F0676:6275-6278 / F0677:6342-6345 falls through
         * to the F0108/F0115 tail, including the original BUG0_64 order. */
    }

    /* ── CORRIDOR / TELEPORTER: draw floor ornament + F0115 + field ──
     * ReDMCSB F0676 lines 6282-6289 / F0677 lines 6349-6356.
     * T0676016 / T0676018 common path:
     *   1. F0108_DUNGEONVIEW_DrawFloorOrnament (floor ornament)
     *   2. F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
     *      (things: creatures, items, projectiles, explosions)
     *   3. F0113_DUNGEONVIEW_DrawField (teleporter field, if TELEPORTER)
     *
     * DOOR_SIDE (16) and STAIRS_SIDE (18) also route here.
     * DOOR_FRONT (17) additionally calls F0111 before the second F0115.
     *
     * Firestaff records the F0115 cell-order receipt here; item/creature/
     * projectile bitmap emission remains with the shared F0115 runtime layer.
     *
     * Source: DUNVIEW.C:6282-6289 (F0676 corridor/teleporter) ·
     *         DUNVIEW.C:6349-6356 (F0677 corridor/teleporter) */
    {
        /* Floor ornament drawing via F0108 — placeholder diamond.
         * F0108_DUNGEONVIEW_DrawFloorOrnament composites the floor
         * ornament bitmap at the D3L2/D3R2 position.
         * Source: DUNVIEW.C:6282-6284 (F0676) · DUNVIEW.C:6349-6351 (F0677) */
        int fo_idx = 0;
        switch (square) {
        case DM1_VIEW_SQUARE_D3L2: fo_idx = state->floor_ornament_indices[0]; break;
        case DM1_VIEW_SQUARE_D3R2: fo_idx = state->floor_ornament_indices[1]; break;
        default: break;
        }
        (void)dm1_viewport_3d_draw_floor_ornament(state, square, fo_idx);

        /* F0115 creature/item/projectile/explosion pass receipt.
         * ReDMCSB calls F0115 here with 0x3421/0x4312 for open cells, 0x0321/
         * 0x0412 for side-door/stairs-side cells, or the door-front split below.
         * Source: DUNVIEW.C:6286 (F0676) · DUNVIEW.C:6353 (F0677). */

        /* Teleporter field effect — only for TELEPORTER element.
         * Source: DUNVIEW.C:6288-6289 (F0676) · DUNVIEW.C:6355-6356 (F0677) */
        if (element == DM1_VP_ELEMENT_TELEPORTER) { /* TELEPORTER */
            dm1_viewport_3d_draw_field(state, rel_forward, rel_side, 0x1C);
        }
    }

    /* ── DOOR_FRONT case: also calls F0111 between the two F0115 passes ──
     * ReDMCSB F0676 lines 6271-6286 / F0677 lines 6337-6353.
     * DOOR_FRONT (17) path:
     *   1. F0108_DUNGEONVIEW_DrawFloorOrnament
     *   2. F0115 with order 0x0128 (pass1: rear cells)
     *   3. F0111_DUNGEONVIEW_DrawDoor → C3700_ZONE_DOOR_D3L2 / C3710_ZONE_DOOR_D3R2
     *   4. F0115 with order 0x0349 (pass2: front cells)
     *
     * The receipt records both source cell orders and the F0111 door slot
     * even when the concrete door bitmap asset is not available in this bounded
     * helper. Host/runtime code must consume this route instead of treating the
     * square as a generic open corridor.
     * Source: DUNVIEW.C:6272 (F0676) · DUNVIEW.C:6339 (F0677) */
    if (element == DM1_VP_ELEMENT_DOOR_FRONT) {
        int slot = dm1_viewport_3d_d3_back_wall_side_slot(square);
        if (slot >= 0) {
            (void)dm1_viewport_3d_draw_d3_door_front_plan(
                state, square, state->door_front_d3[slot]);
        }
    }
    (void)direction; /* unused in current stub */
    (void)fr;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_csb_near_wall
 *
 * Draw a CSB near-wall square (D2L2 or D2R2) with element routing.
 *
 * This implements ReDMCSB F0678_DrawD2L2 (line 6837) and F0679_DrawD2R2
 * (line 6868).  Unlike D3L2/D3R2, D2L2/D2R2 are simpler: they only
 * render the wall bitmap (for WALL) or the teleporter field effect
 * (for TELEPORTER).  They do NOT call F0115 (no thing rendering),
 * do NOT render stairs/pits/floor ornaments, and do NOT handle doors.
 *
 * Element routing:
 *   WALL       → draw wall bitmap (parity-aware) → return
 *   TELEPORTER → draw field effect at wall zone → return
 *   all others → no-op
 *
 * Source: ReDMCSB DUNVIEW.C F0678 (line 6837) · F0679 (line 6868)
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_csb_near_wall(DM1_Viewport3DState *state,
                                          DM1_ViewSquareIndex square,
                                          int direction,
                                          int map_x, int map_y)
{
    (void)direction; /* unused — parity comes from viewport state */
    if (!state) return;

    int cell = dm1_viewport_3d_get_dungeon_element(state, map_x, map_y);
    int element = state->dungeon_aspect_grid ? cell
                                             : dm1_viewport_3d_classify_grid_cell(cell);

    const uint8_t *bm_base = g_dm1_wall_frame_bitmaps;

    /* D2L2 uses D2L2 frame; D2R2 uses D2R2 frame */
    const DM1_WallFrame *fr = NULL;
    int wall_zone = 0;
    int rel_forward = 2;
    int rel_side = 0;
    DM1_WallSetIndex native_wall = DM1_WALL_D2L2;
    DM1_WallSetIndex parity_wall = DM1_WALL_D2R2;

    if (square == DM1_VIEW_SQUARE_D2L2) {
        fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2L2);
        wall_zone = DM1_PC34_ZONE_WALL_D2L2; /* 707 */
        rel_side = -2;
        native_wall = DM1_WALL_D2L2;
        parity_wall = DM1_WALL_D2R2;
    } else if (square == DM1_VIEW_SQUARE_D2R2) {
        fr = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D2R2);
        wall_zone = DM1_PC34_ZONE_WALL_D2R2; /* 708 */
        rel_side = 2;
        native_wall = DM1_WALL_D2R2;
        parity_wall = DM1_WALL_D2L2;
    } else {
        return; /* Not a near-wall square */
    }
    /* ── WALL case: draw wall bitmap + F0107 ornament, then return ──
     * ReDMCSB F0678 lines 6848-6862 / F0679 lines 6879-6893.
     * For D2L2/D2R2, the wall zone is C707/C708 and the bitmap
     * comes from G2107_WallSet[C06_WALL_D2L2] or C05_WALL_D2R2
     * with optional horizontal flip for parity. */
    if (element == DM1_VP_ELEMENT_WALL) { /* C00_ELEMENT_WALL */
        bool flip_h = state->parity_flip;
        DM1_WallSetIndex wall_idx = flip_h ? parity_wall : native_wall;
        const uint8_t *wall_bmp = dm1_viewport_3d_selected_wall_bitmap(state, bm_base, wall_idx);

        if (flip_h) {
            dm1_viewport_3d_draw_wall_parity_mirrored(state, wall_bmp, fr);
        } else {
            dm1_viewport_3d_draw_wall(state, wall_bmp, fr);
        }
        {
            int vwi = (square == DM1_VIEW_SQUARE_D2L2)
                ? DM1_V1_VIEW_WALL_D2L_RIGHT_PC34
                : DM1_V1_VIEW_WALL_D2R_LEFT_PC34;
            (void)wall_zone;
            dm1_viewport_3d_draw_wall_ornament_f0107(state, vwi, map_x, map_y);
        }
        return;
    }

    /* ── TELEPORTER case: draw field effect, then return ──
     * ReDMCSB F0678 lines 6863-6865 / F0679 lines 6894-6896.
     * F0113_DUNGEONVIEW_DrawField draws the teleporter field at
     * zone C707 (D2L2) or C708 (D2R2). */
    if (element == DM1_VP_ELEMENT_TELEPORTER) { /* C05_ELEMENT_TELEPORTER */
        dm1_viewport_3d_draw_field(state, rel_forward, rel_side, 0x1C);
        return;
    }

    /* ── All other elements: no-op ──
     * D2L2/D2R2 do NOT render stairs, pits, floor ornaments,
     * creatures, items, or projectiles.  Only walls and teleporters.
     * Source: DUNVIEW.C:6846-6865 (F0678) · DUNVIEW.C:6877-6896 (F0679) */
}

/* ────────────────────────────────────────────────────────────────────────────
 * Source Evidence
 * ──────────────────────────────────────────────────────────────────────── */
const char *dm1_viewport_3d_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/\n"
        "  VIEWPORT.C:16  F0564_VIEWPORT_InitializeBitPlanes\n"
        "  VIEWPORT.C:33  F0565_VIEWPORT_SetPalette\n"
        "  VIEWPORT.C:56  F0566_VIEWPORT_BlitToScreen\n"
        "  DUNVIEW.C:183  G2107_WallSet[15] PC34/I34E wall bitmap order\n"
        "  DUNVIEW.C:2225 F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF\n"
        "  DUNVIEW.C:2427-2443 G3048_WallSetFlipped pair generation for flipped-capable ports\n"
        "  DUNVIEW.C:581  G0163_aauc_Graphic558_Frame_Walls[12][8]\n  DUNVIEW.C:3053-3058 F0100 uses frame clip + source x/y; C4 zero gates empty walls\n  COORD.C:2390-2409 F0635 clips MEDIA720 zones and source offsets; IMAGE3.C:866-889 F0684 skips empty blits\n"
        "  DEFS.H:4040-4057 PC34 viewport zone constants for ceiling/floor/wall areas\n"
        "  DUNVIEW.C:2962 F0098_DUNGEONVIEW_DrawFloorAndCeiling\n"
        "  DUNVIEW.C:3018 F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal\n"
        "  DUNVIEW.C:3048 F0100_DUNGEONVIEW_DrawWallSetBitmap\n"
        "  DUNVIEW.C:3065 F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency\n"
        "  DUNVIEW.C:3082 F0102_DUNGEONVIEW_DrawDoorBitmap\n"
        "  DUNVIEW.C:3096 F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally\n"
        "  DUNVIEW.C:3113 F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap\n"
        "  DUNVIEW.C:3185 F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally\n"
        "  DUNVIEW.C:3502 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF\n"
        "  DUNVIEW.C:4013-4117 F0109 composes door ornaments into G0074 temporary door bitmap\n"
        "  DUNVIEW.C:4218 F0111_DUNGEONVIEW_DrawDoor\n"
        "  DUNVIEW.C:4218-4335 F0111 copies door panel to G0074, applies ornaments/masks, then blits G0074 into viewport\n"
        "  DUNVIEW.C:4547 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF\n"
        "  DUNVIEW.C:4561-4581 F0115 packed cell-order and object/creature/projectile/explosion z-order\n"
        "  DUNVIEW.C:8466-8477 F0128 D4 far-object passes: D4L, D4R, D4C use F0162 first-object seed and C0x0001 before D3 walls\n"
        "  DUNVIEW.C:5681-5883 F0115 projectile draw pass and PC34 zone draw\n"
        "  DUNVIEW.C:373,5667-5683 projectile occlusion via G2028 row and C2900 zone mapping; D3 front cells/D0 back cells clipped\n"
        "  DUNVIEW.C:5915-5933 F0115 explosion pass after all ordered cells\n"
        "  DUNVIEW.C:373-377,5920-6129; DEFS.H:3749,4232-4235 PC34 explosion viewport zones: C004 D0C pattern, C3000/C3007 rebirth rows, C3014 centered rows, C3031 two-cell rows\n"
        "  DUNVIEW.C:6237-6289 D3L2 stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6375-6495 D3L stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6514-6638 D3R stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6666-6831 D3C stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6304-6356 D3R2 mirrored stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6914-7048 D2L stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:7065-7240 D2R stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:7260-7388 D2C stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6846-6865 D2L2 wall-return/teleporter-field helper has no F0115 thing pass\n"
        "  DUNVIEW.C:6877-6896 D2R2 mirrored wall-return/teleporter-field helper has no F0115 thing pass\n"
        "  DUNVIEW.C:7391-7557 D1L stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:7559-7725 D1R stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:7978-8062 D0L stairs/pit/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:8082-8162 D0R mirrored stairs/pit/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:8185-8240,8241-8308 D0C door-side/stairs foreground blockers draw before common F0115; pit/ceiling/F0115/teleporter-field order\n"
        "  DUNVIEW.C:6443-6459 D3L door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:6270-6286 D3L2 far door-front occlusion: rear pass, far door, front pass\n"
        "  DUNVIEW.C:6337-6353 D3R2 mirrored far door-front occlusion: rear pass, far door, front pass\n"
        "  DUNVIEW.C:6579-6601 D3R mirrored door-front occlusion: rear pass, frame/button/door, front pass\n"
        "  DUNVIEW.C:6722-6746 D3C door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:6988-7003 D2L door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:7181-7196 D2R mirrored door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:7314-7341 D2C door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:7493-7536 D1L door-front occlusion: rear side cell, top frame/door, front side cell\n"
        "  DUNVIEW.C:7661-7704 D1R mirrored door-front occlusion: rear side cell, top frame/door, front side cell\n"
        "  DEFS.H:4082-4088 PC34/I34E D2C door-frame zones 724/725/730\n"
        "  DUNVIEW.C:7289-7312 D2C front wall: wall zone, front ornament/alcove exception, else return before open-cell draw\n"
        "  DUNVIEW.C:7353-7387 D2C open/pit/teleporter order: 0x3421 floor/ceiling/F0115, then field overlay\n"
        "  DUNVIEW.C:7874-7937 D1C door-front occlusion: floor underlay, rear pass, frame/button/door, front pass\n"
        "  DUNVIEW.C:8185-8216 D0C Thieves Eye door-side frame occlusion: copy front frame, composite hole, blit temporary frame before common F0115\n"
        "  DUNVIEW.C:6438-6480,6574-6621,D2/D1/D0 side-door/stairs-side F0115 cell-order occlusion\n"
        "  DUNVIEW.C:6254-6327 F0676/F0677 PC34 parity side-wall selection; wall case returns / front alcove occlusion boundaries\n"
        "  DUNVIEW.C:6849-6893 F0678/F0679 PC34 D2L2/D2R2 side-wall zones and wall-case returns\n"
        "  DUNVIEW.C:6361-6495 F0116_DUNGEONVIEW_DrawSquareD3L D3L1 no-write target evidence; DUNVIEW.C:8488-8499 F0128 keeps adjacent D3 draw paths active\n"
        "  DUNVIEW.C:6361 F0116_DUNGEONVIEW_DrawSquareD3L\n"
        "  DUNVIEW.C:6500 F0117_DUNGEONVIEW_DrawSquareD3R\n"
        "  DUNVIEW.C:6642 F0118_DUNGEONVIEW_DrawSquareD3C_CPSF\n"
        "  DUNVIEW.C:6900 F0119_DUNGEONVIEW_DrawSquareD2L\n"
        "  DUNVIEW.C:7051 F0120_DUNGEONVIEW_DrawSquareD2R_CPSF\n"
        "  DUNVIEW.C:7244 F0121_DUNGEONVIEW_DrawSquareD2C\n"
        "  DUNVIEW.C:7391 F0122_DUNGEONVIEW_DrawSquareD1L\n"
        "  DUNVIEW.C:7559 F0123_DUNGEONVIEW_DrawSquareD1R\n"
        "  DUNVIEW.C:7727 F0124_DUNGEONVIEW_DrawSquareD1C\n"
        "  DUNVIEW.C:7960 F0125_DUNGEONVIEW_DrawSquareD0L\n"
        "  DUNVIEW.C:8064 F0126_DUNGEONVIEW_DrawSquareD0R\n"
        "  DUNVIEW.C:8164 F0127_DUNGEONVIEW_DrawSquareD0C\n"
        "  DUNVIEW.C:8318 F0128_DUNGEONVIEW_Draw_CPSF\n"
        "  DUNGEON.C:1371-1421 F0150 resolves F0128 relative depth/lateral offsets to map X/Y\n"
        "  COMMAND.C:2045-2156 F0380 command dispatch before next main-loop redraw\n"
        "  GAMELOOP.C:55-90 next loop iteration redraws F0128 from post-command G0308/G0306/G0307 party tuple\n"
        "  COMMAND.C:106-114 PC34 movement/dungeon-view mouse zones used by same-viewport capture labels\n"
        "  COMMAND.C:2045-2156 F0380 queue pop/count delta required for same-viewport promotion\n"
        "  CLIKMENU.C:142-174 F0365 turn state mutation required for turn-frame promotion\n"
        "  CLIKMENU.C:180-347 F0366 movement result required for movement-frame promotion\n"
        "  DUNVIEW.C:8318-8611 F0128 direction/mapX/mapY tuple composition required for original frame binding\n"
        "  DRAWVIEW.C:709-858 F0097 PC34 viewport-present boundary required before screenshot acceptance\n"
        "  canonical DM1 PC34 assets GRAPHICS.DAT/DUNGEON.DAT/TITLE hashes are required by pass608\n"
        "  DRAWVIEW.C:709-722 F0097 requests viewport blit and waits for vertical blank\n"
        "  DUNVIEW.C:8446-8542 F0128 back-to-front viewport wall/object draw order\n"
        "  DUNVIEW.C:8478-8541 F0128 wall distance buckets replay D3 side/front, D2 side/front, D1 side/front, then D0 side walls\n"
        "  DUNVIEW.C:8577-8579 F0128 restores G3071_WallSetNotFlipped to G2107_WallSet\n"
        "  DUNVIEW.C:8609-8610 F0128 calls F0097_DUNGEONVIEW_DrawViewport after wall order completes\n"
        "  DRAWVIEW.C:721-722 F0097 viewport blit request/vblank wait\n"
        "  DRAWVIEW.C     F0694_SetMultipleColorsInPalette\n"
        "  DRAWVIEW.C     F0695_SetCreatureReplacementColors\n";
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining DUNVIEW.C function citations for parity
 *
 *   DUNVIEW.C:1996 F0093_DUNGEONVIEW_A
 *   DUNVIEW.C:4269 F0131_VIDEO_F
 *   DUNVIEW.C:6803 F0137_CPSEF_P
 *   DUNVIEW.C:6074 F0277_CPSE_I
 *   DUNVIEW.C:2555 F0707_R
 *   DUNVIEW.C:2936 F0710_W
 *   DUNVIEW.C:2330 F0809_C
 * ══════════════════════════════════════════════════════════════════════ */

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — DRAWVIEW.C remaining function citations
 *
 *   DRAWVIEW.C:1099 F2164_P
 *   DRAWVIEW.C:556 F2261_P
 *   DRAWVIEW.C:558 F8156_S
 *   DRAWVIEW.C:641 F8157_VIDRV_
 *   DRAWVIEW.C:684 F8232_VIDRV_
 * ══════════════════════════════════════════════════════════════════════ */
