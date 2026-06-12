/*
 * DM1 V1 PC 3.4 D2L/D2R F0111 partly-open door contract test.
 *
 * Source-lock evidence:
 * - ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor, including
 *   4248 fully-open guard, 4308 state decrement, 4312-4313 horizontal
 *   frame selection, 4317-4324 C6_UNKNOWN/C10 first-half blit, and
 *   4325-4334 3|MASK0x4000 plus F0791 C10 second-half draw.
 * - ReDMCSB DUNVIEW.C:8504-8508 D2L2/D2R2 MEDIA720 wall guard,
 *   8513-8517 D2L/D2R dispatch, and 8521 D2C dispatch only as order
 *   bound; DUNVIEW.C:7244-7389 F0121 is D2C center, not this route.
 * - ReDMCSB DUNVIEW.C:6987-7004 F0119 D2L and 7180-7197 F0120 D2R
 *   C17_ELEMENT_DOOR_FRONT calls into F0111.
 * - ReDMCSB DUNVIEW.C:6837-6865 F0678 and 6868-6896 F0679 are D2-side
 *   wall anchors which return wall cases before any F0111 door-front
 *   route.
 * - ReDMCSB DEFS.H:1039-1043 door states, 2088 C10_COLOR_FLESH,
 *   2603-2604 M604/M605, 2669/2672 DoorPass1/DoorPass2 anchors,
 *   2790 actual C1_VIEW_DOOR_ORNAMENT_D2LCR, 3508 C6_UNKNOWN,
 *   3516 MASK0x4000, 4254-4258 M627/M629, 5457 G0694, 5539 G0182,
 *   and 5541 G0184.
 * - CSB counterpart: test_csb_v1_viewport_d2c_f0111_partly_open_door_pc34_compat.
 *   Non-overlap sibling: test_dm1_v1_viewport_d0l_d0r_f0111_door_pc34_compat.
 *
 * Contract-only: no real-asset or original-DOS pixel parity is claimed.
 */
#include "firestaff/dm1/v1/viewport/d2l_d2r_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_self_test();
    const DM1_V1_D2LD2RF0111PartlyOpenDoorSelfTestResultPc34 *result =
        dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d2l_d2r_f0111_partly_open_door_pc34_compat "
           "assertions=%d failures=%d d2l_partly=%d d2r_partly=%d "
           "closed_rejections=%d open_rejections=%d unknown_rejections=%d "
           "hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->d2l_partly : 0,
           result ? result->d2r_partly : 0,
           result ? result->closed_rejections : 0,
           result ? result->open_rejections : 0,
           result ? result->unknown_rejections : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
