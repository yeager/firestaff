#include "dm1_v1_hoc_candidate_frame_command_pc34_compat.h"

#include <string.h>

static int material_valid(const DM1_V1_HocCandidateFrameMaterialPc34 *material,
                          int graphicIndex)
{
    return material && material->sourceOwned && material->graphicIndex == graphicIndex &&
           material->width > 0 && material->height > 0 && material->sourceHash != 0u;
}

static int c026_material_valid(const DM1_V1_HocCandidateFrameMaterialPc34 *material)
{
    return material_valid(material, 26) && material->width == 32 &&
           material->height == 29 &&
           material->dstLeft == DM1_V1_HOC_C026_CLICK_LEFT_PC34 &&
           material->dstTop == DM1_V1_HOC_C026_CLICK_TOP_PC34;
}

int DM1_V1_HocCandidateFrameCommand_BuildReceiptPc34(
    const DM1_V1_HocCandidateFrameCommandInputPc34 *input,
    DM1_V1_HocCandidateFrameCommandReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateFrameCommandReceiptPc34 receipt;
    const DM1_V1_HocCandidateCompletionReceiptPc34 *completion;
    const DM1_V1_HocCandidateRenderAdmissionReceiptPc34 *admission;
    DM1_V1_HocCandidateRenderCommandKindPc34 finalKind;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0352 C040 clear; DUNVIEW.C:3913-3928 C026 "
        "source material and D1C destination rectangle";
    completion = input->completion;
    admission = input->admission;
    if (!completion || !admission || !completion->valid || !completion->sourceOwned ||
        !completion->atomic || !admission->valid || !admission->sourceOwned ||
        admission->commandCount != 2 ||
        admission->commands[0].kind != DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 ||
        admission->mirrorOrdinal != completion->mirrorOrdinal ||
        admission->sensorGeneration != completion->sensorGeneration ||
        admission->presentedPanelGeneration != completion->presentedPanelGeneration ||
        !material_valid(&input->c040, 40) || !c026_material_valid(&input->c026)) {
        *outReceipt = receipt;
        return 1;
    }

    finalKind = admission->commands[1].kind;
    if (finalKind == DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34) {
        if (!completion->clearStaleC026Portrait || completion->publishC026Portrait) {
            *outReceipt = receipt;
            return 1;
        }
    } else if (finalKind == DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34) {
        if (!completion->publishC026Portrait || completion->clearStaleC026Portrait) {
            *outReceipt = receipt;
            return 1;
        }
    } else {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.commandCount = 2;
    receipt.commands[0].kind = DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34;
    receipt.commands[0].material = input->c040;
    receipt.commands[1].kind = finalKind;
    receipt.commands[1].material = input->c026;
    receipt.mirrorOrdinal = completion->mirrorOrdinal;
    receipt.sensorGeneration = completion->sensorGeneration;
    receipt.presentedPanelGeneration = completion->presentedPanelGeneration;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateFrameCommand_SourceEvidencePc34(void)
{
    return "ReDMCSB PANEL.C F0352 clears the original C040 panel rectangle; "
           "DUNVIEW.C:3913-3928 consumes the original C026 material at "
           "G0109 {96,127,35,63}; stale completion state may not emit either.";
}
