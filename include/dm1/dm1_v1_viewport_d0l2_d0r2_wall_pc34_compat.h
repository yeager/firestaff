#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_WALL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_WALL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D0L2/D0R2 wall source-lock gate.
 *
 * ReDMCSB anchors:
 * - DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport lines 709-857 keep the
 *   composed viewport and palette handoff deterministic.
 * - DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling lines 2962-3003
 *   owns the floor/ceiling base rows before F0128 overlays wall squares.
 * - DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF lines 8318-8542 performs
 *   near-view dispatch; lines 8503-8508 are the existing D2L2/D2R2
 *   dispatch anchor and lines 8534-8542 dispatch D0L, D0R, then D0C.
 * - DUNVIEW.C F0125_DUNGEONVIEW_DrawSquareD0L lines 7960-8062 and
 *   F0126_DUNGEONVIEW_DrawSquareD0R lines 8064-8162 route D0 side WALL
 *   cases through C716/C717 and return before thing/door/ornament routes.
 * - DUNVIEW.C F0127_DUNGEONVIEW_DrawSquareD0C lines 8164-8294 is a
 *   follow-up anchor only; this D0L2/D0R2 gate must not duplicate D0C.
 * - DUNVIEW.C F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap lines
 *   3113-3156 and F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlipped-
 *   Horizontally lines 3185-3247 bind native/flipped blits with
 *   C10_COLOR_FLESH transparency.
 * - DEFS.H lines 2088, 2597-2606, 3423-3424, 3428-3429, and 4040-4057
 *   bind C10, M610/M611, D0/D2L2 wall ids, and C716/C717 wall zones.
 * - COORD.C lines 1713-1722 and COMMAND.C lines 1126-1127 bind the
 *   synthetic 320x200 screen, 224x136 viewport, and centered 48-pixel
 *   side margin used only by this contract probe.
 */

#define DM1_V1_D0L2_D0R2_WALL_ROUTE_COUNT_PC34 2
#define DM1_V1_D0L2_D0R2_WALL_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D0L2_D0R2_WALL_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D0L2_D0R2_WALL_SCREEN_WIDTH_PC34 320
#define DM1_V1_D0L2_D0R2_WALL_SCREEN_HEIGHT_PC34 200
#define DM1_V1_D0L2_D0R2_WALL_EDGE_WIDTH_PC34 48
#define DM1_V1_D0L2_D0R2_WALL_C10_COLOR_FLESH_PC34 10
#define DM1_V1_D0L2_D0R2_WALL_PROBE_CHECK_CAPACITY_PC34 16

typedef enum {
    DM1_V1_D0L2_D0R2_WALL_SIDE_D0L2_PC34 = 0,
    DM1_V1_D0L2_D0R2_WALL_SIDE_D0R2_PC34 = 1
} DM1_V1_ViewportD0L2D0R2WallSidePc34;

typedef struct {
    DM1_V1_ViewportD0L2D0R2WallSidePc34 side;
    const char *route_name;
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int f0128_draw_order_index;
    int f0128_redmcsb_depth;
    int f0128_redmcsb_lateral;
    int synthetic_edge_lane;
    int view_square_index;
    int wall_element;
    int native_wall_index;
    int flipped_wall_index;
    int wall_zone;
    int wall_frame_row;
    int wall_frame_left;
    int wall_frame_right;
    int wall_frame_top;
    int wall_frame_bottom;
    int screen_x_first;
    int screen_x_last;
    int source_x_first;
    int source_x_last;
    int transparent_color;
    int uses_f0104_native_blit;
    int uses_f0105_parity_scratch_flip;
    int preserves_c10_transparency;
    int wall_case_returns_before_f0111;
    int wall_case_returns_before_f0115;
    int wall_case_returns_before_f0108;
    int nonduplicative_vs_f0115_gate;
    int nonduplicative_vs_d0l_d0r_wall_gate;
    const char *redmcsb_function_anchor;
    const char *redmcsb_wall_anchor;
    const char *source_lines;
} DM1_V1_ViewportD0L2D0R2WallSpecPc34;

typedef struct {
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *spec;
    int row;
    int screen_x;
    int source_x;
    int viewport_y;
    size_t source_offset;
    size_t screen_offset;
    uint8_t pixel_before;
    uint8_t source_pixel;
    uint8_t pixel_after;
    int in_clip;
    int writes_pixel;
    int transparent_skip;
    int uses_scratch;
    int no_write_metadata;
} DM1_V1_ViewportD0L2D0R2WallPixelPc34;

typedef struct {
    int ok;
    int route_count;
    int assertion_contract_count;
    int focused_test_pass_count;
    int focused_test_failure_count;
    int f0098_row_owned;
    int f0128_dispatch_ok;
    int m610_m611_zone_binding_ok;
    int f0104_native_route_ok;
    int f0105_parity_route_ok;
    int c10_transparency_ok;
    int edge_clip_ok;
    int no_f0111_no_f0115_no_f0108_ok;
    int nonduplicative_ok;
    DM1_V1_ViewportD0L2D0R2WallPixelPc34 checks[
        DM1_V1_D0L2_D0R2_WALL_PROBE_CHECK_CAPACITY_PC34];
    size_t check_count;
    const char *source_evidence;
} DM1_V1_ViewportD0L2D0R2WallProbePc34;

size_t dm1_v1_viewport_d0l2_d0r2_wall_spec_count_pc34(void);

const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *
dm1_v1_viewport_d0l2_d0r2_wall_spec_pc34(
    DM1_V1_ViewportD0L2D0R2WallSidePc34 side);

int dm1_v1_viewport_d0l2_d0r2_wall_map_screen_x_to_source_pc34(
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *spec,
    int screen_x,
    int *out_source_x);

uint8_t dm1_v1_viewport_d0l2_d0r2_wall_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color);

int dm1_v1_viewport_d0l2_d0r2_wall_apply_pixel_pc34(
    const DM1_V1_ViewportD0L2D0R2WallSpecPc34 *spec,
    const uint8_t *source,
    size_t source_len,
    uint8_t *screen,
    size_t screen_len,
    int screen_x,
    int row,
    DM1_V1_ViewportD0L2D0R2WallPixelPc34 *out);

int dm1_v1_viewport_d0l2_d0r2_wall_probe_pc34_compat(
    DM1_V1_ViewportD0L2D0R2WallProbePc34 *out);

const char *dm1_v1_viewport_d0l2_d0r2_wall_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0L2_D0R2_WALL_PC34_COMPAT_H */
