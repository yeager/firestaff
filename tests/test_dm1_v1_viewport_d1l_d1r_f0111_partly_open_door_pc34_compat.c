#include "firestaff/dm1/v1/viewport/d1l_d1r_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>

/*
 * DM1 V1 D1L/D1R F0111 front-door source-lock gate.
 *
 * ReDMCSB anchors: DUNVIEW.C F0111 lines 4218-4337; F0122 lines
 * 7391-7557 with D1L door front at 7492-7508 and pass2 at 7536; F0123
 * lines 7559-7725 with D1R door front at 7660-7676 and pass2 at 7704;
 * F0115 lines 4788-4804, 4916-4923, 5176-5188; F0128 lines 8524-8533;
 * F0104 lines 3113-3156; F0105 lines 3185-3247; F0107 lines 3502-3938;
 * F0108 lines 3940-4011. DUNGEON.C F0163 lines 1769-1838, F0164 lines
 * 1840-1905, F0172 lines 2466-2523. DEFS.H lines 2088, 2596-2611,
 * 2661-2667, 2672-2675, 2789-2791, 3508, 3516, 4091-4093, 4258-4260,
 * 5458, 5542, 5544.
 *
 * Contract-only: no game data, no real-asset pixel parity claim.
 */

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_self_test();
    const DM1_V1_D1LD1RF0111DoorSelfTestResultPc34 *result =
        dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_last_self_test_result_pc34();
    const int failures = result ? result->failures : 1;

    printf("%s test_dm1_v1_viewport_d1l_d1r_f0111_partly_open_door_pc34_compat "
           "assertions=%d failures=%d d1l_closed=%d d1r_closed=%d "
           "d1l_partly=%d d1r_partly=%d open_rejections=%d "
           "destroyed_rejections=%d unknown_rejections=%d "
           "out_of_range_square_rejections=%d unsupported_element_rejections=%d "
           "c10_first_skips=%d c10_second_skips=%d f0115_anchors=%d "
           "f0128_anchors=%d hash=0x%08X\n",
           rc == 0 && result && failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           failures,
           result ? result->d1l_closed : 0,
           result ? result->d1r_closed : 0,
           result ? result->d1l_partly : 0,
           result ? result->d1r_partly : 0,
           result ? result->open_rejections : 0,
           result ? result->destroyed_rejections : 0,
           result ? result->unknown_rejections : 0,
           result ? result->out_of_range_square_rejections : 0,
           result ? result->unsupported_element_rejections : 0,
           result ? result->c10_first_half_skips : 0,
           result ? result->c10_second_half_skips : 0,
           result ? result->f0115_doorpass_anchors : 0,
           result ? result->f0128_dispatch_anchors : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && failures == 0 ? 0 : 1;
}
