#include "dm1_v1_action_spell_runtime_frame_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34
admission(unsigned int tick, int action)
{
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 value;
    memset(&value, 0, sizeof(value));
    value.accepted = 1; value.runtimeFrameCurrent = 1;
    value.clearStaleRuntimeFrame = tick > 900; value.suppressSyntheticFallback = 1;
    value.originalRouteKind = action
        ? DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34
        : DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34;
    value.sourceGraphicId = action ? 10 : 9; value.sourceZoneId = action ? 11 : 13;
    value.renderRect = action
        ? (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 }
        : (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    value.frameTick = tick; value.sourceTick = tick - 800;
    value.serial = tick - 891; value.commandFingerprint = tick + 0x10u;
    value.orderingFingerprint = tick + 0x20u; value.lifecycleGeneration = tick - 888;
    return value;
}

int
main(void)
{
    DM1_V1_ActionSpellRuntimeFrameLifecycleStatePc34 state;
    DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 lifecycle;
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 spell = admission(900, 0);
    DM1_V1_ActionSpellRuntimeFrameAdmissionReceiptPc34 action = admission(901, 1);

    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(&state, &spell, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.hostOutputCurrent &&
          !lifecycle.clearStaleHostOutput && !lifecycle.revokeStaleHostOutput);
    CHECK(dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(&state, &action, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.clearStaleHostOutput &&
          lifecycle.revokeStaleHostOutput && lifecycle.staleSourceGraphicId == 9 &&
          lifecycle.staleSourceZoneId == 13 && lifecycle.staleClearRect.y == 42);
    CHECK(dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(&state, &action, &lifecycle));
    CHECK(lifecycle.accepted && lifecycle.alreadyCurrent &&
          !lifecycle.clearStaleHostOutput && !lifecycle.revokeStaleHostOutput);

    CHECK(!dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(&state, &spell, &lifecycle));
    action.sourceZoneId = 13;
    CHECK(!dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(&state, &action, &lifecycle));
    action.sourceZoneId = 11;
    action.suppressSyntheticFallback = 0;
    CHECK(!dm1_v1_action_spell_runtime_frame_lifecycle_apply_pc34(&state, &action, &lifecycle));

    printf("%s\n", failures ? "failed" : "ok: action/spell runtime frame lifecycle");
    return failures ? 1 : 0;
}
