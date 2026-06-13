#include "firestaff/dm1/v1/viewport/d3c_back_wall_item_pc34_compat.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    const int ok = run_dm1_v1_viewport_d3c_back_wall_item_self_test_pc34();
    const DM1_V1_D3CBackWallItemSelfTestResultPc34 *result =
        dm1_v1_viewport_d3c_back_wall_item_last_self_test_result_pc34();
    const char *evidence =
        dm1_v1_viewport_d3c_back_wall_item_source_evidence_pc34();
    const char *disjoint =
        dm1_v1_viewport_d3c_back_wall_item_disjointness_note_pc34();
    int failed_after = 0;

    if (!result || !evidence || !disjoint) {
        printf("FAIL test_dm1_v1_viewport_d3c_back_wall_item_pc34_compat "
               "missing accessor pointers\n");
        return 1;
    }

    if (!ok || result->failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d3c_back_wall_item_pc34_compat "
               "self-test failures=%d assertions=%d hash=0x%08x\n",
               result->failures, result->assertions,
               (unsigned)result->deterministic_hash);
        return 1;
    }

    if (!strstr(evidence, "DUNVIEW.C F0115:4547-4581") ||
        !strstr(evidence, "DUNVIEW.C F0115:4794-4800") ||
        !strstr(evidence, "DUNVIEW.C F0115:4920-4923") ||
        !strstr(evidence, "DUNVIEW.C:6723") ||
        !strstr(evidence, "DUNVIEW.C:6816") ||
        !strstr(evidence, "DUNVIEW.C F0128:8499") ||
        !strstr(evidence, "M600_VIEW_SQUARE_D3C = 11") ||
        !strstr(evidence, "C10_COLOR_FLESH") ||
        !strstr(evidence, "no original DOS pixel parity")) {
        failed_after = 1;
    }
    if (!strstr(disjoint, "D3C F0107 wall-ornament") ||
        !strstr(disjoint, "D3C F0108 floor") ||
        !strstr(disjoint, "D3L2/D3R2 F0115") ||
        !strstr(disjoint, "D1L2/D1R2 F0115") ||
        !strstr(disjoint, "D0L2/D0R2 F0115")) {
        failed_after = 1;
    }
    if (result->wall_route_skips_f0115 != 1 ||
        result->corridor_pit_teleporter_back_then_front != 1 ||
        result->back_cells_visible_at_d3c != 1 ||
        result->front_cells_clipped_at_d3c != 1 ||
        result->f0115_call_count != 2 ||
        result->back_wall_item_zones_seen != 2) {
        failed_after = 1;
    }

    if (failed_after) {
        printf("FAIL test_dm1_v1_viewport_d3c_back_wall_item_pc34_compat "
               "post-self-test invariants wall=%d corridor=%d "
               "back_visible=%d front_clipped=%d "
               "f0115_calls=%d back_zones_seen=%d hash=0x%08x\n",
               result->wall_route_skips_f0115,
               result->corridor_pit_teleporter_back_then_front,
               result->back_cells_visible_at_d3c,
               result->front_cells_clipped_at_d3c,
               result->f0115_call_count,
               result->back_wall_item_zones_seen,
               (unsigned)result->deterministic_hash);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d3c_back_wall_item_pc34_compat "
           "assertions=%d failures=0 wall_skips_f0115=%d "
           "corridor_back_then_front=%d f0115_calls=%d "
           "back_zones_seen=%d c10_skips=%d hash=0x%08x\n",
           result->assertions,
           result->wall_route_skips_f0115,
           result->corridor_pit_teleporter_back_then_front,
           result->f0115_call_count,
           result->back_wall_item_zones_seen,
           result->c10_transparent_skip,
           (unsigned)result->deterministic_hash);
    return 0;
}
