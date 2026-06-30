/*
 * DM1 V1 PC 3.4 D1L/D1R door-frame-top edge contract test.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:607-609 G0176_auc_Graphic558_Frame_DoorFrameTop_D1L
 *   = { 0, 31, 14, 17, 64, 4, 16, 0 } and G0178_auc_Graphic558_Frame_
 *   DoorFrameTop_D1R = { 192, 223, 14, 17, 64, 4, 16, 0 } define the
 *   D1L/D1R door-frame-top strides; the band is a 4-pixel-tall horizontal
 *   strip (Y1=14..Y2=17) above the door panel (Y=17..85 inside G0185/
 *   G0187).
 * - ReDMCSB DUNVIEW.C:7496 (F0122_DUNGEONVIEW_DrawSquareD1L C17_ELEMENT_
 *   DOOR_FRONT MEDIA009 legacy) calls F0100_DUNGEONVIEW_DrawWallSetBitmap
 *   (G0704, G0176). DUNVIEW.C:7500 (F0122 D1L MEDIA508) calls
 *   F0104(G2111_DoorFrameTopD1L, C728_ZONE_DOOR_FRAME_TOP_D1L).
 *   DUNVIEW.C:7503 (F0122 D1L MEDIA720) calls F0104(G2111,
 *   C732_ZONE_DOOR_FRAME_TOP_D1L).
 * - ReDMCSB DUNVIEW.C:7664 (F0123_DUNGEONVIEW_DrawSquareD1R C17_ELEMENT_
 *   DOOR_FRONT MEDIA009 legacy) calls F0100(G0704, G0178).
 *   DUNVIEW.C:7668 (F0123 D1R MEDIA508) calls F0104(G2110_DoorFrameTopD1R,
 *   C730_ZONE_DOOR_FRAME_TOP_D1R). DUNVIEW.C:7671 (F0123 D1R MEDIA720)
 *   calls F0104(G2110, C734_ZONE_DOOR_FRAME_TOP_D1R).
 * - ReDMCSB DUNVIEW.C:3048-3068 F0100_DUNGEONVIEW_DrawWallSetBitmap is
 *   the legacy door-frame-top blit, using C10_COLOR_FLESH transparency
 *   and the stride X/Y/ByteWidth coordinates.
 * - ReDMCSB DUNVIEW.C:7391 F0122 / DUNVIEW.C:7559 F0123 are the dispatch
 *   start lines; DUNVIEW.C:8525 / 8529 are the F0128 caller sites.
 * - ReDMCSB DEFS.H:2585-2586 / 2600-2601 M607_VIEW_SQUARE_D1L (4/7) and
 *   M608_VIEW_SQUARE_D1R (5/8); DEFS.H:4068-4073 (MEDIA508 block)
 *   C725/C726/C727 (D2) + C728_ZONE_DOOR_FRAME_TOP_D1L=728,
 *   C729_ZONE_DOOR_FRAME_TOP_D1C=729, C730_ZONE_DOOR_FRAME_TOP_D1R=730;
 *   DEFS.H:4087-4093 (MEDIA720 block) C729/C730/C731 (D2) +
 *   C732_ZONE_DOOR_FRAME_TOP_D1L=732, C733_ZONE_DOOR_FRAME_TOP_D1C=733,
 *   C734_ZONE_DOOR_FRAME_TOP_D1R=734; DEFS.H:2088 C10_COLOR_FLESH;
 *   DEFS.H:2159 M075_BITMAP_BYTE_COUNT(96, 88) = 4224;
 *   DEFS.H:5458 G0695; DEFS.H:5542 G0185; DEFS.H:5544 G0187;
 *   DEFS.H:2791 C2_VIEW_DOOR_ORNAMENT_D1LCR.
 * - CSB counterpart: csb_v1_viewport_d1c_f0111_door_front_pc34_compat
 *   (D1C door-front layering) and csb_v1_viewport_d1l2_d1r2_f0111_door_
 *   pc34_compat (CSB D1L2/D1R2 door-side F0111 dispatch).
 *
 * Non-overlap siblings:
 * - dm1_v1_viewport_d2l_d2r_door_frame_top_edge_pc34_compat (D2L/D2R
 *   door-frame-top edge, distinct Y1=22..Y2=24 band and G0173/G0175
 *   strides).
 * - dm1_v1_viewport_d2c_door_frame_top_edge_pc34_compat (D2C door-frame-
 *   top edge, distinct stride and zones C726/C730).
 * - dm1_v1_viewport_d1c_door_frame_top_edge_pc34_compat (D1C door-frame-
 *   top edge, distinct G0177 stride and C729/C733 zones, F0124 dispatch).
 * - dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_pc34_compat (F0111
 *   partly-open half-blit body, distinct contract).
 * - dm1_v1_viewport_d1l2_d1r2_f0111_door_front_pair_pc34_compat (D1L2/
 *   D1R2 front-door pair, distinct squares).
 * - dm1_v1_viewport_d0c_door_edge_ornament_pc34_compat (D0C door-frame
 *   border + thieves-eye hole, square 11).
 *
 * Contract-only: no real-asset or original-DOS pixel parity is claimed.
 */
#include "firestaff/dm1/v1/viewport/d1l_d1r_door_frame_top_edge_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_self_test();
    const DM1_V1_D1L_D1RDoorFrameTopEdgeSelfTestResultPc34 *result =
        dm1_v1_viewport_d1l_d1r_door_frame_top_edge_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d1l_d1r_door_frame_top_edge_pc34_compat "
           "assertions=%d failures=%d d1l_legacy=%d d1l_f20e=%d d1l_i34e=%d "
           "d1r_legacy=%d d1r_f20e=%d d1r_i34e=%d invalid=%d "
           "stride_g0176=%d stride_g0178=%d band_strip=%d zone_family=%d "
           "door_panel_post_band=%d view_square=%d non_overlap=%d "
           "bitmap_route=%d c10_transparency=%d hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->d1l_legacy_zone_count : 0,
           result ? result->d1l_f20e_zone_count : 0,
           result ? result->d1l_i34e_zone_count : 0,
           result ? result->d1r_legacy_zone_count : 0,
           result ? result->d1r_f20e_zone_count : 0,
           result ? result->d1r_i34e_zone_count : 0,
           result ? result->invalid_target_count : 0,
           result ? result->stride_g0176_checks : 0,
           result ? result->stride_g0178_checks : 0,
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
