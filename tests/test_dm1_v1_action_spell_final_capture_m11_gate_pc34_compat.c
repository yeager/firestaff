#include "dm1_v1_action_spell_final_capture_m11_gate_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)
static void set_capture(DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 *v, int action) {
    memset(v, 0, sizeof(*v)); v->accepted = v->finalCaptureCurrent = v->suppressSyntheticFallback = 1;
    v->presentationKind = action ? DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 : DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34;
    v->graphicId = action ? 10 : 9; v->zoneId = action ? 11 : 13; v->companionGraphicId = action ? 0 : 11;
    v->assetCount = action ? 1 : 2; v->sourceCommandCount = action ? 3 : 4;
    v->frameTick = 901; v->sourceTick = 101; v->serial = 10; v->commandFingerprint = 0x41u; v->orderingFingerprint = 0x42u;
}
int main(void) {
    DM1_V1_ActionSpellM11CaptureLifecycleReceiptPc34 capture;
    DM1_V1_ActionSpellFinalCaptureM11GateReceiptPc34 gate;
    set_capture(&capture, 1);
    CHECK(dm1_v1_action_spell_final_capture_m11_gate_build_pc34(&capture, &gate));
    CHECK(gate.accepted && gate.m11CaptureGateOpen && gate.originalGraphicId == 10 && gate.originalZoneId == 11);
    set_capture(&capture, 0);
    CHECK(dm1_v1_action_spell_final_capture_m11_gate_build_pc34(&capture, &gate));
    CHECK(gate.originalGraphicId == 9 && gate.originalZoneId == 13 && gate.companionGraphicId == 11);
    capture.zoneId = 11;
    CHECK(!dm1_v1_action_spell_final_capture_m11_gate_build_pc34(&capture, &gate));
    capture.zoneId = 13; capture.finalCaptureCurrent = 0;
    CHECK(!dm1_v1_action_spell_final_capture_m11_gate_build_pc34(&capture, &gate));
    capture.finalCaptureCurrent = 1; capture.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_final_capture_m11_gate_build_pc34(&capture, &gate));
    printf("%s\n", failures ? "failed" : "ok: action/spell final capture M11 gate"); return failures ? 1 : 0;
}
