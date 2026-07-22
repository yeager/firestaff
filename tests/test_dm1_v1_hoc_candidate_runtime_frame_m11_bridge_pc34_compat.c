#include "dm1_v1_hoc_candidate_runtime_frame_m11_bridge_pc34_compat.h"

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

static DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 lifecycle(int revoke)
{
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 result;
    memset(&result, 0, sizeof(result));
    result.valid = 1; result.sourceOwned = 1;
    result.active = !revoke; result.publishCurrent = !revoke; result.revokeStale = revoke;
    result.commandCount = 2; result.mirrorOrdinal = 3;
    result.sensorGeneration = 17; result.presentedPanelGeneration = 29;
    result.admittedTick = 102; result.c040SourceHash = 0x4040u;
    result.c026SourceHash = 0x2626u;
    result.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    result.commands[0].material.sourceOwned = 1; result.commands[0].material.graphicIndex = 40;
    result.commands[0].material.sourceHash = 0x4040u;
    result.commands[1].kind = revoke ? DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34
                                     : DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    result.commands[1].material.sourceOwned = 1; result.commands[1].material.graphicIndex = 26;
    result.commands[1].material.sourceHash = 0x2626u;
    return result;
}

int main(void)
{
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 current = lifecycle(0);
    DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 stale = lifecycle(1);
    DM1_V1_HocCandidateRuntimeFrameM11BridgeInputPc34 input;
    DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34 receipt;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.lifecycle = &current; input.runtimeTick = 102;
    ok &= expect(DM1_V1_HocCandidateRuntimeFrameM11Bridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.publishCurrent && !receipt.clearRevoke &&
                 receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34,
                 "current lifecycle admits only current C026 portrait");

    input.lifecycle = &stale;
    ok &= expect(DM1_V1_HocCandidateRuntimeFrameM11Bridge_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && !receipt.publishCurrent && receipt.clearRevoke &&
                 receipt.commands[1].kind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34,
                 "stale lifecycle admits only source-owned C026 clear");

    if (!ok) return 1;
    puts("ok: DM1 HoC runtime-to-M11 bridge fences current and stale paths");
    return 0;
}
