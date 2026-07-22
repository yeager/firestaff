#include "dm1_v1_action_spell_final_hud_paint_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

int
main(void)
{
    DM1_V1_ActionSpellFeedbackFramePresentationReceiptPc34 feedbackFrame;
    DM1_V1_ActionSpellFinalHudPaintReceiptPc34 paint;

    memset(&feedbackFrame, 0, sizeof(feedbackFrame));
    feedbackFrame.accepted = 1;
    feedbackFrame.resultKind = DM1_V1_ACTION_SPELL_RESULT_SPELL_FAILURE_PC34;
    feedbackFrame.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    feedbackFrame.championIndex = 1;
    feedbackFrame.commandRepaintCurrent = 1;
    feedbackFrame.suppressSyntheticFallback = 1;
    feedbackFrame.frameTick = 900;
    feedbackFrame.sourceTick = 70;
    feedbackFrame.serial = 9;
    feedbackFrame.commandFingerprint = 0x82f1a3c5u;
    feedbackFrame.orderingFingerprint = 0x5db32e7au;
    feedbackFrame.lifecycleGeneration = 12;
    CHECK(dm1_v1_action_spell_final_hud_paint_build_pc34(&feedbackFrame, &paint));
    CHECK(paint.accepted && paint.clearBeforeRender && paint.suppressSyntheticFallback &&
          paint.clearRect.x == 224 && paint.clearRect.y == 42 &&
          paint.clearRect.w == 96 && paint.clearRect.h == 33 &&
          paint.renderRect.x == 233 && paint.renderRect.y == 42 &&
          paint.renderRect.w == 87 && paint.renderRect.h == 25 &&
          paint.sourceGraphicId == 9 && paint.sourceZoneId == 13);

    feedbackFrame.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
    feedbackFrame.resultKind = DM1_V1_ACTION_SPELL_RESULT_ACTION_SUCCESS_PC34;
    CHECK(dm1_v1_action_spell_final_hud_paint_build_pc34(&feedbackFrame, &paint));
    CHECK(paint.clearRect.x == 224 && paint.clearRect.y == 77 &&
          paint.clearRect.w == 96 && paint.clearRect.h == 45 &&
          paint.renderRect.x == 233 && paint.renderRect.y == 77 &&
          paint.renderRect.w == 87 && paint.renderRect.h == 45 &&
          paint.sourceGraphicId == 10 && paint.sourceZoneId == 11);

    feedbackFrame.commandRepaintCurrent = 0;
    CHECK(!dm1_v1_action_spell_final_hud_paint_build_pc34(&feedbackFrame, &paint));
    feedbackFrame.commandRepaintCurrent = 1;
    feedbackFrame.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34;
    CHECK(!dm1_v1_action_spell_final_hud_paint_build_pc34(&feedbackFrame, &paint));
    feedbackFrame.presentationKind = DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34;
    feedbackFrame.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_final_hud_paint_build_pc34(&feedbackFrame, &paint));

    printf("%s\n", failures ? "failed" : "ok: action/spell final HUD paint");
    return failures ? 1 : 0;
}
