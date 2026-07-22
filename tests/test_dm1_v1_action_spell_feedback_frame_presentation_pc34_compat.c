#include "dm1_v1_action_spell_feedback_frame_presentation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

int
main(void)
{
    DM1_V1_ActionSpellResultFeedbackReceiptPc34 feedback;
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 order;
    DM1_V1_ActionSpellPresentationLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34 presentation;

    memset(&feedback, 0, sizeof(feedback));
    memset(&order, 0, sizeof(order));
    memset(&lifecycle, 0, sizeof(lifecycle));
    feedback.accepted = 1;
    feedback.resultKind = DM1_V1_ACTION_SPELL_RESULT_SPELL_FAILURE_PC34;
    feedback.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    feedback.championIndex = 1;
    feedback.inputZoneId = 258;
    feedback.requiresCommandRepaint = 1;
    feedback.suppressSyntheticFallback = 1;
    feedback.sourceTick = 70;
    feedback.serial = 9;
    feedback.commandFingerprint = 0x82f1a3c5u;
    order.accepted = 1;
    order.readyForPresentation = 1;
    order.presentationKind = feedback.presentationKind;
    order.commandCount = 4;
    order.frameTick = 900;
    order.sourceTick = feedback.sourceTick;
    order.serial = feedback.serial;
    order.commandFingerprint = feedback.commandFingerprint;
    order.orderingFingerprint = 0x5db32e7au;
    lifecycle.accepted = 1;
    lifecycle.repainted = 1;
    lifecycle.presentationKind = feedback.presentationKind;
    lifecycle.frameTick = order.frameTick;
    lifecycle.sourceTick = feedback.sourceTick;
    lifecycle.serial = feedback.serial;
    lifecycle.commandFingerprint = feedback.commandFingerprint;
    lifecycle.lifecycleGeneration = 12;

    CHECK(dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
              &feedback, &order, &lifecycle, &presentation));
    CHECK(presentation.accepted && presentation.commandRepaintCurrent &&
          presentation.suppressSyntheticFallback && presentation.frameTick == 900 &&
          presentation.lifecycleGeneration == 12);

    lifecycle.frameTick++;
    CHECK(!dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
              &feedback, &order, &lifecycle, &presentation));
    lifecycle.frameTick--;
    lifecycle.repainted = 0;
    lifecycle.alreadyCurrent = 0;
    CHECK(!dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
              &feedback, &order, &lifecycle, &presentation));
    lifecycle.alreadyCurrent = 1;
    order.commandFingerprint++;
    CHECK(!dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
              &feedback, &order, &lifecycle, &presentation));
    order.commandFingerprint--;
    feedback.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_feedback_frame_presentation_build_pc34(
              &feedback, &order, &lifecycle, &presentation));

    printf("%s\n", failures ? "failed" : "ok: action/spell feedback frame presentation");
    return failures ? 1 : 0;
}
