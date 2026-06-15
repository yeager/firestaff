/* ReDMCSB DUNVIEW.C F0108:3940-4011 D0C floor-ornament keepout contract. */
#include "dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    DM1_V1_D0CF0108FloorOrnamentSelfTestResultPc34 result;
    const int ok =
        run_dm1_v1_viewport_d0c_f0108_floor_ornament_self_test_pc34_compat(&result);

    if (!ok || result.failures != 0 ||
        result.deterministic_hash != DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34) {
        printf("FAIL test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat "
               "assertions=%d failures=%d d0c_keepouts=%d thing_passes=%d "
               "palette_keepouts=%d mutation_rejections=%d hash=0x%08x expected=0x%08x\n",
               result.assertions,
               result.failures + (result.deterministic_hash !=
                   DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34 ? 1 : 0),
               result.d0c_keepout_compositions,
               result.thing_pass_calls,
               result.palette_keepouts,
               result.mutation_rejections,
               result.deterministic_hash,
               (uint32_t)DM1_V1_D0C_F0108_FLOOR_ORNAMENT_HASH_PC34);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d0c_f0108_floor_ornament_pc34_compat "
           "assertions=%d failures=0 d0c_keepouts=%d thing_passes=%d "
           "palette_keepouts=%d mutation_rejections=%d hash=0x%08x\n",
           result.assertions,
           result.d0c_keepout_compositions,
           result.thing_pass_calls,
           result.palette_keepouts,
           result.mutation_rejections,
           result.deterministic_hash);
    return 0;
}
