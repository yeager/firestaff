#include "dm1_v1_action_spell_runtime_capture_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_lifecycle(DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 *value, int action)
{
    memset(value, 0, sizeof(*value));
    value->accepted = 1; value->m11SourceFrameCurrent = 1;
    value->suppressSyntheticFallback = 1; value->frameTick = 901;
    value->sourceTick = 101; value->serial = 10; value->commandFingerprint = 0x41u;
    value->orderingFingerprint = 0x42u;
    value->presentationKind = action ? DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34
                                     : DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    value->originalGraphicId = action ? 10 : 9; value->originalZoneId = action ? 11 : 13;
    value->companionGraphicId = action ? 0 : 11; value->sourceAssetCount = action ? 1 : 2;
    value->sourceCommandCount = action ? 3 : 4;
}

int main(void)
{
    DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellRuntimeCaptureReceiptPc34 capture;
    set_lifecycle(&lifecycle, 1);
    lifecycle.clearStaleSourceFrame = lifecycle.revokeStaleSourceFrame = 1;
    lifecycle.staleOriginalGraphicId = 9; lifecycle.staleOriginalZoneId = 13;
    lifecycle.staleCompanionGraphicId = 11;
    CHECK(dm1_v1_action_spell_runtime_capture_build_pc34(&lifecycle, &capture));
    CHECK(capture.accepted && capture.runtimeCaptureCurrent && capture.originalGraphicId == 10 &&
          capture.originalZoneId == 11 && capture.revokeStaleCapture &&
          capture.staleOriginalGraphicId == 9 && capture.staleCompanionGraphicId == 11);
    set_lifecycle(&lifecycle, 0);
    CHECK(dm1_v1_action_spell_runtime_capture_build_pc34(&lifecycle, &capture));
    CHECK(capture.originalGraphicId == 9 && capture.companionGraphicId == 11 &&
          capture.sourceAssetCount == 2 && !capture.revokeStaleCapture);
    lifecycle.clearStaleSourceFrame = 1;
    CHECK(!dm1_v1_action_spell_runtime_capture_build_pc34(&lifecycle, &capture));
    lifecycle.clearStaleSourceFrame = 0; lifecycle.originalZoneId = 11;
    CHECK(!dm1_v1_action_spell_runtime_capture_build_pc34(&lifecycle, &capture));
    lifecycle.originalZoneId = 13; lifecycle.m11SourceFrameCurrent = 0;
    CHECK(!dm1_v1_action_spell_runtime_capture_build_pc34(&lifecycle, &capture));
    printf("%s\n", failures ? "failed" : "ok: action/spell runtime capture");
    return failures ? 1 : 0;
}
