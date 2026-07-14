#include "dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    uint8_t frame[DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34 *
                  DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34];
    uint16_t palette[DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34];
    DM1_V1_F0902_DrawFTLLogoOriginalInput_PC34 input;
    DM1_V1_F0902_DrawFTLLogoPresentationPlan_PC34 plan;

    memset(frame, 0x5a, sizeof(frame));
    memset(palette, 0, sizeof(palette));
    memset(&input, 0, sizeof(input));
    input.packedFrame = frame;
    input.packedFrameBytes = sizeof(frame);
    input.frameWidth = DM1_V1_F0902_FTL_LOGO_WIDTH_PC34;
    input.frameHeight = DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34;
    input.frameStrideBytes = DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34;
    input.palette = palette;
    input.paletteColorCount = DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34;

    check(dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(&input, &plan),
          "original FTL frame and palette authorize a plan");
    check(plan.kind == DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_BLIT_PACKED_FRAME_PC34 &&
              plan.packedFrame == frame && plan.packedFrameBytes == sizeof(frame) &&
              plan.srcWidth == 320u && plan.srcHeight == 200u &&
              plan.srcStrideBytes == 160u && plan.dstX == 0u && plan.dstY == 0u &&
              plan.palette == palette && plan.paletteColorCount == 16u,
          "plan preserves the original full-screen frame and palette");

    input.packedFrame = NULL;
    check(!dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(&input, &plan) &&
              plan.kind == DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_NONE_PC34,
          "missing original frame fails closed");

    input.packedFrame = frame;
    input.palette = NULL;
    check(!dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(&input, &plan) &&
              plan.kind == DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_NONE_PC34,
          "missing original palette fails closed");

    input.palette = palette;
    input.packedFrameBytes = sizeof(frame) - 1u;
    check(!dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(&input, &plan) &&
              plan.kind == DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_NONE_PC34,
          "truncated original frame fails closed");

    input.packedFrameBytes = sizeof(frame);
    input.paletteColorCount = 15u;
    check(!dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(&input, &plan) &&
              plan.kind == DM1_V1_F0902_DRAW_FTL_LOGO_PLAN_NONE_PC34,
          "incomplete original palette fails closed");

    return failures ? 1 : 0;
}
