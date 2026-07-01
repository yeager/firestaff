#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2C door-frame-top edge source-lock probe.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:605 G0174 = { 64, 159, 22, 24, 48, 3, 0, 0 }.
 * - DUNVIEW.C:7244 F0121_DUNGEONVIEW_DrawSquareD2C start.
 * - DUNVIEW.C:7313-7339 C17_ELEMENT_DOOR_FRONT draw order:
 *   F0108 floor ornament, F0115 pass1, G0703/G0174 legacy top frame,
 *   G2115/C726 F20E top frame, G2115/C730 I34E top frame, optional
 *   C2_VIEW_DOOR_BUTTON_D2C, then F0111 door panel.
 * - DUNVIEW.C:8520-8521 F0128 dispatch to F0121 at relative (2, 0).
 * - DEFS.H:2602 M603_VIEW_SQUARE_D2C = 6; DEFS.H:4069 / 4088 define
 *   C726/C730 door-frame-top D2C zones.
 *
 * Synthetic 320x200 framebuffer contract only: no real-asset load,
 * no DOSBox capture, and no original-vs-Firestaff pixel parity claim.
 */

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34 10

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_LEFT_X_PC34 64
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_RIGHT_X_PC34 159
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TOP_Y_PC34 22
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BOTTOM_Y_PC34 24
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BYTE_WIDTH_PC34 48
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_HEIGHT_PC34 3
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_STRIDE_X_OFFSET_PC34 0
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_STRIDE_Y_OFFSET_PC34 0

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_M603_VIEW_SQUARE_D2C_PC34 6
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_PASS1_CELL_ORDER_PC34 0x0218
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_PASS2_CELL_ORDER_PC34 0x0349

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0703_BITMAP_PC34 703
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0174_STRIDE_PC34 174
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G2115_NATIVE_TOP_PC34 2115
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G2118_NATIVE_LEFT_PC34 2118
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0694_DOOR_BITMAP_PC34 694
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_G0183_DOOR_FRAMES_D2C_PC34 183
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C1_VIEW_DOOR_ORNAMENT_PC34 1
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C2_VIEW_DOOR_BUTTON_PC34 2

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_F0121_LINE_START_PC34 7244
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_FRONT_LINE_PC34 7313
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_FLOOR_ORNAMENT_LINE_PC34 7314
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_THING_PASS1_LINE_PC34 7315
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_LEGACY_LINE_PC34 7317
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_F20E_LINE_PC34 7323
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_I34E_LINE_PC34 7328
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BUTTON_BRANCH_LINE_PC34 7332
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_BUTTON_DRAW_LINE_PC34 7333
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_LEGACY_LINE_PC34 7336
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_MODERN_LINE_PC34 7339
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34 8521

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C726_ZONE_F20E_PC34 726
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_C730_ZONE_I34E_PC34 730
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_M628_ZONE_DOOR_D2C_PC34 3760

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34 24
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34 82
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34 61
#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_COUNT_PC34 3904

#define DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_HASH_PC34 0xea688f88u

typedef enum {
    DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34 = 0,
    DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34 = 1,
    DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34 = 2,
    DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_TARGET_INVALID_PC34 = -1
} DM1_V1_D2CDoorFrameTopEdgeTargetPc34;

typedef struct {
    int target_media;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int stride_left_x;
    int stride_right_x;
    int stride_top_y;
    int stride_bottom_y;
    int stride_byte_width;
    int stride_height;
    int stride_x_offset;
    int stride_y_offset;
    int m603_view_square_d2c;
    int pass1_cell_order;
    int pass2_cell_order;
    int selected_zone;
    int selected_bitmap;
    int selected_uses_f0100;
    int selected_uses_f0104;
    int door_button_view_index;
    int door_panel_top_y;
    int door_panel_bottom_y;
    int door_panel_height;
    int door_panel_byte_count;
    int f0121_line_start;
    int door_front_line;
    int floor_ornament_line;
    int thing_pass1_line;
    int legacy_line;
    int f20e_line;
    int i34e_line;
    int button_branch_line;
    int button_draw_line;
    int door_panel_legacy_line;
    int door_panel_modern_line;
    int f0128_dispatch_line;
    int g0703_bitmap;
    int g0174_stride;
    int g2115_native_top;
    int g2118_native_left;
    int g0694_door_bitmap;
    int g0183_door_frames_d2c;
    int c1_view_door_ornament;
    int c10_transparency;
    int top_edge_inside_viewport;
    int top_edge_above_door_panel;
    uint8_t first_probe_pixel;
    uint8_t second_probe_pixel;
    uint8_t third_probe_pixel;
} DM1_V1_D2CDoorFrameTopEdgeTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int legacy_route_count;
    int f20e_route_count;
    int i34e_route_count;
    int invalid_target_count;
    int stride_checks;
    int zone_checks;
    int dispatch_order_checks;
    int button_branch_checks;
    int post_band_checks;
    int non_overlap_checks;
} DM1_V1_D2CDoorFrameTopEdgeSelfTestResultPc34;

int dm1_v1_viewport_d2c_door_frame_top_edge_trace_pc34(
    int target_media,
    DM1_V1_D2CDoorFrameTopEdgeTracePc34 *out_trace);

const char *
dm1_v1_viewport_d2c_door_frame_top_edge_source_evidence_pc34(void);

int run_dm1_v1_viewport_d2c_door_frame_top_edge_self_test(void);

const DM1_V1_D2CDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d2c_door_frame_top_edge_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
