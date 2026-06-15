#include "firestaff/dm1/v1/viewport/d1c_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>

/*
 * Contract-only DM1 V1 D1C center F0111 partly-open door source-lock test.
 *
 * ReDMCSB anchors:
 * - DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor; line 4248 open
 *   guard, lines 4311-4334 partly-open horizontal frame/zone/C10 path.
 * - DUNVIEW.C:7873-7911 F0124_DUNGEONVIEW_DrawSquareD1C
 *   C17_ELEMENT_DOOR_FRONT branch; F0111 at 7905/7908 binds
 *   G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, 96x88,
 *   C2_VIEW_DOOR_ORNAMENT_D1LCR, G0186_s_Graphic558_Frames_Door_D1C,
 *   and M631_ZONE_DOOR_D1C between DoorPass1 0x0218 and DoorPass2 0x0349.
 * - DUNVIEW.C:8518-8533 F0128_DUNGEONVIEW_Draw_CPSF calls F0124 for
 *   D1C depth=1 center lane at line 8533.
 * - DUNVIEW.C:694-705 defines G0186_s_Graphic558_Frames_Door_D1C.
 * - DEFS.H:1039-1043 C0..C4 door states; 2088 C10_COLOR_FLESH;
 *   2599 M606_VIEW_SQUARE_D1C; 2791 C2_VIEW_DOOR_ORNAMENT_D1LCR;
 *   3508 C6_UNKNOWN; 3516 MASK0x4000; 4259 M631_ZONE_DOOR_D1C;
 *   5458 G0695 extern; 5543 G0186 extern.
 *
 * This test is non-overlapping with the closed-door sibling
 * tests/test_dm1_v1_viewport_d1c_f0111_door_pc34_compat.c and makes no
 * real-asset/original-DOS pixel parity claim.
 */

int main(void)
{
    const int rc =
        run_dm1_v1_viewport_d1c_f0111_partly_open_door_self_test();
    const DM1_V1_D1CF0111PartlyOpenDoorSelfTestResultPc34 *result =
        dm1_v1_viewport_d1c_f0111_partly_open_door_last_self_test_result_pc34();
    const int d1c_partly = result ? result->d1c_partly_one +
                                       result->d1c_partly_two +
                                       result->d1c_partly_three
                                 : 0;

    if (rc != 0 || !result || result->failures != 0) {
        printf("FAIL test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat "
               "assertions=%d failures=%d d1c_partly=%d closed_rejections=%d "
               "open_rejections=%d unknown_rejections=%d door_dim=96x88 "
               "hash=0x%08x\n",
               result ? result->assertions : 0,
               result ? result->failures : 1,
               d1c_partly,
               result ? result->closed_rejections : 0,
               result ? result->open_rejections : 0,
               result ? result->unknown_rejections : 0,
               result ? result->deterministic_hash : 0u);
        return 1;
    }

    printf("PASS test_dm1_v1_viewport_d1c_f0111_partly_open_door_pc34_compat "
           "assertions=%d failures=0 d1c_partly=%d closed_rejections=%d "
           "open_rejections=%d unknown_rejections=%d door_dim=96x88 "
           "hash=0x%08x\n",
           result->assertions,
           d1c_partly,
           result->closed_rejections,
           result->open_rejections,
           result->unknown_rejections,
           result->deterministic_hash);
    return 0;
}
