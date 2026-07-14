#include "redmcsb_f0807_entrance_draw_animation_step_pc34_compat.h"

void redmcsb_f0807_entrance_draw_animation_step_pc34_compat(
    const redmcsb_f0766_renderer_pc34_compat *renderer,
    const void *entrance_animation_step,
    const int16_t xyz[4])
{
    redmcsb_f0766_blit_to_screen_pc34_compat(
        renderer,
        entrance_animation_step,
        xyz,
        REDMCSB_F0807_COLOR_NO_TRANSPARENCY_PC34_COMPAT);
}

const char *redmcsb_f0807_entrance_draw_animation_step_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:85-90: F0807 calls F0766_BlitToScreen with "
           "G2219_puc_EntranceAnimationStep, G2222_ai_XYZ, and "
           "CM1_COLOR_NO_TRANSPARENCY (-1).";
}
