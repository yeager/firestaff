#include "firestaff/dm1/v1/viewport/f0111_door_transparency_pc34_compat.h"

#include <stdio.h>

/*
 * DM1 V1 F0111 door transparency source-lock gate.
 *
 * This is a contract-only gate. It does not load real assets and does not
 * claim original-DOS pixel parity. It is intentionally disjoint from the
 * existing per-square D2C/D1C/D0C/D1L2/D1R2/D2L/D2R/D2L2/D2R2/D3C F0111
 * gates by pinning the shared C10 transparency byte flow, D1C vertical
 * partly-open zone path, row-1/2 horizontal P2084_i_ZoneIndex shifts, and
 * the F0163/F0164 door map-cell thing-list source lock.
 */

int main(void)
{
    const int rc = run_dm1_v1_viewport_f0111_door_transparency_self_test();
    const DM1_V1_F0111DoorTransparencySelfTestResultPc34 *result =
        dm1_v1_viewport_f0111_door_transparency_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_f0111_door_transparency_pc34_compat "
           "assertions=%d failures=%d routes=%d d1c_vertical=%d "
           "horizontal=%d open_rejections=%d closed_rejections=%d "
           "map_cell=%d d2_wall_exclusions=%d c10=%d hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->route_count : 0,
           result ? result->d1c_vertical_partly : 0,
           result ? result->horizontal_partly : 0,
           result ? result->open_rejections : 0,
           result ? result->closed_rejections : 0,
           result ? result->map_cell_checks : 0,
           result ? result->d2l2_d2r2_wall_exclusions : 0,
           result ? result->c10_transparency_checks : 0,
           result ? result->deterministic_hash : 0U);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
