/*
 * DM1 V1 PC 3.4 D1C/D2C door-frame-top edge contract test.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:605 G0174_auc_Graphic558_Frame_DoorFrameTop_D2C =
 *   {64,159,22,24,48,3,0,0}; DUNVIEW.C:608 G0177_auc_Graphic558_Frame_
 *   DoorFrameTop_D1C = {48,175,14,17,64,4,0,0}.
 * - ReDMCSB DUNVIEW.C:7317/7323/7328 route D2C legacy/F20E/I34E
 *   door-frame-top; DUNVIEW.C:7877/7882/7886 route D1C legacy/F20E/I34E.
 * - ReDMCSB DUNVIEW.C:7244/7727 are F0121/F0124 starts; 8521/8533 are
 *   the F0128 caller sites.
 * - ReDMCSB DEFS.H:4068-4073 and 4087-4093 pin center zones C726/C730
 *   for D2C and C729/C733 for D1C.
 *
 * Contract-only: no real-asset screenshot or original-DOS pixel parity claim.
 */

#include "firestaff/dm1/v1/viewport/d1c_d2c_door_frame_top_edge_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d1c_d2c_door_frame_top_edge_self_test();
    const DM1_V1_D1C_D2CDoorFrameTopEdgeSelfTestResultPc34 *result =
        dm1_v1_viewport_d1c_d2c_door_frame_top_edge_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d1c_d2c_door_frame_top_edge_pc34_compat "
           "assertions=%d failures=%d d2c=%d/%d/%d d1c=%d/%d/%d "
           "invalid=%d stride=%d zone=%d dispatch=%d order=%d non_overlap=%d "
           "bitmap=%d viewport=%d hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->d2c_legacy_count : 0,
           result ? result->d2c_f20e_count : 0,
           result ? result->d2c_i34e_count : 0,
           result ? result->d1c_legacy_count : 0,
           result ? result->d1c_f20e_count : 0,
           result ? result->d1c_i34e_count : 0,
           result ? result->invalid_count : 0,
           result ? result->stride_checks : 0,
           result ? result->zone_checks : 0,
           result ? result->dispatch_checks : 0,
           result ? result->order_checks : 0,
           result ? result->non_overlap_checks : 0,
           result ? result->bitmap_route_checks : 0,
           result ? result->viewport_band_checks : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
