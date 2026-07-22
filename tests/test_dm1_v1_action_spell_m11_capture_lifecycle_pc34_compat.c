#include "dm1_v1_action_spell_m11_capture_lifecycle_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)
static DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 input(unsigned int tick, int action) {
    DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 v;
    memset(&v, 0, sizeof(v)); v.accepted = v.m11SourceFrameCurrent = v.suppressSyntheticFallback = 1;
    v.presentationKind = action ? DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 : DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    v.originalGraphicId = action ? 10 : 9; v.originalZoneId = action ? 11 : 13;
    v.companionGraphicId = action ? 0 : 11; v.sourceAssetCount = action ? 1 : 2; v.sourceCommandCount = action ? 3 : 4;
    v.frameTick = tick; v.sourceTick = tick - 800; v.serial = tick - 891;
    v.commandFingerprint = tick + 0x10u; v.orderingFingerprint = tick + 0x20u; return v;
}
int main(void) {
    DM1_V1_ActionSpellM11CaptureLifecycleStatePc34 state;
    DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 capture;
    DM1_V1_ActionSpellSourceFrameM11LifecycleReceiptPc34 spell = input(900, 0), action = input(901, 1);
    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(&state, &spell, &capture));
    CHECK(capture.finalCaptureCurrent && !capture.clearStaleCapture && capture.graphicId == 9);
    CHECK(dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(&state, &action, &capture));
    CHECK(capture.clearStaleCapture && capture.revokeStaleCapture && capture.staleGraphicId == 9 && capture.graphicId == 10);
    CHECK(dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(&state, &action, &capture));
    CHECK(capture.alreadyCurrent && !capture.clearStaleCapture);
    CHECK(!dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(&state, &spell, &capture));
    action.originalZoneId = 13;
    CHECK(!dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(&state, &action, &capture));
    action.originalZoneId = 11; action.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_m11_capture_lifecycle_apply_pc34(&state, &action, &capture));
    printf("%s\n", failures ? "failed" : "ok: action/spell M11 capture lifecycle"); return failures ? 1 : 0;
}
