/*
 * DM1 V1 PC 3.4 D2L/D2R door-frame-top edge contract test.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:604-606 G0173_auc_Graphic558_Frame_DoorFrameTop_D2L
 *   = { 0, 59, 22, 24, 48, 3, 16, 0 } and G0175_auc_Graphic558_Frame_
 *   DoorFrameTop_D2R = { 164, 223, 22, 24, 48, 3, 16, 0 } define the
 *   D2L/D2R door-frame-top strides; the band is a 3-pixel-tall horizontal
 *   strip (Y1=22..Y2=24) above the door panel (Y=24..82 inside G0182/
 *   G0184).
 * - ReDMCSB DUNVIEW.C:6991 (F0119_DUNGEONVIEW_DrawSquareD2L C17_ELEMENT_
 *   DOOR_FRONT MEDIA009 legacy) calls F0100_DUNGEONVIEW_DrawWallSetBitmap
 *   (G0703, G0173). DUNVIEW.C:6994 (F0119 D2L MEDIA508) calls
 *   F0104(G2114_DoorFrameTopD2L, C725_ZONE_DOOR_FRAME_TOP_D2L).
 *   DUNVIEW.C:6997 (F0119 D2L MEDIA720) calls F0104(G2114,
 *   C729_ZONE_DOOR_FRAME_TOP_D2L).
 * - ReDMCSB DUNVIEW.C:7184 (F0120_DUNGEONVIEW_DrawSquareD2R_CPSF
 *   C17_ELEMENT_DOOR_FRONT MEDIA009 legacy) calls F0100(G0703, G0175).
 *   DUNVIEW.C:7187 (F0120 D2R MEDIA508) calls F0104(G2113_DoorFrameTopD2R,
 *   C727_ZONE_DOOR_FRAME_TOP_D2R). DUNVIEW.C:7190 (F0120 D2R MEDIA720)
 *   calls F0104(G2113, C731_ZONE_DOOR_FRAME_TOP_D2R).
 * - ReDMCSB DUNVIEW.C:3048-3068 F0100_DUNGEONVIEW_DrawWallSetBitmap is
 *   the legacy door-frame-top blit, using C10_COLOR_FLESH transparency
 *   and the stride X/Y/ByteWidth coordinates.
 * - ReDMCSB DUNVIEW.C:6900 F0119 / DUNVIEW.C:7051 F0120 are the dispatch
 *   start lines; DUNVIEW.C:8513 / 8517 are the F0128 caller sites.
 * - ReDMCSB DEFS.H:2582-2583 / 2603-2604 M604_VIEW_SQUARE_D2L (4/7) and
 *   M605_VIEW_SQUARE_D2R (5/8); DEFS.H:4068-4070 (MEDIA508 block)
 *   C725/C726/C727; DEFS.H:4087-4089 (MEDIA720 block) C729/C730/C731;
 *   DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:5457 G0694; DEFS.H:5539 G0182;
 *   DEFS.H:5541 G0184; DEFS.H:2790 C1_VIEW_DOOR_ORNAMENT_D2LCR.
 * - CSB counterpart: csb_v1_viewport_d2c_f0111_door_front_pc34_compat
 *   (D2C door-front layering) and csb_v1_viewport_d2l2_d2r2_f0111_door_
 *   pc34_compat (CSB D2L2/D2R2 door-side F0111 dispatch).
 *
 * Non-overlap siblings:
 * - dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat (F0111
 *   partly-open half-blit body, distinct contract).
 * - dm1_v1_viewport_d2l2_d2r2_f0111_door_front_pair_pc34_compat (D2L2/
 *   D2R2 front-door pair, distinct squares 9/10).
 * - dm1_v1_viewport_d0c_door_edge_ornament_pc34_compat (D0C door-frame
 *   border + thieves-eye hole, square 11).
 *
 * Contract-only: no real-asset or original-DOS pixel parity is claimed.
 */
#include "firestaff/dm1/v1/viewport/d2l_d2r_door_frame_top_edge_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_self_test();
    const DM1_V1_D2L_D2RDoorFrameTopEdgeSelfTestResultPc34 *result =
        dm1_v1_viewport_d2l_d2r_door_frame_top_edge_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d2l_d2r_door_frame_top_edge_pc34_compat "
           "assertions=%d failures=%d d2l_legacy=%d d2l_f20e=%d d2l_i34e=%d "
           "d2r_legacy=%d d2r_f20e=%d d2r_i34e=%d invalid=%d "
           "stride_g0173=%d stride_g0175=%d band_strip=%d zone_family=%d "
           "door_panel_post_band=%d view_square=%d non_overlap=%d "
           "bitmap_route=%d c10_transparency=%d hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->d2l_legacy_zone_count : 0,
           result ? result->d2l_f20e_zone_count : 0,
           result ? result->d2l_i34e_zone_count : 0,
           result ? result->d2r_legacy_zone_count : 0,
           result ? result->d2r_f20e_zone_count : 0,
           result ? result->d2r_i34e_zone_count : 0,
           result ? result->invalid_target_count : 0,
           result ? result->stride_g0173_checks : 0,
           result ? result->stride_g0175_checks : 0,
           result ? result->band_strip_checks : 0,
           result ? result->zone_id_family_checks : 0,
           result ? result->door_panel_post_band_checks : 0,
           result ? result->view_square_anchor_checks : 0,
           result ? result->non_overlap_checks : 0,
           result ? result->bitmap_route_checks : 0,
           result ? result->c10_transparency_checks : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
