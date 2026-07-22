#include "dm1_v1_hoc_candidate_runtime_lifecycle_host_bridge_pc34_compat.h"

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

static DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 lifecycle(void)
{
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1;
    result.consumeC040 = 1; result.consumeC026 = 1;
    result.mirrorOrdinal = 3; result.sensorGeneration = 17;
    result.presentedPanelGeneration = 29; result.consumedTick = 102;
    result.c040SourceHash = 0x4040u; result.c026SourceHash = 0x2626u;
    return result;
}

static DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 admission(void)
{
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1; result.admitHostRoute = 1;
    result.commandCount = 2; result.mirrorOrdinal = 3;
    result.sensorGeneration = 17; result.presentedPanelGeneration = 29;
    result.renderTick = 102;
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
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 active = lifecycle();
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 current = admission();
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeInputPc34 input;
    DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.lifecycle = &active; input.admission = &current; input.hostTick = 102;
    ok &= expect(DM1_V1_HocCandidateRuntimeLifecycleHostBridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.admitActiveHostState && receipt.commandCount == 2 &&
                 receipt.commands[0].material.graphicIndex == 40 &&
                 receipt.commands[1].material.graphicIndex == 26,
                 "active source lifecycle publishes original C040/C026 state");

    input.hostTick = 103;
    ok &= expect(DM1_V1_HocCandidateRuntimeLifecycleHostBridge_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "later host tick cannot consume stale lifecycle state");

    if (!ok) return 1;
    puts("ok: DM1 HoC lifecycle host bridge admits only active source state");
    return 0;
}
