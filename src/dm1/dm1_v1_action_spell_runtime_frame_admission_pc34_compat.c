#include "dm1_v1_action_spell_runtime_frame_admission_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_runtime_frame_admission_inputs_match_pc34(
    const DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *paint,
    const DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *hostRoute)
{
    return paint && hostRoute && paint->accepted && paint->hostConsumptionCurrent &&
           paint->suppressSyntheticFallback && hostRoute->accepted &&
           hostRoute->hostRenderCurrent && hostRoute->suppressSyntheticFallback &&
           paint->sourceGraphicId == hostRoute->originalGraphicId &&
           paint->sourceZoneId == hostRoute->originalZoneId &&
           paint->renderRect.x == hostRoute->originalRenderRect.x &&
           paint->renderRect.y == hostRoute->originalRenderRect.y &&
           paint->renderRect.w == hostRoute->originalRenderRect.w &&
           paint->renderRect.h == hostRoute->originalRenderRect.h &&
           paint->frameTick == hostRoute->frameTick &&
           paint->sourceTick == hostRoute->sourceTick && paint->serial == hostRoute->serial &&
           paint->commandFingerprint == hostRoute->commandFingerprint &&
           paint->orderingFingerprint == hostRoute->orderingFingerprint &&
           paint->lifecycleGeneration == hostRoute->lifecycleGeneration &&
           paint->frameTick > 0 && paint->sourceTick > 0 && paint->serial > 0 &&
           paint->commandFingerprint > 0 && paint->orderingFingerprint > 0 &&
           paint->lifecycleGeneration > 0 &&
           paint->renderRect.w > 0 && paint->renderRect.h > 0 &&
           paint->retirePreviousHostConsumption == hostRoute->clearStaleHostRoute;
}

static int
dm1_v1_action_spell_runtime_frame_admission_same_pc34(
    const DM1_V1_ActionSpellRuntimeFrameAdmissionStatePc34 *state,
    const DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *hostRoute)
{
    return state->originalRouteKind == hostRoute->originalRouteKind &&
           state->sourceGraphicId == hostRoute->originalGraphicId &&
           state->sourceZoneId == hostRoute->originalZoneId &&
           state->sourceTick == hostRoute->sourceTick && state->serial == hostRoute->serial &&
           state->commandFingerprint == hostRoute->commandFingerprint &&
           state->orderingFingerprint == hostRoute->orderingFingerprint &&
           state->lifecycleGeneration == hostRoute->lifecycleGeneration;
}

int
dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
    DM1_V1_ActionSpellRuntimeFrameAdmissionStatePc34 *state,
    const DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *paint,
    const DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *hostRoute,
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !dm1_v1_action_spell_runtime_frame_admission_inputs_match_pc34(
            paint, hostRoute)) return 0;
    if (state->active && hostRoute->frameTick < state->frameTick) return 0;
    if (state->active && hostRoute->frameTick == state->frameTick &&
        !dm1_v1_action_spell_runtime_frame_admission_same_pc34(state, hostRoute)) return 0;
    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->runtimeFrameCurrent = 1;
        outReceipt->clearStaleRuntimeFrame = state->active &&
            hostRoute->frameTick > state->frameTick;
        outReceipt->alreadyCurrent = state->active && hostRoute->frameTick == state->frameTick;
        outReceipt->originalRouteKind = hostRoute->originalRouteKind;
        outReceipt->sourceGraphicId = hostRoute->originalGraphicId;
        outReceipt->sourceZoneId = hostRoute->originalZoneId;
        outReceipt->suppressSyntheticFallback = 1;
        outReceipt->renderRect = hostRoute->originalRenderRect;
        outReceipt->frameTick = hostRoute->frameTick;
        outReceipt->sourceTick = hostRoute->sourceTick;
        outReceipt->serial = hostRoute->serial;
        outReceipt->commandFingerprint = hostRoute->commandFingerprint;
        outReceipt->orderingFingerprint = hostRoute->orderingFingerprint;
        outReceipt->lifecycleGeneration = hostRoute->lifecycleGeneration;
    }
    state->active = 1;
    state->originalRouteKind = hostRoute->originalRouteKind;
    state->sourceGraphicId = hostRoute->originalGraphicId;
    state->sourceZoneId = hostRoute->originalZoneId;
    state->frameTick = hostRoute->frameTick;
    state->sourceTick = hostRoute->sourceTick;
    state->serial = hostRoute->serial;
    state->commandFingerprint = hostRoute->commandFingerprint;
    state->orderingFingerprint = hostRoute->orderingFingerprint;
    state->lifecycleGeneration = hostRoute->lifecycleGeneration;
    return 1;
}
