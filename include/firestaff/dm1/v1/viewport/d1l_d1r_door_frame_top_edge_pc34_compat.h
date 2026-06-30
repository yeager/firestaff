#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D1L_D1R_DOOR_FRAME_TOP_EDGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contract-only DM1 V1 D1L/D1R door-frame-top edge source-lock probe.
 *
 * ReDMCSB anchors confirmed from the local checkout
 * (Toolchains/Common/Source/DUNVIEW.C, DEFS.H, F0100_DrawWallSetBitmap,
 * F0104_DrawFloorPitOrStairsBitmap):
 *
 * - DUNVIEW.C:607-609 defines the three D1 door-frame-top strides as
 *   G0176_auc_Graphic558_Frame_DoorFrameTop_D1L = { 0, 31, 14, 17, 64,
 *   4, 16, 0 }, G0177_auc_Graphic558_Frame_DoorFrameTop_D1C = { 48,
 *   175, 14, 17, 64, 4, 0, 0 }, and G0178_auc_Graphic558_Frame_
 *   DoorFrameTop_D1R = { 192, 223, 14, 17, 64, 4, 16, 0 }. Each stride
 *   has X1=0/48/192, X2=31/175/223, Y1=14, Y2=17, ByteWidth=64, Height=4,
 *   X=16/0/16, Y=0. These strides define the door-frame-top border
 *   above the door panel in the D1 row; Y1=14..Y2=17 is the 4-pixel-tall
 *   frame strip that sits just above the door opening (which itself
 *   spans Y1=17..Y2=85 inside G0185/G0187). The D1L/D1R X-offsets (16)
 *   reflect the side-view shift relative to the centered D1C frame.
 *
 * - DUNVIEW.C:7496 (F0122_DUNGEONVIEW_DrawSquareD1L C17_ELEMENT_DOOR_
 *   FRONT MEDIA009_S10EA_S10EB_S11E_S12E_S12G_S13FA_S13FB_S20E_S21E_
 *   G14ED_G20E_G21E_A20ED_A20E_A20F_A20G_A21E_A22E_A22G) calls
 *   F0100_DUNGEONVIEW_DrawWallSetBitmap(G0704_puc_Bitmap_WallSet_
 *   DoorFrameTop_D1LCR, G0176_auc_Graphic558_Frame_DoorFrameTop_D1L)
 *   which is the legacy PC 3.4 door-frame-top edge for D1L. F0100
 *   (DUNVIEW.C:3048-3068) blits the bitmap into G0296_puc_Bitmap_
 *   Viewport using the stride X/Y/ByteWidth and C10_COLOR_FLESH
 *   transparency; this gate pins that contract for D1L.
 *
 * - DUNVIEW.C:7500 (F0122 D1L C17_ELEMENT_DOOR_FRONT MEDIA508_F20E_
 *   F20J_X30J_P20JA_P20JB) calls F0104_DUNGEONVIEW_DrawFloorPitOr
 *   StairsBitmap(G2111_DoorFrameTopD1L, C728_ZONE_DOOR_FRAME_TOP_D1L).
 *   DUNVIEW.C:7503 (F0122 D1L C17_ELEMENT_DOOR_FRONT MEDIA720_I34E_
 *   I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J) calls
 *   F0104(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L).
 *   DUNVIEW.C:7664 (F0123_DUNGEONVIEW_DrawSquareD1R C17_ELEMENT_DOOR_
 *   FRONT MEDIA009) calls F0100(G0704, G0178) for D1R. DUNVIEW.C:7668
 *   (F0123 D1R MEDIA508) calls F0104(G2110_DoorFrameTopD1R,
 *   C730_ZONE_DOOR_FRAME_TOP_D1R); DUNVIEW.C:7671 (F0123 D1R
 *   MEDIA720) calls F0104(G2110_DoorFrameTopD1R, C734_ZONE_DOOR_
 *   FRAME_TOP_D1R).
 *
 * - DEFS.H:2585-2586 (modern block, view-square indices 7/8) and
 *   DEFS.H:2600-2601 (legacy MEDIA008 block, indices 4/5) declare
 *   M607_VIEW_SQUARE_D1L / M608_VIEW_SQUARE_D1R; this gate pins the
 *   modern indices 7/8 used by the F0115 thing pass1 dispatch inside
 *   F0122/F0123. DEFS.H:4068-4073 (MEDIA508 block) defines C725/
 *   C726/C727 (D2) and C728_ZONE_DOOR_FRAME_TOP_D1L = 728, C729_ZONE_
 *   DOOR_FRAME_TOP_D1C = 729, C730_ZONE_DOOR_FRAME_TOP_D1R = 730.
 *   DEFS.H:4087-4093 (MEDIA720 block) defines C729/C730/C731 (D2)
 *   and C732_ZONE_DOOR_FRAME_TOP_D1L = 732, C733_ZONE_DOOR_FRAME_TOP_
 *   D1C = 733, C734_ZONE_DOOR_FRAME_TOP_D1R = 734. This gate pins
 *   C728/C730 (F20E) and C732/C734 (I34E) for the D1L/D1R side strip;
 *   C729/C733 belong to the D1C parallel gate and are NOT consumed
 *   here.
 *
 * - DEFS.H:2088 C10_COLOR_FLESH is the legacy door-frame-top blit
 *   transparency (F0100 path). The door-frame-top blit runs after the
 *   F0108 floor-ornament blit (M594_VIEW_FLOOR_D1L / M596_VIEW_FLOOR_
 *   D1R) and the F0115 thing pass1 (C0x0028_CELL_ORDER_DOORPASS1_
 *   BACKRIGHT at M607, C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT at M608).
 *   The post-frame byte-stability surface for the door-frame-top edge
 *   is the F0111_DUNGEONVIEW_DrawDoor door-panel state machine which
 *   immediately follows the door-frame-top blit in dispatch order
 *   (G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, G0185_s_Graphic558_
 *   Frames_Door_D1L / G0187_s_Graphic558_Frames_Door_D1R).
 *
 * - DUNVIEW.C:7391 F0122_DUNGEONVIEW_DrawSquareD1L and DUNVIEW.C:7559
 *   F0123_DUNGEONVIEW_DrawSquareD1R are the D1L/D1R dispatch
 *   functions; DUNVIEW.C:8525 / 8529 are the F0128 caller sites. The
 *   door-frame-top edge stride (4-pixel-tall horizontal bar above the
 *   door panel) is part of the C17_ELEMENT_DOOR_FRONT draw body in
 *   both functions and is distinct from the C16_ELEMENT_DOOR_SIDE
 *   branch which is the side-wall F0128 MEDIA720 D1L2/D1R2 row guard.
 *
 * - CSB counterpart: csb_v1_viewport_d1c_f0111_door_front_pc34_compat
 *   (D1C door-front layering), csb_v1_viewport_d1l2_d1r2_f0111_door_
 *   pc34_compat (CSB D1L2/D1R2 door-side F0111 dispatch). Non-overlap
 *   DM1 siblings: dm1_v1_viewport_d2l_d2r_door_frame_top_edge_pc34_
 *   compat (D2L/D2R door-frame-top edge, distinct Y1=22..Y2=24 band
 *   and G0173/G0175 strides), dm1_v1_viewport_d2c_door_frame_top_
 *   edge_pc34_compat (D2C door-frame-top edge, distinct stride and
 *   zones C726/C730), dm1_v1_viewport_d1c_door_frame_top_edge_pc34_
 *   compat (D1C door-frame-top edge, distinct G0177 stride and
 *   C729/C733 zones, F0124 dispatch), dm1_v1_viewport_d1l_d1r_f0111_
 *   partly_open_door_pc34_compat (D1L/D1R F0111 partly-open half-blit
 *   body, distinct contract), dm1_v1_viewport_d1c_f0111_partly_open_
 *   door_pc34_compat (D1C F0111 partly-open), dm1_v1_viewport_d0c_
 *   door_edge_ornament_pc34_compat (D0C door-frame border + thieves-
 *   eye hole).
 *
 * Synthetic 320x200 framebuffer with a 224x136 viewport; no real-asset
 * bitmap parity, no original-DOS pixel claim. This gate intentionally
 * targets the door-frame-top edge contract only; the F0111 door-panel
 * state machine is a distinct contract covered by dm1_v1_viewport_
 * d1l_d1r_f0111_partly_open_door_pc34_compat.
 *
 * Non-overlap marker: pass794-d1l-d1r-door-frame-top-edge-source-lock.
 */

#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_WIDTH_PC34 320
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_FRAMEBUFFER_HEIGHT_PC34 200
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_VIEWPORT_WIDTH_PC34 224
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_VIEWPORT_HEIGHT_PC34 136
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C10_COLOR_FLESH_PC34 10

/* G0176_auc_Graphic558_Frame_DoorFrameTop_D1L = { 0, 31, 14, 17, 64,
 * 4, 16, 0 } (DUNVIEW.C:607). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_LEFT_X_PC34 0
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_RIGHT_X_PC34 31
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_TOP_Y_PC34 14
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BOTTOM_Y_PC34 17
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_BYTE_WIDTH_PC34 64
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_HEIGHT_PC34 4
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_X_OFFSET_PC34 16
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_STRIDE_Y_OFFSET_PC34 0

/* G0178_auc_Graphic558_Frame_DoorFrameTop_D1R = { 192, 223, 14, 17,
 * 64, 4, 16, 0 } (DUNVIEW.C:609). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_LEFT_X_PC34 192
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_RIGHT_X_PC34 223
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_TOP_Y_PC34 14
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BOTTOM_Y_PC34 17
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_BYTE_WIDTH_PC34 64
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_HEIGHT_PC34 4
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_X_OFFSET_PC34 16
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_STRIDE_Y_OFFSET_PC34 0

/* Common 4-pixel-tall frame strip band shared by D1L, D1C, D1R. */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_TOP_Y_PC34 14
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_BOTTOM_Y_PC34 17
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_HEIGHT_PC34 4
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_BAND_BYTE_WIDTH_PC34 64

/* View-square indices (DEFS.H:2585-2586 modern block; 2600-2601 legacy
 * block redefined inside #ifdef MEDIA008). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M607_VIEW_SQUARE_D1L_PC34 7
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M608_VIEW_SQUARE_D1R_PC34 8

/* Cell-order anchors (DUNVIEW.C:7495 D1L, 7663 D1R). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1L_PASS1_CELL_ORDER_PC34 0x0028
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_D1R_PASS1_CELL_ORDER_PC34 0x0018

/* Native door-panel bitmaps (F0111_DUNGEONVIEW_DrawDoor dispatch). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0695_DOOR_BITMAP_PC34 695
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0185_DOOR_FRAMES_D1L_PC34 185
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0187_DOOR_FRAMES_D1R_PC34 187
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C2_VIEW_DOOR_ORNAMENT_D1LCR_PC34 2

/* F0122 / F0123 dispatch anchors. */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_LINE_START_PC34 7391
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRONT_LINE_PC34 7494
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_LEGACY_LINE_PC34 7496
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_F20E_LINE_PC34 7500
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0122_D1L_DOOR_FRAME_TOP_I34E_LINE_PC34 7503
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_LINE_START_PC34 7559
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRONT_LINE_PC34 7662
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_LEGACY_LINE_PC34 7664
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_F20E_LINE_PC34 7668
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0123_D1R_DOOR_FRAME_TOP_I34E_LINE_PC34 7671
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_F0128_DISPATCH_LINE_PC34 8525

/* DEFS.H zone IDs (both blocks; D1L/D1R only — C729/C733 belong to
 * the D1C parallel gate). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C728_ZONE_D1L_F20E_PC34 728
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C730_ZONE_D1R_F20E_PC34 730
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C732_ZONE_D1L_I34E_PC34 732
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_C734_ZONE_D1R_I34E_PC34 734

/* Bitmap / view-square identifiers. */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0704_DOOR_FRAME_TOP_BITMAP_PC34 704
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0176_STRIDE_PC34 176
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G0178_STRIDE_PC34 178
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G2111_DOOR_FRAME_TOP_D1L_PC34 2111
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G2110_DOOR_FRAME_TOP_D1R_PC34 2110
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_G2112_DOOR_FRAME_TOP_D1LCR_PC34 2112

/* Door-panel half-height anchor used by the F0111 dispatch following
 * the door-frame-top blit. The D1 door panel is 96x88 = 4224 bytes
 * (M075_BITMAP_BYTE_COUNT(96, 88)). */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_M075_BITMAP_BYTE_COUNT_D1LCR_PC34 4224
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_TOP_Y_PC34 17
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BOTTOM_Y_PC34 102
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_HEIGHT_PC34 88
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_DOOR_PANEL_BYTE_WIDTH_PC34 48

/* Forward-declared hash slot — concrete value is set by the source
 * module after the source evidence string is finalized. */
#define DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_HASH_PC34 0x4811F8C7u

typedef enum {
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_LEGACY_PC34 = 0,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_F20E_PC34 = 1,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_I34E_PC34 = 2,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_TARGET_INVALID_PC34 = -1
} DM1_V1_D1L_D1RDoorFrameTopEdgeTargetPc34;

typedef enum {
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_LEFT_PC34 = 0,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_RIGHT_PC34 = 1,
    DM1_V1_D1L_D1R_DOOR_FRAME_TOP_EDGE_SIDE_INVALID_PC34 = -1
} DM1_V1_D1L_D1RDoorFrameTopEdgeSidePc34;

typedef struct {
    int side;
    int target_media;
    int framebuffer_width;
    int framebuffer_height;
    int viewport_width;
    int viewport_height;
    int d1l_stride_left_x;
    int d1l_stride_right_x;
    int d1l_stride_top_y;
    int d1l_stride_bottom_y;
    int d1l_stride_byte_width;
    int d1l_stride_height;
    int d1l_stride_x_offset;
    int d1l_stride_y_offset;
    int d1r_stride_left_x;
    int d1r_stride_right_x;
    int d1r_stride_top_y;
    int d1r_stride_bottom_y;
    int d1r_stride_byte_width;
    int d1r_stride_height;
    int d1r_stride_x_offset;
    int d1r_stride_y_offset;
    int band_top_y;
    int band_bottom_y;
    int band_height;
    int band_byte_width;
    int m607_view_square_d1l;
    int m608_view_square_d1r;
    int d1l_pass1_cell_order;
    int d1r_pass1_cell_order;
    int door_panel_top_y;
    int door_panel_bottom_y;
    int door_panel_height;
    int door_panel_byte_width;
    int bitmap_byte_count_d1lcr;
    int door_frame_top_bitmap_id;
    int door_frame_top_stride_d1l_id;
    int door_frame_top_stride_d1r_id;
    int door_frame_top_native_bitmap_d1l;
    int door_frame_top_native_bitmap_d1r;
    int door_frame_top_native_bitmap_d1lcr;
    int f0100_blit_transparency_color;
    int f0122_d1l_door_front_line;
    int f0122_d1l_door_frame_top_legacy_line;
    int f0122_d1l_door_frame_top_f20e_line;
    int f0122_d1l_door_frame_top_i34e_line;
    int f0123_d1r_door_front_line;
    int f0123_d1r_door_frame_top_legacy_line;
    int f0123_d1r_door_frame_top_f20e_line;
    int f0123_d1r_door_frame_top_i34e_line;
    int f0128_dispatch_line;
    int d1l_door_frame_top_zone;
    int d1r_door_frame_top_zone;
    int g0695_door_panel_bitmap;
    int g0185_door_frames_d1l;
    int g0187_door_frames_d1r;
    int c2_view_door_ornament_d1lcr;
    int d1l_band_in_viewport;
    int d1r_band_in_viewport;
    int d1l_band_inside_door_panel_band;
    int d1r_band_inside_door_panel_band;
    int d1l_legacy_route_uses_g0704;
    int d1r_legacy_route_uses_g0704;
    int d1l_f20e_route_uses_g2111;
    int d1r_f20e_route_uses_g2110;
    int d1l_i34e_route_uses_g2111;
    int d1r_i34e_route_uses_g2110;
    int c10_transparent_blit;
    int band_strip_destination_x_d1l;
    int band_strip_destination_x_d1r;
    int band_strip_destination_y;
    int band_strip_byte_width_d1l;
    int band_strip_byte_width_d1r;
    uint8_t d1l_first_probe_pixel;
    uint8_t d1l_second_probe_pixel;
    uint8_t d1l_third_probe_pixel;
    uint8_t d1r_first_probe_pixel;
    uint8_t d1r_second_probe_pixel;
    uint8_t d1r_third_probe_pixel;
} DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34;

typedef struct {
    int assertions;
    int failures;
    uint32_t deterministic_hash;
    int d1l_legacy_zone_count;
    int d1l_f20e_zone_count;
    int d1l_i34e_zone_count;
    int d1r_legacy_zone_count;
    int d1r_f20e_zone_count;
    int d1r_i34e_zone_count;
    int invalid_target_count;
    int stride_g0176_checks;
    int stride_g0178_checks;
    int band_strip_checks;
    int zone_id_family_checks;
    int door_panel_post_band_checks;
    int view_square_anchor_checks;
    int non_overlap_checks;
    int bitmap_route_checks;
    int c10_transparency_checks;
} DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34;

int dm1_v1_viewport_d1l_d1r_door_frame_top_edge_trace_pc34(
    int side,
    int target_media,
    DM1_V1_D1L_D1RDoorFrameTopEdgeTracePc34 *out_trace);

const char *
dm1_v1_viewport_d1l_d1r_door_frame_top_edge_source_evidence_pc34(void);

int run_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_self_test(void);

const DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34 *
dm1_v1_viewport_d1l_d1r_door_frame_top_edge_last_self_test_result_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
