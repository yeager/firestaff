/*
 * ReDMCSB anchors: DUNVIEW.C F0108:3940-4011, F0107:3502-3938,
 * F0098:2962-3002, F0115:4547-4581/5180-5188/5211-5214/5668-5671;
 * DEFS.H:2088,2596-2611,2668-2677,2698-2702,4045-4046.
 */
#include "firestaff/dm1/v1/viewport/d0c_f0108_floor_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 result;
    const int ok =
        run_dm1_v1_viewport_d0c_f0108_floor_ornament_self_test_pc34_compat(&result);

    if (!ok || result.failures != 0 ||
        result.deterministic_hash != DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34) {
        printf("FAIL test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat "
               "assertions=%d failures=%d floor_writes=%d thing_passes=%d "
               "keepouts=%d mutation_rejections=%d hash=0x%08x expected=0x%08x\n",
               result.assertions,
               result.failures + (result.deterministic_hash !=
                   DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34 ? 1 : 0),
               result.floor_writes,
               result.thing_pass_calls,
               result.keepout_preservations,
               result.mutation_rejections,
               result.deterministic_hash,
               (uint32_t)DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat "
           "assertions=%d failures=0 floor_writes=%d thing_passes=%d "
           "keepouts=%d mutation_rejections=%d hash=0x%08x\n",
           result.assertions,
           result.floor_writes,
           result.thing_pass_calls,
           result.keepout_preservations,
           result.mutation_rejections,
           result.deterministic_hash);
    return 0;
}
