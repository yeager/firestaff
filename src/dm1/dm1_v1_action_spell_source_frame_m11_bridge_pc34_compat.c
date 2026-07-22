#include "dm1_v1_action_spell_source_frame_m11_bridge_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_source_frame_m11_bridge_asset_proof_pc34(
    int graphicId, int zoneId, int companionGraphicId)
{
    return (graphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
            zoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 && companionGraphicId == 0) ||
           (graphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
            zoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
            companionGraphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34);
}

static int
dm1_v1_action_spell_source_frame_m11_bridge_current_valid_pc34(
    const DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 *evidence)
{
    if (!evidence || !evidence->accepted || !evidence->liveSourceFrameCurrent ||
        !evidence->suppressSyntheticFallback || evidence->sourceCommandCount <= 0 ||
        evidence->frameTick == 0 || evidence->sourceTick == 0 || evidence->serial == 0 ||
        evidence->commandFingerprint == 0 || evidence->orderingFingerprint == 0 ||
        !dm1_v1_action_spell_source_frame_m11_bridge_asset_proof_pc34(
            evidence->originalGraphicId, evidence->originalZoneId,
            evidence->companionGraphicId)) return 0;
    if (evidence->presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34) {
        return evidence->originalGraphicId == 10 && evidence->sourceAssetCount == 1;
    }
    return evidence->presentationKind >= DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 &&
           evidence->presentationKind <= DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34 &&
           evidence->originalGraphicId == 9 && evidence->sourceAssetCount == 2;
}

int
dm1_v1_action_spell_source_frame_m11_bridge_build_pc34(
    const DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 *evidence,
    DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_action_spell_source_frame_m11_bridge_current_valid_pc34(evidence) ||
        evidence->clearStaleSourceFrame != evidence->revokeStaleSourceFrame) {
        return 0;
    }
    if (evidence->clearStaleSourceFrame &&
        !dm1_v1_action_spell_source_frame_m11_bridge_asset_proof_pc34(
            evidence->staleOriginalGraphicId, evidence->staleOriginalZoneId,
            evidence->staleCompanionGraphicId)) return 0;

    outReceipt->accepted = 1;
    outReceipt->m11SourceFrameReady = 1;
    outReceipt->presentationKind = evidence->presentationKind;
    outReceipt->originalGraphicId = evidence->originalGraphicId;
    outReceipt->originalZoneId = evidence->originalZoneId;
    outReceipt->companionGraphicId = evidence->companionGraphicId;
    outReceipt->sourceAssetCount = evidence->sourceAssetCount;
    outReceipt->sourceCommandCount = evidence->sourceCommandCount;
    outReceipt->clearStaleSourceFrame = evidence->clearStaleSourceFrame;
    outReceipt->revokeStaleSourceFrame = evidence->revokeStaleSourceFrame;
    outReceipt->staleOriginalGraphicId = evidence->staleOriginalGraphicId;
    outReceipt->staleOriginalZoneId = evidence->staleOriginalZoneId;
    outReceipt->staleCompanionGraphicId = evidence->staleCompanionGraphicId;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->frameTick = evidence->frameTick;
    outReceipt->sourceTick = evidence->sourceTick;
    outReceipt->serial = evidence->serial;
    outReceipt->commandFingerprint = evidence->commandFingerprint;
    outReceipt->orderingFingerprint = evidence->orderingFingerprint;
    return 1;
}
