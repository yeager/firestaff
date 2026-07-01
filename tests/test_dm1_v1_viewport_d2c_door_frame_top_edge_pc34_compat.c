/*
 * DM1 V1 D2C door-frame-top edge source-lock contract.
 *
 * ReDMCSB source anchors:
 * - DUNVIEW.C:605 G0174 top-frame stride.
 * - DUNVIEW.C:7244 F0121_DUNGEONVIEW_DrawSquareD2C.
 * - DUNVIEW.C:7313-7339 C17_ELEMENT_DOOR_FRONT draw order.
 * - DUNVIEW.C:8521 F0128 center D2 dispatch.
 *
 * Data-free regression only; no real-asset or original DOS parity claim.
 */

#include <stdio.h>

#include "firestaff/dm1/v1/viewport/d2c_door_frame_top_edge_pc34_compat.h"

int main(void)
{
    const int rc = run_dm1_v1_viewport_d2c_door_frame_top_edge_self_test();
    const DM1_V1_D2CDoorFrameTopEdgeSelfTestResultPc34 *result =
        dm1_v1_viewport_d2c_door_frame_top_edge_last_self_test_result_pc34();

    if (!result) {
        fprintf(stderr, "FAIL no self-test result\n");
        return 1;
    }

    printf("%s test_dm1_v1_viewport_d2c_door_frame_top_edge_pc34_compat "
           "assertions=%d hash=0x%08x legacy=%d f20e=%d i34e=%d\n",
           rc == 0 ? "PASS" : "FAIL",
           result->assertions,
           (unsigned)result->deterministic_hash,
           result->legacy_route_count,
           result->f20e_route_count,
           result->i34e_route_count);

    if (rc != 0) return rc;
    if (result->failures != 0) return 1;
    if (result->assertions < 150) return 1;
    if (result->deterministic_hash !=
        DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_HASH_PC34) {
        fprintf(stderr, "FAIL hash got=0x%08x want=0x%08x\n",
                (unsigned)result->deterministic_hash,
                (unsigned)DM1_V1_D2C_DOOR_FRAME_TOP_EDGE_HASH_PC34);
        return 1;
    }
    if (result->legacy_route_count != 1 ||
        result->f20e_route_count != 1 ||
        result->i34e_route_count != 1 ||
        result->invalid_target_count != 3 ||
        result->stride_checks != 3 ||
        result->zone_checks != 3 ||
        result->dispatch_order_checks != 3 ||
        result->button_branch_checks != 3 ||
        result->post_band_checks != 3 ||
        result->non_overlap_checks != 8) {
        fprintf(stderr, "FAIL counter invariant\n");
        return 1;
    }

    return 0;
}
