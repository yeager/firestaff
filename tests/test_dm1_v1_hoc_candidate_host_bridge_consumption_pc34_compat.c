#include "dm1_v1_hoc_candidate_host_bridge_consumption_pc34_compat.h"

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

static DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 active_bridge(void)
{
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.admitActiveHostState = 1;
    result.commandCount = 2; result.mirrorOrdinal = 3;
    result.sensorGeneration = 17; result.presentedPanelGeneration = 29;
    result.hostTick = 102;
    result.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    result.commands[0].material.sourceOwned = 1;
    result.commands[0].material.graphicIndex = 40;
    result.commands[0].material.sourceHash = 0x4040u;
    result.commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    result.commands[1].material.sourceOwned = 1;
    result.commands[1].material.graphicIndex = 26;
    result.commands[1].material.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 bridge = active_bridge();
    DM1_V1_HocCandidateHostBridgeConsumptionInputPc34 input;
    DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.hostBridge = &bridge; input.hostTick = 102;
    ok &= expect(DM1_V1_HocCandidateHostBridgeConsumption_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.consumeBeforeM11 && receipt.commandCount == 2 &&
                 receipt.commands[0].material.graphicIndex == 40 &&
                 receipt.commands[1].material.graphicIndex == 26,
                 "active host route publishes original C040/C026 before M11");

    input.hostTick = 103;
    ok &= expect(DM1_V1_HocCandidateHostBridgeConsumption_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "stale host route cannot publish before M11");

    if (!ok) return 1;
    puts("ok: DM1 HoC host bridge consumption fences pre-M11 route");
    return 0;
}
