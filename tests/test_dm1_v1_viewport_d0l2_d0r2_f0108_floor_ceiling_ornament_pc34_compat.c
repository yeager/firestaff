#include "dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const DM1_V1_D0L2D0R2F0108SelfTestResultPc34 *result;
    const int ok =
        run_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_self_test();

    result = dm1_v1_viewport_d0l2_d0r2_f0108_last_self_test_result_pc34();
    if (!ok || !result || result->failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_pc34_compat "
               "assertions=%d failures=%d hash=0x%08x\n",
               result ? result->assertions : 0,
               result ? result->failures : 1,
               result ? result->deterministic_hash : 0u);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d0l2_d0r2_f0108_floor_ceiling_ornament_pc34_compat "
           "assertions=%d failures=0 floor_recursions=%d ceiling_copies=%d "
           "thing_passes=%d dispatch_entries=%d row_guard_rejections=%d "
           "mutation_rejections=%d hash=0x%08x\n",
           result->assertions,
           result->floor_recursion_calls,
           result->ceiling_copies,
           result->thing_pass_calls,
           result->dispatch_entries,
           result->row_guard_rejections,
           result->mutation_rejections,
           result->deterministic_hash);
    return 0;
}
