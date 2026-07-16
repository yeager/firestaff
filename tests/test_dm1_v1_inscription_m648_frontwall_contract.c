#include "dm1_v1_inscription_font_pc34_compat.h"

#include <stdio.h>

int main(void)
{
    const unsigned char glyphs[] = { 'D', 'M', 0x81u };
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34 plan;

    if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
            glyphs, (int)sizeof(glyphs), 0, 0, 224, 136, &plan) ||
        DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 != 258 ||
        DM1_V1_INSCRIPTION_GLYPH_WIDTH != 8 ||
        DM1_V1_INSCRIPTION_GLYPH_HEIGHT != 8 ||
        DM1_V1_INSCRIPTION_TRANSPARENT_COLOR != 10 ||
        plan.glyphCount != 2 || plan.textWidth != 16 ||
        plan.textX != 104 || plan.textY != 41 || !plan.done) {
        return 1;
    }
    puts("ok: DM1 M648 inscription keeps PC34 cells and C10 transparency");
    return 0;
}
