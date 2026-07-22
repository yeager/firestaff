#include "dm1_v1_hoc_candidate_runtime_frame_admission_pc34_compat.h"

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
    commands[0].material.sourceOwned = 1; commands[0].material.graphicIndex = 40;
    commands[0].material.width = 144; commands[0].material.height = 73;
    commands[0].material.sourceHash = 0x4040u;
    commands[1].kind = DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34;
    commands[1].material.sourceOwned = 1; commands[1].material.graphicIndex = 26;
    commands[1].material.width = 32; commands[1].material.height = 29;
    commands[1].material.dstLeft = 96; commands[1].material.dstTop = 35;
    commands[1].material.sourceHash = 0x2626u;
}

int main(void)
{
    DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 bridge;
    DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 render;
    DM1_V1_HocCandidateRuntimeFrameAdmissionInputPc34 input;
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 receipt;
    int ok = 1;

    memset(&bridge, 0, sizeof(bridge));
    memset(&render, 0, sizeof(render));
    bridge.valid = 1; bridge.sourceOwned = 1; bridge.admitToM11 = 1;
    bridge.mirrorOrdinal = 3; bridge.sensorGeneration = 17;
    bridge.presentedPanelGeneration = 29; bridge.hostTick = 102;
    bridge.c040SourceHash = 0x4040u; bridge.c026SourceHash = 0x2626u;
    render.valid = 1; render.sourceOwned = 1; render.renderAtM11Boundary = 1;
    render.commandCount = 2; render.mirrorOrdinal = 3; render.sensorGeneration = 17;
    render.presentedPanelGeneration = 29; render.hostTick = 102;
    render.c040SourceHash = 0x4040u; render.c026SourceHash = 0x2626u;
    source_commands(render.commands);
    memset(&input, 0, sizeof(input));
    input.preM11Bridge = &bridge; input.hostRender = &render; input.runtimeTick = 102;

    ok &= expect(DM1_V1_HocCandidateRuntimeFrameAdmission_BuildReceiptPc34(&input, &receipt) &&
                 receipt.valid && receipt.admitRuntimeFrame && receipt.commandCount == 2 &&
                 receipt.commands[0].material.width == 144 &&
                 receipt.commands[1].material.dstTop == 35 &&
                 receipt.c040SourceHash == 0x4040u && receipt.c026SourceHash == 0x2626u,
                 "matching bridge/render proof admits exact source runtime frame");

    render.sensorGeneration++;
    ok &= expect(DM1_V1_HocCandidateRuntimeFrameAdmission_BuildReceiptPc34(&input, &receipt) &&
                 !receipt.valid,
                 "generation mismatch cannot admit runtime frame");

    if (!ok) return 1;
    puts("ok: DM1 HoC runtime frame admission combines exact C040/C026 proof");
    return 0;
}
