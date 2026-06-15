#include "firestaff/dm1/v1/viewport/d3c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const DM1_V1_D3CF0108SelfTestResultPc34 *result;
    const int ok = run_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_self_test();

    result = dm1_v1_viewport_d3c_f0108_last_self_test_result_pc34();
    if (!ok || !result || result->failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_pc34_compat "
               "assertions=%d failures=%d hash=0x%08x\n",
               result ? result->assertions : 0,
               result ? result->failures : 1,
               result ? result->deterministic_hash : 0u);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d3c_f0108_floor_ceiling_ornament_pc34_compat "
           "assertions=%d failures=0 d3c_floor=%d footprint_recursions=%d "
           "ceilings=%d thing_passes=%d mutation_rejections=%d hash=0x%08x\n",
           result->assertions,
           result->d3c_floor_calls,
           result->footprint_recursions,
           result->ceiling_calls,
           result->thing_pass_calls,
           result->mutation_rejections,
           result->deterministic_hash);
    return 0;
}
