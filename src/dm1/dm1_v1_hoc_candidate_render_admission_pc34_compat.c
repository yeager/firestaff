#include "dm1_v1_hoc_candidate_render_admission_pc34_compat.h"

#include <string.h>

static void append_command(DM1_V1_HocCandidateRenderAdmissionReceiptPc34 *receipt,
                           DM1_V1_HocCandidateRenderCommandKindPc34 kind,
                           int graphicIndex)
{
    DM1_V1_HocCandidateRenderCommandPc34 *command;

    command = &receipt->commands[receipt->commandCount++];
    memset(command, 0, sizeof(*command));
    command->kind = kind;
    command->sourceOwned = 1;
    command->graphicIndex = graphicIndex;
    command->left = DM1_V1_HOC_C026_CLICK_LEFT_PC34;
    command->right = DM1_V1_HOC_C026_CLICK_RIGHT_PC34;
    command->top = DM1_V1_HOC_C026_CLICK_TOP_PC34;
    command->bottom = DM1_V1_HOC_C026_CLICK_BOTTOM_PC34;
}

int DM1_V1_HocCandidateRenderAdmission_BuildReceiptPc34(
    const DM1_V1_HocCandidateRenderAdmissionInputPc34 *input,
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 receipt;
    const DM1_V1_HocCandidateCompletionReceiptPc34 *completion;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0282; PANEL.C F0352; DUNVIEW.C:3913-3928 C026";
    completion = input->completion;
    if (!completion || !completion->valid || !completion->sourceOwned ||
        !completion->atomic || !completion->publishC040Cleared ||
        !completion->publishCandidateCleared || !input->mirrorSourceBound ||
        input->mirrorOrdinal != completion->mirrorOrdinal ||
        input->candidateChampionOrdinal != 0u || input->c040PanelOpen ||
        input->sensorGeneration != completion->sensorGeneration ||
        input->presentedPanelGeneration != completion->presentedPanelGeneration ||
        !input->c040PanelGraphicAvailable) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.mirrorOrdinal = completion->mirrorOrdinal;
    receipt.sensorGeneration = completion->sensorGeneration;
    receipt.presentedPanelGeneration = completion->presentedPanelGeneration;
    append_command(&receipt, DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34, 40);

    if (completion->clearStaleC026Portrait) {
        if (completion->publishMirrorSensorEnabled || completion->publishC026Portrait ||
            input->c127SensorEnabled || input->c026PortraitVisible) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.sourceAnchor =
                "ReDMCSB F0282 C160/C161 C026 retirement admission";
            *outReceipt = receipt;
            return 1;
        }
        append_command(&receipt, DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34, 26);
        *outReceipt = receipt;
        return 1;
    }

    if (!completion->publishMirrorSensorEnabled || !completion->publishC026Portrait ||
        !input->c127SensorEnabled || !input->c026PortraitVisible ||
        !input->c026PortraitAtlasAvailable) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.sourceAnchor =
            "ReDMCSB F0282 C162 C127/C026 restore admission";
        *outReceipt = receipt;
        return 1;
    }
    append_command(&receipt, DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34, 26);
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateRenderAdmission_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0282 selects C160/C161 sensor retirement or "
           "C162 restore; PANEL.C F0352 clears C040; DUNVIEW.C:3913-3928 "
           "is the only C026 portrait presentation path.";
}
