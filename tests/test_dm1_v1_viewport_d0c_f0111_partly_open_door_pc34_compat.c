#include "firestaff/dm1/v1/viewport/d0c_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>

/*
 * DM1 V1 D0C F0111 partly-open boundary test.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor C10 transparent
 *   horizontal half-blit body.
 * - DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C routes the
 *   center F0111 door-front path for D2C at 7313/7316.
 * - DUNVIEW.C:8164-8311 F0127_DUNGEONVIEW_DrawSquareD0C is the D0C body;
 *   its C16 door-side branch draws the G0172/G2116 D0C door frame and has
 *   no F0111 call.
 * - DUNVIEW.C:8498-8542 F0128 calls F0121 at 8521 and D0C/F0127 at 8542.
 * - DUNVIEW.C:92/2654-2658 and DEFS.H:5458 pin G0695 as D1LCR, not D0C;
 *   D0C uses G2116_DoorFrameFrontD0C.
 * - DEFS.H:1039-1044 C0..C5 door states.
 *
 * Synthetic framebuffer/viewport contract only; no original DOS pixel
 * parity claim.
 */

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d0c_f0111_partly_open_door_self_test();
    const DM1_V1_D0CF0111PartlyOpenDoorSelfTestResultPc34 *result =
        dm1_v1_viewport_d0c_f0111_partly_open_door_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d0c_f0111_partly_open_door_pc34_compat "
           "assertions=%d failures=%d open=%d partly=%d closed=%d destroyed=%d "
           "invalid=%d c10=%d d0c_dispatch=%d native_bitmap=%d non_overlap=%d "
           "hash=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->open_branch : 0,
           result ? result->partly_open_branches : 0,
           result ? result->closed_branch : 0,
           result ? result->destroyed_branch : 0,
           result ? result->invalid_branch : 0,
           result ? result->c10_write_skip_checks : 0,
           result ? result->d0c_dispatch_boundary_checks : 0,
           result ? result->native_bitmap_boundary_checks : 0,
           result ? result->non_overlap_checks : 0,
           result ? result->deterministic_hash : 0u);

    return rc == 0 && result && result->failures == 0 ? 0 : 1;
}
