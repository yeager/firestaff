#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2L_D2R_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D2L/D2R door-frame-top edge source-lock probe.
 *
 * ReDMCSB anchors confirmed from the local checkout
 * (Toolchains/Common/Source/DUNVIEW.C, DEFS.H, F0100_DrawWallSetBitmap):
 *
 * - DUNVIEW.C:604-606 defines the three D2 door-frame-top strides as
 *   G0173_auc_Graphic558_Frame_DoorFrameTop_D2L = { 0, 59, 22, 24, 48,
 *   3, 16, 0 }, G0174_auc_Graphic558_Frame_DoorFrameTop_D2C = { 64, 159,
 *   22, 24, 48, 3, 0, 0 }, and G0175_auc_Graphic558_Frame_DoorFrameTop_D2R
 *   = { 164, 223, 22, 24, 48, 3, 16, 0 }. Each stride has X1=0/64/164,
 *   X2=59/159/223, Y1=22, Y2=24, ByteWidth=48, Height=3, X=0/16/16, Y=0.
 *   These strides define the door-frame-top border above the door panel
 *   in the D2 row; Y1=22..Y2=24 is the 3-pixel-tall frame strip that
 *   sits just above the door opening (which itself spans Y1=24..Y2=82
 *   inside G0182/G0184). The D2L/D2R X-offsets (16) reflect the
 *   side-view shift relative to the centered D2C frame.
 *
 * - DUNVIEW.C:6991 (F0119_DUNGEONVIEW_DrawSquareD2L C17_ELEMENT_DOOR_FRONT
 *   MEDIA009_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_G14ED_G20E_
 *   G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G) calls
 *   F0100_DUNGEONVIEW_DrawWallSetBitmap(G0703_puc_Bitmap_WallSet_DoorFrame
 *   Top_D2LCR, G0173_auc_Graphic558_Frame_DoorFrameTop_D2L) which is the
 *   legacy PC 3.4 door-frame-top edge for D2L. F0100 (DUNVIEW.C:3048-3068)
 *   blits the bitmap into G0296_puc_Bitmap_Viewport using the stride
 *   X/Y/ByteWidth and C10_COLOR_FLESH transparency; this gate pins that
 *   contract for D2L.
 *
 * - DUNVIEW.C:6994 (F0119 D2L C17_ELEMENT_DOOR_FRONT MEDIA508_F20E_F20J_
 *   X30J_P20JA_P20JB) calls
 *   F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2114_DoorFrameTopD2L,
 *   C725_ZONE_DOOR_FRAME_TOP_D2L); DUNVIEW.C:6997 (F0119 D2L C17_ELEMENT_
 *   DOOR_FRONT MEDIA720_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_
 *   X31J_P31J) calls F0104(G2114_DoorFrameTopD2L, C729_ZONE_DOOR_FRAME_TOP_
 *   D2L). DUNVIEW.C:7184 (F0120_DUNGEONVIEW_DrawSquareD2R_CPSF C17_ELEMENT_
 *   DOOR_FRONT MEDIA009) calls F0100(G0703, G0175) for D2R. DUNVIEW.C:7187
 *   (F0120 D2R MEDIA508) calls F0104(G2113_DoorFrameTopD2R,
 *   C727_ZONE_DOOR_FRAME_TOP_D2R); DUNVIEW.C:7190 (F0120 D2R MEDIA720)
 *   calls F0104(G2113_DoorFrameTopD2R, C731_ZONE_DOOR_FRAME_TOP_D2R).
 *
 * - DEFS.H:2582-2583 and 2603-2604 declare M604_VIEW_SQUARE_D2L /
 *   M605_VIEW_SQUARE_D2R as 7/8 (the modern view-square indices; the
 *   4/5 row holds the legacy indices, gated by #ifdef MEDIA008).
 *   DEFS.H:4068-4070 (MEDIA508 F20E/F20J block) defines
 *   C725_ZONE_DOOR_FRAME_TOP_D2L = 725, C726_ZONE_DOOR_FRAME_TOP_D2C =
 *   726, C727_ZONE_DOOR_FRAME_TOP_D2R = 727. DEFS.H:4087-4089 (MEDIA720
 *   I34E block) redefines C729_ZONE_DOOR_FRAME_TOP_D2L = 729,
 *   C730_ZONE_DOOR_FRAME_TOP_D2C = 730, C731_ZONE_DOOR_FRAME_TOP_D2R =
 *   731. This gate pins both blocks.
 *
 * - DEFS.H:2088 C10_COLOR_FLESH is the legacy door-frame-top blit
 *   transparency (F0100 path), and the door-frame-top blit also runs
 *   after the F0108 floor-ornament blit (M558_FLOOR_ORNAMENT_ORDINAL)
 *   and the F0115 thing pass1 (C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_
 *   BACKRIGHT for D2L at M604, C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_
 *   BACKLEFT for D2R at M605). The post-frame byte-stability surface
 *   for the door-frame-top edge is the F0111_DUNGEONVIEW_DrawDoor
 *   door-panel state machine which immediately follows the door-frame-
 *   top blit in dispatch order.
 *
 * - DUNVIEW.C:6900 F0119_DUNGEONVIEW_DrawSquareD2L and DUNVIEW.C:7051
 *   F0120_DUNGEONVIEW_DrawSquareD2R_CPSF are the D2L/D2R dispatch
 *   functions; DUNVIEW.C:8513 / 8517 are the F0128 caller sites. The
 *   door-frame-top edge stride (3-pixel-tall horizontal bar above the
 *   door panel) is part of the C17_ELEMENT_DOOR_FRONT draw body in
 *   both functions and is distinct from the C16_ELEMENT_DOOR_SIDE
 *   branch which is the side-wall F0128 MEDIA720 D2L2/D2R2 row guard.
 *
 * - CSB counterpart: test_csb_v1_viewport_d2c_f0111_door_front_pc34_compat
 *   (D2C door-front layering), test_csb_v1_viewport_d2l2_d2r2_f0111_door_
 *   pc34_compat (CSB D2L2/D2R2 door-side F0111 dispatch). Non-overlap
 *   DM1 siblings: dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_
 *   compat (D2L/D2R F0111 partly-open half-blit body, distinct contract),
 *   dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat (D2L2/
 *   D2R2 front-door pair), dm1_v1_viewport_d0c_door_edge_ornament_pc34_
 *   compat (D0C door-frame border + thieves-eye hole), and
 *   dm1_v1_viewport_d2l_d2r_wall_pc34_compat (D2L/D2R wall content).
 *
 * Synthetic 320x200 framebuffer with a 224x136 viewport; no real-asset
 * bitmap parity, no original-DOS pixel claim. This gate intentionally
 * targets the door-frame-top edge contract only; the F0111 door-panel
 * state machine is a distinct contract covered by
 * dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat.
 *
 * Non-overlap marker: pass794-d2l-d2r-door-frame-top-edge-source-lock.
 */

#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34 10

/* G0173_auc_Graphic558_Frame_DoorFrameTop_D2L = { 0, 59, 22, 24, 48, 3,
 * 16, 0 } (DUNVIEW.C:604). */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_LEFT_X_PC34 0
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_RIGHT_X_PC34 59
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_TOP_Y_PC34 22
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BOTTOM_Y_PC34 24
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_BYTE_WIDTH_PC34 48
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_HEIGHT_PC34 3
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_X_OFFSET_PC34 16
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_STRIDE_Y_OFFSET_PC34 0

/* G0175_auc_Graphic558_Frame_DoorFrameTop_D2R = { 164, 223, 22, 24, 48,
 * 3, 16, 0 } (DUNVIEW.C:606). */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_LEFT_X_PC34 164
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_RIGHT_X_PC34 223
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_TOP_Y_PC34 22
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BOTTOM_Y_PC34 24
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_BYTE_WIDTH_PC34 48
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_HEIGHT_PC34 3
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_X_OFFSET_PC34 16
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_STRIDE_Y_OFFSET_PC34 0

/* Common 3-pixel-tall frame strip band shared by D2L, D2C, D2R. */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34 22
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_BOTTOM_Y_PC34 24
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_HEIGHT_PC34 3
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_BAND_BYTE_WIDTH_PC34 48

/* View-square indices (DEFS.H:2603-2604 modern block; 2582-2583 legacy
 * block redefined inside #ifdef MEDIA008). */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M604_VIEW_SQUARE_D2L_PC34 7
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M605_VIEW_SQUARE_D2R_PC34 8

/* Cell-order anchors (DUNVIEW.C:6990 D2L, 7183 D2R). */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2L_PASS1_CELL_ORDER_PC34 0x0218
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_D2R_PASS1_CELL_ORDER_PC34 0x0128

/* Native door-panel bitmaps (F0111_DUNGEONVIEW_DrawDoor dispatch). */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0694_DOOR_BITMAP_PC34 694
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0182_DOOR_FRAMES_D2L_PC34 182
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0184_DOOR_FRAMES_D2R_PC34 184
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C1_VIEW_DOOR_ORNAMENT_D2LCR_PC34 1

/* F0119 / F0120 dispatch anchors. */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_LINE_START_PC34 6900
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRONT_LINE_PC34 6987
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_LEGACY_LINE_PC34 6991
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_F20E_LINE_PC34 6994
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0119_D2L_DOOR_FRAME_TOP_I34E_LINE_PC34 6997
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_LINE_START_PC34 7051
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRONT_LINE_PC34 7180
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_LEGACY_LINE_PC34 7184
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_F20E_LINE_PC34 7187
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0120_D2R_DOOR_FRAME_TOP_I34E_LINE_PC34 7190
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34 8513

/* DEFS.H zone IDs (both blocks). */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C725_ZONE_D2L_F20E_PC34 725
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C726_ZONE_D2C_F20E_PC34 726
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C727_ZONE_D2R_F20E_PC34 727
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C729_ZONE_D2L_I34E_PC34 729
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C730_ZONE_D2C_I34E_PC34 730
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_C731_ZONE_D2R_I34E_PC34 731

/* Bitmap / view-square identifiers. */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0703_DOOR_FRAME_TOP_BITMAP_PC34 703
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0173_STRIDE_PC34 173
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G0175_STRIDE_PC34 175
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G2114_DOOR_FRAME_TOP_D2L_PC34 2114
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G2113_DOOR_FRAME_TOP_D2R_PC34 2113
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_G2115_DOOR_FRAME_TOP_D2LCR_PC34 2115

/* Door-panel half-height anchor used by the F0111 dispatch following
 * the door-frame-top blit. */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_M075_BITMAP_BYTE_COUNT_D2LCR_PC34 3904
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34 24
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34 82
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34 61
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_WIDTH_PC34 32

/* Forward-declared hash slot — concrete value is set by the source
 * module after the source evidence string is finalized. */
#define DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_HASH_PC34 0x2757CC4Du

typedef enum {
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34 = 0,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34 = 1,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34 = 2,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_TARGET_INVALID_PC34 = -1
} DM1_V1_D2L_D2RDoorFrameTopEdgeTargetPc34;

typedef enum {
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34 = 0,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34 = 1,
    DM1_V1_D2L_D2R_DOOR_FRAME_TOP_EDGE_SIDE_INVALID_PC34 = -1
} DM1_V1_D2L_D2RDoorFrameTopEdgeSidePc34;

typedef struct {
    int side;
    int target_media;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int d2l_stride_left_x;
    int d2l_stride_right_x;
    int d2l_stride_top_y;
    int d2l_stride_bottom_y;
    int d2l_stride_byte_width;
    int d2l_stride_height;
    int d2l_stride_x_offset;
    int d2l_stride_y_offset;
    int d2r_stride_left_x;
    int d2r_stride_right_x;
    int d2r_stride_top_y;
    int d2r_stride_bottom_y;
    int d2r_stride_byte_width;
    int d2r_stride_height;
    int d2r_stride_x_offset;
    int d2r_stride_y_offset;
    int band_top_y;
    int band_bottom_y;
    int band_height;
    int band_byte_width;
    int m604_view_square_d2l;
    int m605_view_square_d2r;
    int d2l_pass1_cell_order;
    int d2r_pass1_cell_order;
    int door_panel_top_y;
    int door_panel_bottom_y;
    int door_panel_height;
    int door_panel_byte_width;
    int bitmap_byte_count_d2lcr;
    int door_frame_top_bitmap_id;
    int door_frame_top_stride_d2l_id;
    int door_frame_top_stride_d2r_id;
    int door_frame_top_native_bitmap_d2l;
    int door_frame_top_native_bitmap_d2r;
    int door_frame_top_native_bitmap_d2lcr;
    int f0100_blit_transparency_color;
    int f0119_d2l_door_front_line;
    int f0119_d2l_door_frame_top_legacy_line;
    int f0119_d2l_door_frame_top_f20e_line;
    int f0119_d2l_door_frame_top_i34e_line;
    int f0120_d2r_door_front_line;
    int f0120_d2r_door_frame_top_legacy_line;
    int f0120_d2r_door_frame_top_f20e_line;
    int f0120_d2r_door_frame_top_i34e_line;
    int f0128_dispatch_line;
    int d2l_door_frame_top_zone;
    int d2r_door_frame_top_zone;
    int g0694_door_panel_bitmap;
    int g0182_door_frames_d2l;
    int g0184_door_frames_d2r;
    int c1_view_door_ornament_d2lcr;
    int d2l_band_in_viewport;
    int d2r_band_in_viewport;
    int d2l_band_inside_door_panel_band;
    int d2r_band_inside_door_panel_band;
    int d2l_legacy_route_uses_g0703;
    int d2r_legacy_route_uses_g0703;
    int d2l_f20e_route_uses_g2114;
    int d2r_f20e_route_uses_g2113;
    int d2l_i34e_route_uses_g2114;
    int d2r_i34e_route_uses_g2113;
    int c10_transparent_blit;
    int band_strip_destination_x_d2l;
    int band_strip_destination_x_d2r;
    int band_strip_destination_y;
    int band_strip_byte_width_d2l;
    int band_strip_byte_width_d2r;
    uint8_t d2l_first_probe_pixel;
    uint8_t d2l_second_probe_pixel;
    uint8_t d2l_third_probe_pixel;
    uint8_t d2r_first_probe_pixel;
    uint8_t d2r_second_probe_pixel;
    uint8_t d2r_third_probe_pixel;
} DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d2l_legacy_zone_count;
    int d2l_f20e_zone_count;
    int d2l_i34e_zone_count;
    int d2r_legacy_zone_count;
    int d2r_f20e_zone_count;
    int d2r_i34e_zone_count;
    int invalid_target_count;
    int stride_g0173_checks;
    int stride_g0175_checks;
    int band_strip_checks;
    int zone_id_family_checks;
    int door_panel_post_band_checks;
    int view_square_anchor_checks;
    int non_overlap_checks;
    int bitmap_route_checks;
    int c10_transparency_checks;
} DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34;

int dm1_v1_viewport_d2l_d2r_door_frame_top_edge_trace_pc34(
    int side,
    int target_media,
    DM1_V1_D2L_D2RDoorFrameTopEdgeTracePc34 *out_trace);

const char *
dm1_v1_viewport_d2l_d2r_door_frame_top_edge_source_evidence_pc34(void);

int run_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_self_test(void);

const DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d2l_d2r_door_frame_top_edge_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
