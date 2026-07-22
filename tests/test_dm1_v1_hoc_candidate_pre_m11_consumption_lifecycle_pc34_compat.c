#include "dm1_v1_hoc_candidate_pre_m11_consumption_lifecycle_pc34_compat.h"

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
    DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 consumption;
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 route;
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleInputPc34 input;
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 receipt;
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 previous;
    int ok = 1;

    memset(&consumption, 0, sizeof(consumption));
    memset(&route, 0, sizeof(route));
    consumption.valid = 1; consumption.sourceOwned = 1; consumption.consumeBeforeM11 = 1;
    consumption.commandCount = 2; consumption.mirrorOrdinal = 3;
    consumption.sensorGeneration = 17; consumption.presentedPanelGeneration = 29;
    consumption.hostTick = 102; source_commands(consumption.commands);
    route.valid = 1; route.sourceOwned = 1; route.admitActiveHostState = 1;
    route.commandCount = 2; route.mirrorOrdinal = 3;
    route.sensorGeneration = 17; route.presentedPanelGeneration = 29;
    route.hostTick = 102; source_commands(route.commands);
    memset(&input, 0, sizeof(input));
    input.consumption = &consumption; input.activeHostRoute = &route; input.hostTick = 102;

    ok &= expect(DM1_V1_HocCandidatePreM11ConsumptionLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.activeHostRouteProven && receipt.consumedTick == 102 &&
                 receipt.c040SourceHash == 0x4040u && receipt.c026SourceHash == 0x2626u,
                 "active host route proves source pre-M11 consumption");

    previous = receipt;
    input.prior = &previous;
    ok &= expect(DM1_V1_HocCandidatePreM11ConsumptionLifecycle_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "same tick cannot replay pre-M11 consumption");

    if (!ok) return 1;
    puts("ok: DM1 HoC pre-M11 lifecycle fences stale host consumption");
    return 0;
}
