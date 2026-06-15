/*
 * DM1 V1 PC 3.4 D1L2/D1R2 F0111 partly-open lateral corridor-side
 * door contract test.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor, including
 *   4248 fully-open guard, 4308 state decrement, 4312-4313 horizontal
 *   frame selection, 4317-4324 C6_UNKNOWN/C10 first-half blit, and
 *   4325-4334 3|MASK0x4000 plus F0791 C10 second-half draw.
 * - ReDMCSB DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L and
 *   7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R are the D1L/D1R body
 *   callers; the D1L2/D1R2 lateral door fronts ride these bodies when
 *   the door square is in L2464_ai_SquareAspect[M556_DOOR_STATE] 1..3.
 * - ReDMCSB DUNVIEW.C:8524-8542 F0128_DUNGEONVIEW_Draw_CPSF dispatches
 *   D1L (line 8526) then D1R (line 8531); D1C followup at 8536, D0L
 *   at 8541, D0R at 8546, and D0C at 8549; F0127 object-pass boundary
 *   at 8294.
 * - ReDMCSB DEFS.H:1039-1043 door states, 2088 C10_COLOR_FLESH,
 *   2599-2601 M606/M607/M608 view squares, 2600-2601 M607_D1L /
 *   M608_D1R primary view squares, 2670-2676 DoorPass anchors,
 *   2791 C2_VIEW_DOOR_ORNAMENT_D1LCR, 3508 C6_UNKNOWN, 3516 MASK0x4000,
 *   4053-4054 C713/C714 wall zones, 4258/4260 M630/M632 door zones,
 *   5458 G0695, 5543/5545 G0186/G0188 symbols.
 * - CSB counterpart: test_csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.
 *   Non-overlap DM1 siblings: D1C
 *   test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat and
 *   D2L/D2R test_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat.
 *
 * Contract-only: no real-asset or original-DOS pixel parity is claimed.
 */
#include "firestaff/dm1/v1/viewport/d1l2_d1r2_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_self_test();
    const DM1_V1_D1L2D1R2F0111PartlyOpenDoorSelfTestResultPc34 *result =
        dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat "
           "assertions=%d failures=%d d1l2_partly=%d d1r2_partly=%d "
           "closed_rejections=%d open_rejections=%d unknown_rejections=%d "
           "c10_first_skips=%d c10_second_skips=%d followup_anchors=%d "
           "hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->d1l2_partly : 0,
           result ? result->d1r2_partly : 0,
           result ? result->closed_rejections : 0,
           result ? result->open_rejections : 0,
           result ? result->unknown_rejections : 0,
           result ? result->c10_first_half_skips : 0,
           result ? result->c10_second_half_skips : 0,
           result ? result->f0128_followup_anchors : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
