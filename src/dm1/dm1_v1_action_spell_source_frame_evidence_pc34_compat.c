#include "dm1_v1_action_spell_source_frame_evidence_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_source_frame_evidence_assets_valid_pc34(
    const DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 *assets)
{
    if (!assets || !assets->accepted || !assets->suppressSyntheticFallback ||
        assets->sourceCommandCount <= 0 || assets->frameTick == 0 ||
        assets->sourceTick == 0 || assets->serial == 0 ||
        assets->commandFingerprint == 0 || assets->orderingFingerprint == 0) return 0;
    if (assets->presentationKind == DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34) {
        return assets->originalGraphicId == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
               assets->originalZoneId == DM1_V1_ACTION_AREA_ZONE_ID_PC34 &&
               assets->companionGraphicId == 0 && assets->sourceAssetCount == 1;
    }
    if (assets->presentationKind >= DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 &&
        assets->presentationKind <= DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34) {
        return assets->originalGraphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
               assets->originalZoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
               assets->companionGraphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
               assets->sourceAssetCount == 2;
    }
    return 0;
}

static int
dm1_v1_action_spell_source_frame_evidence_same_pc34(
    const DM1_V1_ActionSpellSourceFrameEvidenceStatePc34 *state,
    const DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 *assets)
{
    return state->originalGraphicId == assets->originalGraphicId &&
           state->originalZoneId == assets->originalZoneId &&
           state->companionGraphicId == assets->companionGraphicId &&
           state->sourceAssetCount == assets->sourceAssetCount &&
           state->sourceTick == assets->sourceTick && state->serial == assets->serial &&
           state->commandFingerprint == assets->commandFingerprint &&
           state->orderingFingerprint == assets->orderingFingerprint;
}

int
dm1_v1_action_spell_source_frame_evidence_apply_pc34(
    DM1_V1_ActionSpellSourceFrameEvidenceStatePc34 *state,
    const DM1_V1_ActionSpellSourceAssetRuntimeReceiptPc34 *assets,
    DM1_V1_ActionSpellSourceFrameEvidenceReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_source_frame_evidence_assets_valid_pc34(assets)) {
        return 0;
    }
    if (state->active && assets->frameTick < state->frameTick) return 0;
    if (state->active && assets->frameTick == state->frameTick &&
        !dm1_v1_action_spell_source_frame_evidence_same_pc34(state, assets)) return 0;
    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->liveSourceFrameCurrent = 1;
        outReceipt->clearStaleSourceFrame = state->active && assets->frameTick > state->frameTick;
        outReceipt->revokeStaleSourceFrame = outReceipt->clearStaleSourceFrame;
        outReceipt->alreadyCurrent = state->active && assets->frameTick == state->frameTick;
        outReceipt->presentationKind = assets->presentationKind;
        outReceipt->originalGraphicId = assets->originalGraphicId;
        outReceipt->originalZoneId = assets->originalZoneId;
        outReceipt->companionGraphicId = assets->companionGraphicId;
        outReceipt->sourceAssetCount = assets->sourceAssetCount;
        outReceipt->sourceCommandCount = assets->sourceCommandCount;
        outReceipt->suppressSyntheticFallback = 1;
        if (outReceipt->clearStaleSourceFrame) {
            outReceipt->staleOriginalGraphicId = state->originalGraphicId;
            outReceipt->staleOriginalZoneId = state->originalZoneId;
            outReceipt->staleCompanionGraphicId = state->companionGraphicId;
        }
        outReceipt->frameTick = assets->frameTick;
        outReceipt->sourceTick = assets->sourceTick;
        outReceipt->serial = assets->serial;
        outReceipt->commandFingerprint = assets->commandFingerprint;
        outReceipt->orderingFingerprint = assets->orderingFingerprint;
    }
    state->active = 1;
    state->originalGraphicId = assets->originalGraphicId;
    state->originalZoneId = assets->originalZoneId;
    state->companionGraphicId = assets->companionGraphicId;
    state->sourceAssetCount = assets->sourceAssetCount;
    state->frameTick = assets->frameTick;
    state->sourceTick = assets->sourceTick;
    state->serial = assets->serial;
    state->commandFingerprint = assets->commandFingerprint;
    state->orderingFingerprint = assets->orderingFingerprint;
    return 1;
}
