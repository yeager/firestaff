#include "dm1_v1_action_spell_runtime_frame_admission_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_pair(unsigned int tick, int action,
         DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 *paint,
         DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 *hostRoute)
{
    int graphicId = action ? 10 : 9;
    int zoneId = action ? 11 : 13;
    DM1_V1_ActionSpellHudPaintRectPc34 rect = action
        ? (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 }
        : (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    memset(paint, 0, sizeof(*paint)); memset(hostRoute, 0, sizeof(*hostRoute));
    paint->accepted = 1; paint->hostConsumptionCurrent = 1;
    paint->retirePreviousHostConsumption = tick > 900; paint->suppressSyntheticFallback = 1;
    paint->sourceGraphicId = graphicId; paint->sourceZoneId = zoneId; paint->renderRect = rect;
    hostRoute->accepted = 1; hostRoute->hostRenderCurrent = 1;
    hostRoute->clearStaleHostRoute = tick > 900; hostRoute->suppressSyntheticFallback = 1;
    hostRoute->originalRouteKind = action
        ? DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34
        : DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34;
    hostRoute->originalGraphicId = graphicId; hostRoute->originalZoneId = zoneId;
    hostRoute->originalRenderRect = rect;
    paint->frameTick = hostRoute->frameTick = tick;
    paint->sourceTick = hostRoute->sourceTick = tick - 800;
    paint->serial = hostRoute->serial = tick - 891;
    paint->commandFingerprint = hostRoute->commandFingerprint = tick + 0x10u;
    paint->orderingFingerprint = hostRoute->orderingFingerprint = tick + 0x20u;
    paint->lifecycleGeneration = hostRoute->lifecycleGeneration = tick - 888;
}

int
main(void)
{
    DM1_V1_ActionSpellRuntimeFrameAdmissionStatePc34 state;
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 admission;
    DM1_V1_ActionSpellRenderConsumptionLifecycleReceiptPc34 spellPaint, actionPaint;
    DM1_V1_ActionSpellM11HostRenderLifecycleReceiptPc34 spellHost, actionHost;

    set_pair(900, 0, &spellPaint, &spellHost);
    set_pair(901, 1, &actionPaint, &actionHost);
    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
              &state, &spellPaint, &spellHost, &admission));
    CHECK(admission.accepted && admission.runtimeFrameCurrent &&
          !admission.clearStaleRuntimeFrame && admission.sourceGraphicId == 9);
    CHECK(dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
              &state, &actionPaint, &actionHost, &admission));
    CHECK(admission.accepted && admission.clearStaleRuntimeFrame &&
          admission.sourceGraphicId == 10 && admission.sourceZoneId == 11);
    CHECK(dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
              &state, &actionPaint, &actionHost, &admission));
    CHECK(admission.accepted && admission.alreadyCurrent && !admission.clearStaleRuntimeFrame);

    CHECK(!dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
              &state, &spellPaint, &spellHost, &admission));
    actionPaint.sourceZoneId = 13;
    CHECK(!dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
              &state, &actionPaint, &actionHost, &admission));
    actionPaint.sourceZoneId = 11;
    actionPaint.retirePreviousHostConsumption = 0;
    CHECK(!dm1_v1_action_spell_runtime_frame_admission_apply_pc34(
              &state, &actionPaint, &actionHost, &admission));

    printf("%s\n", failures ? "failed" : "ok: action/spell runtime frame admission");
    return failures ? 1 : 0;
}
