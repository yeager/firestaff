#include "dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const DM1_V1_D1L2D1R2F0108WallSelfTestResultPc34 *result;
    const int ok =
        run_dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_self_test();

    result = dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_last_self_test_result_pc34();
    if (!ok || !result || result->failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_pc34_compat "
               "assertions=%d failures=%d hash=0x%08x\n",
               result ? result->assertions : 0,
               result ? result->failures : 1,
               result ? result->deterministic_hash : 0u);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d1l2_d1r2_f0108_wall_composition_pc34_compat "
           "assertions=%d failures=0 wall_draws=%d footprint_recursions=%d "
           "row_guard_rejections=%d mutation_rejections=%d hash=0x%08x\n",
           result->assertions,
           result->wall_draws,
           result->footprint_recursions,
           result->row_guard_rejections,
           result->mutation_rejections,
           result->deterministic_hash);
    return 0;
}
