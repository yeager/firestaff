/*
 * ReDMCSB ENTRANCE.C F0807_ENTRANCE_DrawAnimationStep, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0807_ENTRANCE_DRAW_ANIMATION_STEP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0807_ENTRANCE_DRAW_ANIMATION_STEP_PC34_COMPAT_H

#include <stdint.h>

#include "redmcsb_f0766_blit_to_screen_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB CM1_COLOR_NO_TRANSPARENCY. */
#define REDMCSB_F0807_COLOR_NO_TRANSPARENCY_PC34_COMPAT INT16_C(-1)

/*
 * ENTRANCE.C F0807 delegates one already-composed entrance animation frame
 * to F0766. The renderer owns all bitmap pixels and screen storage.
 */
void redmcsb_f0807_entrance_draw_animation_step_pc34_compat(
    const redmcsb_f0766_renderer_pc34_compat *renderer,
    const void *entrance_animation_step,
    const int16_t xyz[4]);

const char *redmcsb_f0807_entrance_draw_animation_step_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
