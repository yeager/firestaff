#include "dm1_v1_action_spell_runtime_capture_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_runtime_capture_proof_pc34(int g, int z, int c, int count)
{
    return (g == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
            z == DM1_V1_ACTION_AREA_ZONE_ID_PC34 && c == 0 && count == 1) ||
           (g == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
            z == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
            c == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 && count == 2);
}

int
dm1_v1_action_spell_runtime_capture_build_pc34(
    const DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellRuntimeCaptureReceiptPc34 *outReceipt)
{
    int staleCount;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!lifecycle || !lifecycle->accepted || !lifecycle->m11SourceFrameCurrent ||
        !lifecycle->suppressSyntheticFallback || lifecycle->sourceCommandCount <= 0 ||
        lifecycle->frameTick == 0 || lifecycle->sourceTick == 0 || lifecycle->serial == 0 ||
        lifecycle->commandFingerprint == 0 || lifecycle->orderingFingerprint == 0 ||
        !dm1_v1_action_spell_runtime_capture_proof_pc34(
            lifecycle->originalGraphicId, lifecycle->originalZoneId,
            lifecycle->companionGraphicId, lifecycle->sourceAssetCount) ||
        lifecycle->clearStaleSourceFrame != lifecycle->revokeStaleSourceFrame) return 0;
    staleCount = lifecycle->staleOriginalGraphicId == 10 ? 1 : 2;
    if (lifecycle->revokeStaleSourceFrame &&
        !dm1_v1_action_spell_runtime_capture_proof_pc34(
            lifecycle->staleOriginalGraphicId, lifecycle->staleOriginalZoneId,
            lifecycle->staleCompanionGraphicId, staleCount)) return 0;
    outReceipt->accepted = 1; outReceipt->runtimeCaptureCurrent = 1;
    outReceipt->presentationKind = lifecycle->presentationKind;
    outReceipt->originalGraphicId = lifecycle->originalGraphicId;
    outReceipt->originalZoneId = lifecycle->originalZoneId;
    outReceipt->companionGraphicId = lifecycle->companionGraphicId;
    outReceipt->sourceAssetCount = lifecycle->sourceAssetCount;
    outReceipt->sourceCommandCount = lifecycle->sourceCommandCount;
    outReceipt->revokeStaleCapture = lifecycle->revokeStaleSourceFrame;
    outReceipt->staleOriginalGraphicId = lifecycle->staleOriginalGraphicId;
    outReceipt->staleOriginalZoneId = lifecycle->staleOriginalZoneId;
    outReceipt->staleCompanionGraphicId = lifecycle->staleCompanionGraphicId;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->frameTick = lifecycle->frameTick; outReceipt->sourceTick = lifecycle->sourceTick;
    outReceipt->serial = lifecycle->serial; outReceipt->commandFingerprint = lifecycle->commandFingerprint;
    outReceipt->orderingFingerprint = lifecycle->orderingFingerprint;
    return 1;
}
