#include "dm1_v1_action_spell_render_consumption_lifecycle_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_render_consumption_lifecycle_valid_pc34(
    const DM1_V1_ActionSpellRenderConsumptionReceiptPc34 *consumption)
{
    return consumption && consumption->accepted && consumption->renderReadyForHost &&
           consumption->suppressSyntheticFallback && consumption->clearCount > 0 &&
           consumption->sourceGraphicId > 0 && consumption->sourceZoneId > 0 &&
           consumption->renderRect.w > 0 && consumption->renderRect.h > 0 &&
           consumption->frameTick > 0 && consumption->sourceTick > 0 &&
           consumption->serial > 0 && consumption->commandFingerprint > 0 &&
           consumption->orderingFingerprint > 0 && consumption->lifecycleGeneration > 0;
}

static int
dm1_v1_action_spell_render_consumption_lifecycle_same_pc34(
    const DM1_V1_ActionSpellRenderConsumptionLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellRenderConsumptionReceiptPc34 *consumption)
{
    return state->sourceGraphicId == consumption->sourceGraphicId &&
           state->sourceZoneId == consumption->sourceZoneId &&
           state->sourceTick == consumption->sourceTick &&
           state->serial == consumption->serial &&
           state->commandFingerprint == consumption->commandFingerprint &&
           state->orderingFingerprint == consumption->orderingFingerprint &&
           state->lifecycleGeneration == consumption->lifecycleGeneration;
}

int
dm1_v1_action_spell_render_consumption_lifecycle_apply_pc34(
    DM1_V1_ActionSpellRenderConsumptionLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellRenderConsumptionReceiptPc34 *consumption,
    DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_render_consumption_lifecycle_valid_pc34(
            consumption)) return 0;
    if (state->active) {
        if (consumption->frameTick < state->frameTick) return 0;
        if (consumption->frameTick == state->frameTick) {
            if (!dm1_v1_action_spell_render_consumption_lifecycle_same_pc34(
                    state, consumption)) return 0;
            if (outReceipt) {
                outReceipt->accepted = 1;
                outReceipt->hostConsumptionCurrent = 1;
                outReceipt->alreadyCurrent = 1;
                outReceipt->suppressSyntheticFallback = 1;
            }
        } else {
            if (outReceipt) {
                outReceipt->accepted = 1;
                outReceipt->hostConsumptionCurrent = 1;
                outReceipt->retirePreviousHostConsumption = 1;
                outReceipt->suppressSyntheticFallback = 1;
            }
        }
    } else if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->hostConsumptionCurrent = 1;
        outReceipt->suppressSyntheticFallback = 1;
    }
    if (outReceipt) {
        outReceipt->sourceGraphicId = consumption->sourceGraphicId;
        outReceipt->sourceZoneId = consumption->sourceZoneId;
        outReceipt->renderRect = consumption->renderRect;
        outReceipt->frameTick = consumption->frameTick;
        outReceipt->sourceTick = consumption->sourceTick;
        outReceipt->serial = consumption->serial;
        outReceipt->commandFingerprint = consumption->commandFingerprint;
        outReceipt->orderingFingerprint = consumption->orderingFingerprint;
        outReceipt->lifecycleGeneration = consumption->lifecycleGeneration;
    }
    state->active = 1;
    state->sourceGraphicId = consumption->sourceGraphicId;
    state->sourceZoneId = consumption->sourceZoneId;
    state->frameTick = consumption->frameTick;
    state->sourceTick = consumption->sourceTick;
    state->serial = consumption->serial;
    state->commandFingerprint = consumption->commandFingerprint;
    state->orderingFingerprint = consumption->orderingFingerprint;
    state->lifecycleGeneration = consumption->lifecycleGeneration;
    return 1;
}
