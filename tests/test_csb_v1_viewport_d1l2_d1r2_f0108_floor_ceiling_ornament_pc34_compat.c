#include "csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int passed =
        run_csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_self_test();
    const CSB_V1_D1L2D1R2F0108SelfTestResultPc34 *result =
        csb_v1_viewport_d1l2_d1r2_f0108_last_self_test_result_pc34();

    if (!passed || !result || !result->ok) {
        printf("FAIL test_csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat assertions=%d failures=%d\n",
               result ? result->assertions : 0,
               result ? result->failures : 1);
        return 1;
    }

    printf("PASS test_csb_v1_viewport_d1l2_d1r2_f0108_floor_ceiling_ornament_pc34_compat assertions=%d failures=%d hash=0x%08x\n",
           result->assertions, result->failures,
           (unsigned int)result->deterministic_hash);
    return 0;
}
