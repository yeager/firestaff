#include "dm1_v1_action_spell_runtime_host_m11_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { ++failures; fprintf(stderr, "FAIL: %s\n", #c); } } while (0)

static void
set_runtime(DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 *runtime, int action)
{
    memset(runtime, 0, sizeof(*runtime));
    runtime->accepted = 1; runtime->hostOutputCurrent = 1;
    runtime->suppressSyntheticFallback = 1; runtime->frameTick = 901;
    runtime->sourceTick = 101; runtime->serial = 10;
    runtime->commandFingerprint = 0x41u; runtime->orderingFingerprint = 0x42u;
    runtime->lifecycleGeneration = 13;
    runtime->originalRouteKind = action
        ? DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_ACTION_PC34
        : DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34;
    runtime->sourceGraphicId = action ? 10 : 9;
    runtime->sourceZoneId = action ? 11 : 13;
    runtime->renderRect = action
        ? (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 77, 87, 45 }
        : (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
}

int
main(void)
{
    DM1_V1_ActionSpellRuntimeFrameLifecycleReceiptPc34 runtime;
    DM1_V1_ActionSpellRuntimeHostM11BridgeReceiptPc34 bridge;

    set_runtime(&runtime, 1);
    runtime.clearStaleHostOutput = 1; runtime.revokeStaleHostOutput = 1;
    runtime.staleOriginalRouteKind = DM1_V1_ACTION_SPELL_M11_ORIGINAL_ROUTE_SPELL_PC34;
    runtime.staleSourceGraphicId = 9; runtime.staleSourceZoneId = 13;
    runtime.staleClearRect = (DM1_V1_ActionSpellHudPaintRectPc34){ 233, 42, 87, 25 };
    CHECK(dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(&runtime, &bridge));
    CHECK(bridge.accepted && bridge.m11HostOutputReady &&
          bridge.originalGraphicId == 10 && bridge.originalZoneId == 11 &&
          bridge.clearStaleHostOutput && bridge.revokeStaleHostOutput &&
          bridge.staleOriginalGraphicId == 9 && bridge.staleClearRect.y == 42);

    runtime.clearStaleHostOutput = 0; runtime.revokeStaleHostOutput = 0;
    runtime.staleOriginalRouteKind = 0; runtime.staleSourceGraphicId = 0;
    runtime.staleSourceZoneId = 0; memset(&runtime.staleClearRect, 0, sizeof(runtime.staleClearRect));
    CHECK(dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(&runtime, &bridge));
    CHECK(!bridge.clearStaleHostOutput && !bridge.revokeStaleHostOutput);

    runtime.revokeStaleHostOutput = 1;
    CHECK(!dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(&runtime, &bridge));
    runtime.revokeStaleHostOutput = 0;
    runtime.sourceZoneId = 13;
    CHECK(!dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(&runtime, &bridge));
    runtime.sourceZoneId = 11;
    runtime.hostOutputCurrent = 0;
    CHECK(!dm1_v1_action_spell_runtime_host_m11_bridge_build_pc34(&runtime, &bridge));

    printf("%s\n", failures ? "failed" : "ok: action/spell runtime host M11 bridge");
    return failures ? 1 : 0;
}
