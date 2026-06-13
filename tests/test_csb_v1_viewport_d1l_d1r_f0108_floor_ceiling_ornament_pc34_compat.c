#include "firestaff/csb/v1/viewport/d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const CSB_V1_D1LD1RF0108SelfTestResultPc34 *result;
    const int ok = run_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_self_test();

    result = csb_v1_viewport_d1l_d1r_f0108_last_self_test_result_pc34();
    if (!ok || !result || result->failures != 0) {
        printf("FAIL test_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat "
               "assertions=%d failures=%d hash=0x%08x\n",
               result ? result->assertions : 0,
               result ? result->failures : 1,
               result ? result->deterministic_hash : 0u);
        return 1;
    }

    printf("PASS test_csb_v1_viewport_d1l_d1r_f0108_floor_ceiling_ornament_pc34_compat "
           "assertions=%d failures=0 d1l_floor=%d d1r_floor=%d ceiling_copies=%d "
           "custom_bg_masks=%d thing_passes=%d palette_keepouts=%d "
           "mutation_rejections=%d hash=0x%08x\n",
           result->assertions,
           result->d1l_floor_calls,
           result->d1r_floor_calls,
           result->ceiling_copies,
           result->custom_background_masks,
           result->thing_pass_calls,
           result->palette_keepouts,
           result->mutation_rejections,
           result->deterministic_hash);
    return 0;
}
