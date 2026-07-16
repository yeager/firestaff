#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    V1_TitleFrontendSourceTiming timing;
    int palette;

    timing = V1_TitleFrontend_GetSourceTimingEvidence();
    if (!V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS, &palette) ||
        palette != VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS ||
        !V1_TitleFrontend_GetFallbackFramePalette(1u, &palette) ||
        palette != VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS ||
        !V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT, &palette) ||
        palette != VGA_PALETTE_PC34_SPECIAL_TITLE ||
        !V1_TitleFrontend_GetStepPalette(
            V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT,
            &palette) ||
        palette != VGA_PALETTE_PC34_SPECIAL_TITLE ||
        VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS ==
            VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS ||
        timing.zoomStepCount != 18u ||
        timing.vblankBeforeEachZoomStep != 1u ||
        timing.postZoomVblankCount != 2u ||
        timing.finalFadeGuardVblankCount != 1u) {
        return 1;
    }
    puts("ok: DM1 TITLE.C C12/C13/C14 phases and cadence are source-locked");
    return 0;
}
