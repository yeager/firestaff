#include "dm1_v1_action_spell_feedback_frame_presentation_pc34_compat.h"

#include <string.h>

int
dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
    const DM1_V1_ActionSpellResultFeedbackReceiptPc34 *feedback,
    const DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *order,
    const DM1_V1_ActionSpellPresentationLifecycleReceiptPc34 *lifecycle,
    DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34 *outReceipt)
{
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!feedback || !order || !lifecycle || !feedback->accepted ||
        !feedback->requiresCommandRepaint ||
        !feedback->suppressSyntheticFallback || !order->accepted ||
        !order->readyForPresentation || !lifecycle->accepted ||
        (!lifecycle->repainted && !lifecycle->alreadyCurrent) ||
        lifecycle->lifecycleGeneration == 0 ||
        feedback->presentationKind != order->presentationKind ||
        feedback->presentationKind != lifecycle->presentationKind ||
        feedback->sourceTick != order->sourceTick ||
        feedback->sourceTick != lifecycle->sourceTick ||
        feedback->serial != order->serial ||
        feedback->serial != lifecycle->serial ||
        feedback->commandFingerprint != order->commandFingerprint ||
        feedback->commandFingerprint != lifecycle->commandFingerprint ||
        lifecycle->frameTick != order->frameTick ||
        order->commandCount <= 0 || order->orderingFingerprint == 0) {
        return 0;
    }

    outReceipt->accepted = 1;
    outReceipt->resultKind = feedback->resultKind;
    outReceipt->presentationKind = feedback->presentationKind;
    outReceipt->championIndex = feedback->championIndex;
    outReceipt->inputZoneId = feedback->inputZoneId;
    outReceipt->commandRepaintCurrent = 1;
    outReceipt->suppressSyntheticFallback = 1;
    outReceipt->frameTick = lifecycle->frameTick;
    outReceipt->sourceTick = feedback->sourceTick;
    outReceipt->serial = feedback->serial;
    outReceipt->commandFingerprint = feedback->commandFingerprint;
    outReceipt->orderingFingerprint = order->orderingFingerprint;
    outReceipt->lifecycleGeneration = lifecycle->lifecycleGeneration;
    return 1;
}
