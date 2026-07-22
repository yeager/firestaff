#include "dm1_v1_hoc_candidate_pre_m11_lifecycle_bridge_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static void source_commands(DM1_V1_HocCandidateFrameCommandPc34 commands[2])
{
    memset(commands, 0, sizeof(DM1_V1_HocCandidateFrameCommandPc34) * 2u);
    commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    commands[0].material.sourceOwned = 1;
    commands[0].material.graphicIndex = 40;
    commands[0].material.sourceHash = 0x4040u;
    commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    commands[1].material.sourceOwned = 1;
    commands[1].material.graphicIndex = 26;
    commands[1].material.sourceHash = 0x2626u;
}

int main(void)
{
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 lifecycle;
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 route;
    DM1_V1_HocCandidatePreM11LifecycleBridgeInputPc34 input;
    DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 receipt;
    int ok = 1;

    memset(&lifecycle, 0, sizeof(lifecycle));
    memset(&route, 0, sizeof(route));
    lifecycle.valid = 1; lifecycle.sourceOwned = 1; lifecycle.consumeBeforeM11 = 1;
    lifecycle.activeHostRouteProven = 1; lifecycle.mirrorOrdinal = 3;
    lifecycle.sensorGeneration = 17; lifecycle.presentedPanelGeneration = 29;
    lifecycle.consumedTick = 102; lifecycle.c040SourceHash = 0x4040u;
    lifecycle.c026SourceHash = 0x2626u;
    route.valid = 1; route.sourceOwned = 1; route.admitActiveHostState = 1;
    route.commandCount = 2; route.mirrorOrdinal = 3;
    route.sensorGeneration = 17; route.presentedPanelGeneration = 29;
    route.hostTick = 102; source_commands(route.commands);
    memset(&input, 0, sizeof(input));
    input.lifecycle = &lifecycle; input.activeHostRoute = &route; input.hostTick = 102;

    ok &= expect(DM1_V1_HocCandidatePreM11LifecycleBridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.admitToM11 && receipt.hostTick == 102 &&
                 receipt.c040SourceHash == 0x4040u && receipt.c026SourceHash == 0x2626u,
                 "current active host route admits source receipt to M11 boundary");

    route.hostTick = 101;
    ok &= expect(DM1_V1_HocCandidatePreM11LifecycleBridge_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale host route cannot cross M11 boundary");

    if (!ok) return 1;
    puts("ok: DM1 HoC pre-M11 bridge requires active route proof");
    return 0;
}
