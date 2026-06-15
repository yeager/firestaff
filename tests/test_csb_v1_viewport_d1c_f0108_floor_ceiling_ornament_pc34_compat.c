#include "firestaff/csb/v1/viewport/d1c_f0108_floor_ceiling_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int passed =
        run_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_self_test();
    const CSB_V1_D1CF0108SelfTestResultPc34 *result =
        csb_v1_viewport_d1c_f0108_last_self_test_result_pc34();

    if (!passed || !result || !result->ok || result->failures != 0) {
        printf("FAIL test_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_pc34_compat assertions=%d failures=%d\n",
               result ? result->assertions : 0,
               result ? result->failures : 1);
        return 1;
    }

    printf("PASS test_csb_v1_viewport_d1c_f0108_floor_ceiling_ornament_pc34_compat assertions=%d failures=0 custom_bg_masks=%d d1c_floor=%d thing_passes=%d palette_keepouts=%d mutation_rejections=%d hash=0x%08x\n",
           result->assertions,
           result->custom_bg_masks,
           result->d1c_floor,
           result->thing_passes,
           result->palette_keepouts,
           result->mutation_rejections,
           (unsigned int)result->deterministic_hash);
    return 0;
}
