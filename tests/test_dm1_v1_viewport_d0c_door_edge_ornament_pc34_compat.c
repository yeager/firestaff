/*
 * DM1 V1 D0C door-edge-ornament source-lock.
 *
 * Pins the ReDMCSB DUNVIEW.C:8185-8236 F0127 C16_ELEMENT_DOOR_SIDE
 * door-edge-ornament contract: G0172_auc_Graphic558_Frame_DoorFrame_D0C
 * stride { 96, 127, 0, 122, 16, 123, 0, 0 } + G2116_DoorFrameFrontD0C
 * (modern PC I34E) / G0709_puc_Bitmap_WallSet_DoorFrameFront (legacy
 * PC 3.4 Atari/Amiga) + C728/C724_ZONE_DOOR_FRAME_D0C + C736/C732_*
 * THIEVES_EYE_HOLE_IN_DOOR_FRAME + C10_COLOR_FLESH frame transparency +
 * C09_COLOR_GOLD hole transparency + post-frame F0112 ceiling-pit +
 * F0115/M609/C0x0021 thing-pass + F0113/C713/C715 field-blit
 * byte-stability. This is distinct from the F0111 door-panel state
 * machine, the F0111 partly-open half-blit body, the F0108 floor
 * ornament, and the F0108 floor+ceiling ornament.
 */
#include "firestaff/dm1/v1/viewport/d0c_door_edge_ornament_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const int rc = run_dm1_v1_viewport_d0c_door_edge_ornament_self_test();
    const DM1_V1_D0CDoorEdgeOrnamentSelfTestResultPc34 *result =
        dm1_v1_viewport_d0c_door_edge_ornament_last_self_test_result_pc34();

    printf("%s test_dm1_v1_viewport_d0c_door_edge_ornament_pc34_compat "
           "assertions=%d failures=%d "
           "no_thieves_eye_legacy=%d no_thieves_eye_f20e=%d "
           "no_thieves_eye_i34e=%d thieves_eye_legacy=%d "
           "thieves_eye_f20e=%d thieves_eye_i34e=%d invalid=%d "
           "g0172=%d g2116=%d thieve=%d transpar=%d gating=%d "
           "f0112=%d f0115=%d f0113=%d nonoverlap=%d strip=%d "
           "hash=0x%08X expected=0x%08X\n",
           rc == 0 && result && result->failures == 0 ? "PASS" : "FAIL",
           result ? result->assertions : 0,
           result ? result->failures : 1,
           result ? result->no_thieves_eye_legacy_branch : 0,
           result ? result->no_thieves_eye_f20e_branch : 0,
           result ? result->no_thieves_eye_i34e_branch : 0,
           result ? result->thieves_eye_legacy_branch : 0,
           result ? result->thieves_eye_f20e_branch : 0,
           result ? result->thieves_eye_i34e_branch : 0,
           result ? result->invalid_branch : 0,
           result ? result->g0172_stride_checks : 0,
           result ? result->g2116_zone_checks : 0,
           result ? result->thieves_eye_zone_checks : 0,
           result ? result->transparency_color_checks : 0,
           result ? result->thieves_eye_branch_gating_checks : 0,
           result ? result->post_frame_f0112_checks : 0,
           result ? result->post_frame_f0115_checks : 0,
           result ? result->post_frame_f0113_checks : 0,
           result ? result->non_overlap_checks : 0,
           result ? result->bitmap_strip_byte_width_checks : 0,
           result ? result->deterministic_hash : 0u,
           (uint32_t)DM1_V1_D0C_DOOR_EDGE_ORNAMENT_HASH_PC34);

    return rc == 0 ? 0 : 1;
}
