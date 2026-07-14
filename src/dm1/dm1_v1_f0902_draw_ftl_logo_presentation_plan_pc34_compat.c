#include "dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34_compat.h"

#include <string.h>

int dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(
    const DM1_V1_F0902_DrawFTLLogoOriginalInput_PC34 *input,
    DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34 *outPlan)
{
    size_t requiredFrameBytes;

    if (!outPlan) {
        return 0;
    }
    memset(outPlan, 0, sizeof(*outPlan));
    if (!input || !input->packedFrame || !input->palette ||
        input->frameWidth != DM1_V1_F0902_FTL_LOGO_WIDTH_PC34 ||
        input->frameHeight != DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34 ||
        input->frameStrideBytes != DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34 ||
        input->paletteColorCount != DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34) {
        return 0;
    }

    requiredFrameBytes = (size_t)input->frameStrideBytes * input->frameHeight;
    if (input->packedFrameBytes < requiredFrameBytes) {
        return 0;
    }

    outPlan->kind = DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_BLIT_PACKED_FRAME_PC34;
    outPlan->packedFrame = input->packedFrame;
    outPlan->packedFrameBytes = requiredFrameBytes;
    outPlan->srcWidth = input->frameWidth;
    outPlan->srcHeight = input->frameHeight;
    outPlan->srcStrideBytes = input->frameStrideBytes;
    outPlan->palette = input->palette;
    outPlan->paletteColorCount = input->paletteColorCount;
    return 1;
}

const char *dm1_v1_f0902_draw_ftl_logo_evidence_pc34(void)
{
    return "ReDMCSB SWSH.C F0902_DrawFTLLogo expands Graphic_FTLLogo "
           "to the 320x200 physical screen; the SWSH palette program owns "
           "the corresponding 16 source colors.";
}
