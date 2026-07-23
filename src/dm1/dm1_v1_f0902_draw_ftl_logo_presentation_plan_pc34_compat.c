#include "dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"

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

static uint32_t dm1_v1_f0902_fnv1a_pc34(const uint8_t *bytes, size_t byteCount)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0U; i < byteCount; ++i) {
        hash ^= (uint32_t)bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_f0902_draw_ftl_logo_real_media_pc34(
    const DM1_V1_F0902_RealMediaInput_PC34 *input,
    DM1_V1_F0902_RealMediaReceipt_PC34 *outReceipt)
{
    SWSH_CompatLogoPayload payload;
    DM1_V1_F0902_DrawFTLLogoOriginalInput_PC34 original;
    SWSH_CompatSourceTiming timing;
    unsigned int stepOrdinal;
    int result = 0;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!input || !input->sourceMedia || input->sourceMediaBytes == 0U ||
        !input->sourceMediaHashVerified ||
        (input->sourceMediaKind != DM1_V1_F0902_SOURCE_MEDIA_PC34_SWOOSH_PC34 &&
         input->sourceMediaKind != DM1_V1_F0902_SOURCE_MEDIA_IIGS_SWSHIIGS_PC34) ||
        !input->decodedPackedFrame ||
        input->decodedPackedFrameCapacity <
            DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34 *
                DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34 ||
        !input->sourcePalette ||
        input->sourcePaletteCapacity < DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34 ||
        input->sourceMediaBytes > 0xffffffffU) {
        return 0;
    }

    memset(&payload, 0, sizeof(payload));
    if (!SWSH_Compat_FindLogoImagePayloadEx(
            input->sourceMedia, (unsigned int)input->sourceMediaBytes, &payload)) {
        return 0;
    }

    SWSH_Compat_ExpandLogoToBitmap(payload.payload, input->decodedPackedFrame);
    memset(input->sourcePalette, 0,
           DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34 * sizeof(*input->sourcePalette));
    for (stepOrdinal = 1U;
         stepOrdinal <= SWSH_Compat_GetSourceAnimationStepCount();
         ++stepOrdinal) {
        SWSH_CompatSourceAnimationStep step;
        if (!SWSH_Compat_GetSourceAnimationStep(stepOrdinal, &step)) {
            goto cleanup;
        }
        if (step.kind == SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR &&
            step.colorIndex < DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34) {
            input->sourcePalette[step.colorIndex] = (uint16_t)step.colorValue;
        }
    }

    memset(&original, 0, sizeof(original));
    original.packedFrame = input->decodedPackedFrame;
    original.packedFrameBytes =
        DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34 * DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34;
    original.frameWidth = DM1_V1_F0902_FTL_LOGO_WIDTH_PC34;
    original.frameHeight = DM1_V1_F0902_FTL_LOGO_HEIGHT_PC34;
    original.frameStrideBytes = DM1_V1_F0902_FTL_LOGO_PACKED_STRIDE_PC34;
    original.palette = input->sourcePalette;
    original.paletteColorCount = DM1_V1_F0902_FTL_LOGO_PALETTE_COLORS_PC34;
    if (!dm1_v1_f0902_draw_ftl_logo_presentation_plan_pc34(
            &original, &outReceipt->plan)) {
        goto cleanup;
    }

    timing = SWSH_Compat_GetSourceTimingEvidence();
    outReceipt->valid = 1;
    outReceipt->sourceMediaKind = input->sourceMediaKind;
    outReceipt->sourceMediaFnv1a = dm1_v1_f0902_fnv1a_pc34(
        input->sourceMedia, input->sourceMediaBytes);
    if (!payload.ownedBytes) {
        outReceipt->sourcePayloadStoredInMedia = 1;
        outReceipt->sourcePayloadOffset = (size_t)(payload.payload - input->sourceMedia);
    }
    outReceipt->paletteCommandCount = timing.paletteCommandCount;
    outReceipt->paletteWaitVblanks = timing.paletteWaitVblankCount;
    outReceipt->initialHoldVblanks = SWSH_COMPAT_SOURCE_INITIAL_LOGO_HOLD_VBLANKS;
    outReceipt->finalHoldVblanks = SWSH_COMPAT_SOURCE_FINAL_HOLD_VBLANKS;
    result = 1;

cleanup:
    SWSH_Compat_ReleaseLogoImagePayload(&payload);
    if (!result) memset(outReceipt, 0, sizeof(*outReceipt));
    return result;
}
