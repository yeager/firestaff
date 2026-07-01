#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1C_D2C_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1C_D2C_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1C/D2C door-frame-top edge source-lock probe.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:605 G0174 = {64,159,22,24,48,3,0,0}; DUNVIEW.C:608
 *   G0177 = {48,175,14,17,64,4,0,0}.
 * - DUNVIEW.C:7244 F0121_DUNGEONVIEW_DrawSquareD2C start;
 *   DUNVIEW.C:7313/7317/7323/7328 C17_ELEMENT_DOOR_FRONT, legacy
 *   F0100(G0703,G0174), F20E F0104(G2115,C726), and I34E
 *   F0104(G2115,C730).
 * - DUNVIEW.C:7727 F0124_DUNGEONVIEW_DrawSquareD1C start;
 *   DUNVIEW.C:7873/7877/7882/7886 C17_ELEMENT_DOOR_FRONT, legacy
 *   F0100(G0704,G0177), F20E F0104(G2112,C729), and I34E
 *   F0104(G2112,C733).
 * - DEFS.H:2581/2584 and 2599/2602 define M603/M606 view-square
 *   indices; DEFS.H:2669 pins C0x0218 cell order; DEFS.H:4068-4073
 *   and 4087-4093 define the F20E/I34E door-frame-top zone families.
 *
 * The fixture is data-free and does not claim real-asset or original DOS
 * pixel parity. It closes the center-row sibling hole left by the D1L/D1R
 * and D2L/D2R door-frame-top edge gates.
 */

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34 10

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_LEFT_X_PC34 64
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_RIGHT_X_PC34 159
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_TOP_Y_PC34 22
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_BOTTOM_Y_PC34 24
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_BYTE_WIDTH_PC34 48
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_HEIGHT_PC34 3
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_X_OFFSET_PC34 0
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_Y_OFFSET_PC34 0

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_LEFT_X_PC34 48
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_RIGHT_X_PC34 175
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_TOP_Y_PC34 14
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_BOTTOM_Y_PC34 17
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_BYTE_WIDTH_PC34 64
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_HEIGHT_PC34 4
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_X_OFFSET_PC34 0
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_Y_OFFSET_PC34 0

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_M603_D2C_MODERN_PC34 6
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_M606_D1C_MODERN_PC34 3
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_PASS1_CELL_ORDER_PC34 0x0218

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_F0121_START_LINE_PC34 7244
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_DOOR_FRONT_LINE_PC34 7313
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_LEGACY_LINE_PC34 7317
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_F20E_LINE_PC34 7323
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_I34E_LINE_PC34 7328
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D2C_F0128_LINE_PC34 8521

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_F0124_START_LINE_PC34 7727
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_DOOR_FRONT_LINE_PC34 7873
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_LEGACY_LINE_PC34 7877
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_F20E_LINE_PC34 7882
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_I34E_LINE_PC34 7886
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_D1C_F0128_LINE_PC34 8533

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0703_D2_TOP_BITMAP_PC34 703
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0704_D1_TOP_BITMAP_PC34 704
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0174_D2C_STRIDE_PC34 174
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G0177_D1C_STRIDE_PC34 177
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G2115_D2_TOP_NATIVE_PC34 2115
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_G2112_D1_TOP_NATIVE_PC34 2112

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C726_D2C_F20E_PC34 726
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C730_D2C_I34E_PC34 730
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C729_D1C_F20E_PC34 729
#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_C733_D1C_I34E_PC34 733

#define DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_HASH_PC34 0x1FC6BE62u

typedef enum {
    DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D2C_PC34 = 0,
    DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_SQUARE_D1C_PC34 = 1
} DM1_V1_D1C_D2CDoorFrameTopEdgeSquarePc34;

typedef enum {
    DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34 = 0,
    DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34 = 1,
    DM1_V1_D1C_D2C_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34 = 2
} DM1_V1_D1C_D2CDoorFrameTopEdgeTargetPc34;

typedef struct {
    int square;
    int target_media;
    int left_x;
    int right_x;
    int top_y;
    int bottom_y;
    int byte_width;
    int height;
    int x_offset;
    int y_offset;
    int view_square;
    int pass1_cell_order;
    int legacy_bitmap_id;
    int legacy_stride_id;
    int native_bitmap_id;
    int zone_id;
    int line_start;
    int door_front_line;
    int legacy_line;
    int f20e_line;
    int i34e_line;
    int f0128_line;
    int band_inside_viewport;
    int center_has_no_side_shift;
    int c10_transparency;
} DM1_V1_D1C_D2CDoorFrameTopEdgeTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d2c_legacy_count;
    int d2c_f20e_count;
    int d2c_i34e_count;
    int d1c_legacy_count;
    int d1c_f20e_count;
    int d1c_i34e_count;
    int invalid_count;
    int stride_checks;
    int zone_checks;
    int dispatch_checks;
    int order_checks;
    int non_overlap_checks;
    int bitmap_route_checks;
    int viewport_band_checks;
} DM1_V1_D1C_D2CDoorFrameTopEdgeSelfTestResultPc34;

int dm1_v1_viewport_d1c_d2c_door_frame_top_edge_trace_pc34(
    int square,
    int target_media,
    DM1_V1_D1C_D2CDoorFrameTopEdgeTracePc34 *out_trace);

const char *
dm1_v1_viewport_d1c_d2c_door_frame_top_edge_source_evidence_pc34(void);

int run_dm1_v1_viewport_d1c_d2c_door_frame_top_edge_self_test(void);

const DM1_V1_D1C_D2CDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d1c_d2c_door_frame_top_edge_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
