#include "dm1_v1_action_spell_presentation_lifecycle_pc34_compat.h"

#include <string.h>

void
dm1_v1_action_spell_presentation_lifecycle_begin_frame_pc34(
    DM1_V1_ActionSpellPresentationLifecycleStatePc34 *state,
    unsigned int frameTick)
{
    if (!state) return;
    if (!state->frameOpen) {
        state->frameOpen = 1;
        state->frameTick = frameTick;
        return;
    }
    if (state->frameTick == frameTick) return;
    state->frameTick = frameTick;
    if (state->active) {
        state->clearRequired = 1;
        state->active = 0;
    }
}

static int
dm1_v1_action_spell_lifecycle_order_matches_frame_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order)
{
    return frameState && order && frameState->frameOpen &&
           frameState->hasPresentation && order->accepted &&
           order->readyForPresentation &&
           frameState->presentationKind == order->presentationKind &&
           frameState->frameTick == order->frameTick &&
           frameState->sourceTick == order->sourceTick &&
           frameState->serial == order->serial &&
           frameState->commandFingerprint == order->commandFingerprint &&
           frameState->commandCount == order->commandCount &&
           order->commandCount > 0;
}

int
dm1_v1_action_spell_presentation_lifecycle_apply_pc34(
    DM1_V1_ActionSpellPresentationLifecycleStatePc34 *state,
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *frameState,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    DM1_V1_ActionSpellPresentationLifecycleReceiptPc34 *outReceipt)
{
    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !state->frameOpen || !frameState || !order ||
        state->frameTick != frameState->frameTick ||
        !dm1_v1_action_spell_lifecycle_order_matches_frame_pc34(
            frameState, order)) {
        return 0;
    }
    if (state->active) {
        if (state->serial != order->serial ||
            state->commandFingerprint != order->commandFingerprint ||
            state->presentationKind != order->presentationKind) {
            return 0;
        }
        if (outReceipt) {
            outReceipt->accepted = 1;
            outReceipt->alreadyCurrent = 1;
            outReceipt->presentationKind = state->presentationKind;
            outReceipt->frameTick = state->frameTick;
            outReceipt->sourceTick = state->sourceTick;
            outReceipt->serial = state->serial;
            outReceipt->commandFingerprint = state->commandFingerprint;
            outReceipt->lifecycleGeneration = state->lifecycleGeneration;
        }
        return 1;
    }

    if (outReceipt) outReceipt->clearPrevious = state->clearRequired;
    state->clearRequired = 0;
    state->active = 1;
    state->presentationKind = order->presentationKind;
    state->sourceTick = order->sourceTick;
    state->serial = order->serial;
    state->commandFingerprint = order->commandFingerprint;
    ++state->lifecycleGeneration;

    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->repainted = 1;
        outReceipt->presentationKind = state->presentationKind;
        outReceipt->frameTick = state->frameTick;
        outReceipt->sourceTick = state->sourceTick;
        outReceipt->serial = state->serial;
        outReceipt->commandFingerprint = state->commandFingerprint;
        outReceipt->lifecycleGeneration = state->lifecycleGeneration;
    }
    return 1;
}
