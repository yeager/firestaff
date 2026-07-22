#include "dm1_v1_hoc_candidate_panel_receipt_pc34_compat.h"

#include <string.h>

static int receipt_matches_active_mirror(
    const HocMirrorCandidateRuntimeReceipt_Compat *receipt,
    const DM1_V1_HocCandidatePanelReceiptInputPc34 *input)
{
    return receipt && receipt->valid && receipt->sourceOwned &&
           input->activeMirrorSourceBound &&
           receipt->mirrorOrdinal == input->activeMirrorOrdinal &&
           input->c127SensorOrdinal == input->activeMirrorOrdinal;
}

int DM1_V1_HocCandidatePanel_BuildReceiptPc34(
    const DM1_V1_HocCandidatePanelReceiptInputPc34 *input,
    DM1_V1_HocCandidatePanelReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidatePanelReceiptPc34 receipt;
    const HocMirrorCandidateRuntimeReceipt_Compat *active;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    active = input->activeReceipt;
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0280/F0282; PANEL.C F0344-F0347/F0352; "
        "DUNGEON.C F0172/F0174; DUNVIEW.C:3913-3928";

    if (input->activePanelGeneration == 0u ||
        input->presentedPanelGeneration > input->activePanelGeneration ||
        !receipt_matches_active_mirror(active, input)) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.mirrorOrdinal = input->activeMirrorOrdinal;
    receipt.candidateChampionOrdinal = active->nextCandidateChampionOrdinal;
    receipt.nextPanelGeneration = input->activePanelGeneration;

    if (input->transition == DM1_V1_HOC_CANDIDATE_PANEL_REDRAW_PC34) {
        if (!active->keepsCandidatePanelOpen ||
            !input->c026PortraitAtlasAvailable ||
            !input->c040PanelGraphicAvailable ||
            !input->c127SensorEnabled) {
            *outReceipt = receipt;
            return 1;
        }
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.redrawC026Portrait = 1;
        receipt.redrawC040Panel = 1;
        receipt.keepsSensorEnabled = 1;
        *outReceipt = receipt;
        return 1;
    }

    if (input->transition == DM1_V1_HOC_CANDIDATE_PANEL_CLOSE_PC34) {
        if (!active->clearsCandidatePanel ||
            active->nextCandidateChampionOrdinal != 0u) {
            *outReceipt = receipt;
            return 1;
        }
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.clearC040Panel = 1;
        receipt.clearStalePanelPayload = 1;
        receipt.nextPanelGeneration = input->activePanelGeneration + 1u;
        receipt.disablesSensor = active->disablesMirrorSensor ? 1 : 0;
        receipt.keepsSensorEnabled = active->disablesMirrorSensor ? 0 : 1;
        if (active->restoreC127MirrorPortrait) {
            if (!input->c127SensorEnabled || !input->c026PortraitAtlasAvailable) {
                memset(&receipt, 0, sizeof(receipt));
                receipt.sourceAnchor =
                    "ReDMCSB REVIVE.C F0282 C162 requires live C127/C026";
                *outReceipt = receipt;
                return 1;
            }
            receipt.restoreC127Portrait = 1;
            receipt.redrawC026Portrait = 1;
        }
        *outReceipt = receipt;
        return 1;
    }

    if (input->transition == DM1_V1_HOC_CANDIDATE_PANEL_REOPEN_PC34) {
        const HocMirrorCandidateRuntimeReceipt_Compat *reopen =
            input->reopenReceipt;
        if (!reopen || !reopen->valid || !reopen->sourceOwned ||
            !reopen->opensCandidatePanel || !reopen->keepsCandidatePanelOpen ||
            reopen->mirrorOrdinal != input->activeMirrorOrdinal ||
            !input->c127SensorEnabled || !input->c026PortraitAtlasAvailable ||
            !input->c040PanelGraphicAvailable ||
            input->presentedPanelGeneration != input->activePanelGeneration) {
            *outReceipt = receipt;
            return 1;
        }
        receipt.valid = 1;
        receipt.sourceOwned = 1;
        receipt.opensCandidatePanel = 1;
        receipt.redrawC026Portrait = 1;
        receipt.redrawC040Panel = 1;
        receipt.keepsSensorEnabled = 1;
        receipt.candidateChampionOrdinal =
            reopen->nextCandidateChampionOrdinal;
        receipt.nextPanelGeneration = input->activePanelGeneration + 1u;
        *outReceipt = receipt;
        return 1;
    }

    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidatePanel_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0280:124-132/272-276 opens G0299; "
           "F0282:744-806 clears C040 and disables the first mirror sensor "
           "only on C160/C161; PANEL.C F0344-F0347/F0352 redraws C040; "
           "DUNGEON.C F0172/F0174 and DUNVIEW.C:3913-3928 restore C127/C026 "
           "only while the source sensor remains enabled.";
}
