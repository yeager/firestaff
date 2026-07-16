#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    V1_TitleFrontendSourceAnimationStep presents;
    V1_TitleFrontendSourceAnimationStep zoom;
    V1_TitleFrontendC001BlitPlan presents_plan;
    V1_TitleFrontendC001BlitPlan zoom_plan;
    int palette;

    if (!V1_TitleFrontend_GetSourceAnimationStep(1u, &presents) ||
        !V1_TitleFrontend_GetSourceAnimationStep(2u, &zoom) ||
        !V1_TitleFrontend_GetC001BlitPlanForStep(&presents, &presents_plan) ||
        !V1_TitleFrontend_GetC001BlitPlanForStep(&zoom, &zoom_plan) ||
        !V1_TitleFrontend_GetStepPalette(presents.kind, &palette) ||
        palette != VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS ||
        palette == VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_PRESENTS ||
        presents_plan.kind != V1_TITLE_FRONTEND_C001_BLIT_REGION ||
        presents_plan.srcY != 137u || presents_plan.dstY != 90u ||
        !V1_TitleFrontend_GetStepPalette(zoom.kind, &palette) ||
        palette != VGA_PALETTE_PC34_SPECIAL_TITLE ||
        zoom_plan.kind != V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION ||
        !zoom_plan.clearBeforeVblank ||
        zoom_plan.specialPaletteBeforeVblank !=
            VGA_PALETTE_PC34_SPECIAL_TITLE ||
        zoom_plan.paletteLoadCountBeforeVblank != 2u) {
        return 1;
    }
    puts("ok: DM1 title host palette apply keeps PRESENTS and zoom separate");
    return 0;
}
