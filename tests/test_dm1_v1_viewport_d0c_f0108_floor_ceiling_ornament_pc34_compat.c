#include "firestaff/dm1/v1/viewport/d0c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>

/*
 * DM1 V1 D0C F0108 floor+ceiling+ornament contract gate.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C F0108:3940-4011 floor-ornament ordinal, footprint, C10,
 *   and PC34 C1500 zone math.
 * - DUNVIEW.C F0127:8184-8311 D0C body; F0112 precedes F0115 and
 *   teleporter F0113 follows F0115.
 * - DUNVIEW.C F0128:8491-8542 D3 corridor neighborhood and D0L/D0R
 *   terminal side-pair correction before D0C.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, F0172:2466-2523.
 * - DEFS.H M550..M579 and C705/C706 wall-zone ordinals.
 */
int main(void)
{
    const int ok =
        run_dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_self_test();
    const DM1_V1_D0CF0108FloorCeilingOrnamentSelfTestResultPc34 *result =
        dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_last_self_test_result_pc34();
    const uint32_t expected = DM1_V1_D0C_F0108_FCO_EXPECTED_HASH_PC34;

    if (!ok || !result || result->failures != 0 ||
        result->deterministic_hash != expected) {
        printf("FAIL test_dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_pc34_compat "
               "assertions=%d failures=%d contexts=%d d0c_body=%d f0108=%d "
               "ordering=%d c10=%d mutation_rejections=%d non_overlap=%d "
               "hash=0x%08x expected=0x%08x\n",
               result ? result->assertions : 0,
               result ? result->failures +
                   (result->deterministic_hash != expected ? 1 : 0) : 1,
               result ? result->contexts_checked : 0,
               result ? result->d0c_body_checks : 0,
               result ? result->f0108_reference_checks : 0,
               result ? result->ordering_checks : 0,
               result ? result->c10_checks : 0,
               result ? result->mutation_rejections : 0,
               result ? result->non_overlap_checks : 0,
               result ? result->deterministic_hash : 0u,
               expected);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d0c_f0108_floor_ceiling_ornament_pc34_compat "
           "assertions=%d failures=0 contexts=%d d0c_body=%d f0108=%d "
           "ordering=%d c10=%d mutation_rejections=%d non_overlap=%d "
           "hash=0x%08x\n",
           result->assertions,
           result->contexts_checked,
           result->d0c_body_checks,
           result->f0108_reference_checks,
           result->ordering_checks,
           result->c10_checks,
           result->mutation_rejections,
           result->non_overlap_checks,
           result->deterministic_hash);
    return 0;
}
