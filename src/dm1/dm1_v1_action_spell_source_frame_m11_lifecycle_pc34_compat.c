#include "dm1_v1_action_spell_source_frame_m11_lifecycle_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_source_frame_m11_lifecycle_proof_pc34(int g, int z, int c, int count)
{
    return (g == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 &&
            z == DM1_V1_ACTION_AREA_ZONE_ID_PC34 && c == 0 && count == 1) ||
           (g == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
            z == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
            c == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 && count == 2);
}

static int
dm1_v1_action_spell_source_frame_m11_lifecycle_valid_pc34(
    const DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 *bridge)
{
    return bridge && bridge->accepted && bridge->m11SourceFrameReady &&
           bridge->suppressSyntheticFallback && bridge->sourceCommandCount > 0 &&
           bridge->frameTick > 0 && bridge->sourceTick > 0 && bridge->serial > 0 &&
           bridge->commandFingerprint > 0 && bridge->orderingFingerprint > 0 &&
           bridge->clearStaleSourceFrame == bridge->revokeStaleSourceFrame &&
           dm1_v1_action_spell_source_frame_m11_lifecycle_proof_pc34(
               bridge->originalGraphicId, bridge->originalZoneId,
               bridge->companionGraphicId, bridge->sourceAssetCount) &&
           (!bridge->clearStaleSourceFrame ||
            dm1_v1_action_spell_source_frame_m11_lifecycle_proof_pc34(
                bridge->staleOriginalGraphicId, bridge->staleOriginalZoneId,
                bridge->staleCompanionGraphicId,
                bridge->staleOriginalGraphicId == 10 ? 1 : 2));
}

static int
dm1_v1_action_spell_source_frame_m11_lifecycle_same_pc34(
    const DM1_V1_ActionSpellSourceFrameM11LifecycleStatePc34 *state,
    const DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 *bridge)
{
    return state->originalGraphicId == bridge->originalGraphicId &&
           state->originalZoneId == bridge->originalZoneId &&
           state->companionGraphicId == bridge->companionGraphicId &&
           state->sourceAssetCount == bridge->sourceAssetCount &&
           state->sourceTick == bridge->sourceTick && state->serial == bridge->serial &&
           state->commandFingerprint == bridge->commandFingerprint &&
           state->orderingFingerprint == bridge->orderingFingerprint;
}

int
dm1_v1_action_spell_source_frame_m11_lifecycle_apply_pc34(
    DM1_V1_ActionSpellSourceFrameM11LifecycleStatePc34 *state,
    const DM1_V1_ActionSpellSourceFrameM11BridgeReceiptPc34 *bridge,
    DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_source_frame_m11_lifecycle_valid_pc34(bridge)) return 0;
    if (state->active && bridge->frameTick < state->frameTick) return 0;
    if (state->active && bridge->frameTick == state->frameTick &&
        !dm1_v1_action_spell_source_frame_m11_lifecycle_same_pc34(state, bridge)) return 0;
    if (outReceipt) {
        outReceipt->accepted = 1; outReceipt->m11SourceFrameCurrent = 1;
        outReceipt->clearStaleSourceFrame = state->active && bridge->frameTick > state->frameTick;
        outReceipt->revokeStaleSourceFrame = outReceipt->clearStaleSourceFrame;
        outReceipt->alreadyCurrent = state->active && bridge->frameTick == state->frameTick;
        outReceipt->presentationKind = bridge->presentationKind;
        outReceipt->originalGraphicId = bridge->originalGraphicId;
        outReceipt->originalZoneId = bridge->originalZoneId;
        outReceipt->companionGraphicId = bridge->companionGraphicId;
        outReceipt->sourceAssetCount = bridge->sourceAssetCount;
        outReceipt->sourceCommandCount = bridge->sourceCommandCount;
        outReceipt->suppressSyntheticFallback = 1;
        if (outReceipt->clearStaleSourceFrame) {
            outReceipt->staleOriginalGraphicId = state->originalGraphicId;
            outReceipt->staleOriginalZoneId = state->originalZoneId;
            outReceipt->staleCompanionGraphicId = state->companionGraphicId;
        }
        outReceipt->frameTick = bridge->frameTick; outReceipt->sourceTick = bridge->sourceTick;
        outReceipt->serial = bridge->serial; outReceipt->commandFingerprint = bridge->commandFingerprint;
        outReceipt->orderingFingerprint = bridge->orderingFingerprint;
    }
    state->active = 1; state->originalGraphicId = bridge->originalGraphicId;
    state->originalZoneId = bridge->originalZoneId; state->companionGraphicId = bridge->companionGraphicId;
    state->sourceAssetCount = bridge->sourceAssetCount; state->frameTick = bridge->frameTick;
    state->sourceTick = bridge->sourceTick; state->serial = bridge->serial;
    state->commandFingerprint = bridge->commandFingerprint;
    state->orderingFingerprint = bridge->orderingFingerprint;
    return 1;
}
