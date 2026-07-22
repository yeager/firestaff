#include "dm1_v1_hoc_candidate_handoff_pc34_compat.h"

#include <string.h>

int DM1_V1_HocCandidateHandoff_BuildReceiptPc34(
    const DM1_V1_HocCandidateHandoffInputPc34 *input,
    DM1_V1_HocCandidateHandoffReceiptPc34 *outReceipt)
{
    DM1_V1_HocCandidateHandoffReceiptPc34 receipt;
    const HocMirrorCandidateRuntimeReceipt_Compat *transition;
    const DM1_V1_HocCandidatePanelReceiptPc34 *panel;

    if (!input || !outReceipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB REVIVE.C F0282:744-806; PANEL.C F0352; "
        "DUNGEON.C F0172/F0174; DUNVIEW.C:3913-3928";
    transition = input->transition;
    panel = input->panelClose;

    if (!transition || !panel || !transition->valid ||
        !transition->sourceOwned || transition->awaitsRename ||
        !transition->clearsCandidatePanel ||
        transition->nextCandidateChampionOrdinal != 0u ||
        !panel->valid || !panel->sourceOwned || !panel->clearC040Panel ||
        !panel->clearStalePanelPayload ||
        !input->activeMirrorSourceBound ||
        transition->mirrorOrdinal != input->activeMirrorOrdinal ||
        panel->mirrorOrdinal != input->activeMirrorOrdinal ||
        input->c127SensorOrdinal != input->activeMirrorOrdinal) {
        *outReceipt = receipt;
        return 1;
    }

    receipt.valid = 1;
    receipt.sourceOwned = 1;
    receipt.candidateStateCleared = 1;
    receipt.c040PanelRetired = 1;
    receipt.clearStalePanelPayload = 1;
    receipt.mirrorOrdinal = input->activeMirrorOrdinal;
    receipt.nextPartyChampionCount = transition->nextPartyChampionCount;
    receipt.nextCandidateChampionOrdinal = 0;

    if (transition->disablesMirrorSensor) {
        if (!panel->disablesSensor || panel->keepsSensorEnabled ||
            !input->c127SensorEnabled || panel->restoreC127Portrait) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.sourceAnchor =
                "ReDMCSB REVIVE.C F0282 C160/C161 sensor-disable ownership";
            *outReceipt = receipt;
            return 1;
        }
        receipt.disablesMirrorSensor = 1;
        /* The sensor is retired with the panel.  Clear a prior C026 overlay
         * rather than reusing it after C160/C161. */
        receipt.clearStalePortrait = 1;
        *outReceipt = receipt;
        return 1;
    }

    /* Only C162 reaches this side: it rolls back the provisional champion
     * and restores the exact source portrait, never an inferred replacement. */
    if (!transition->restoreC127MirrorPortrait || !panel->restoreC127Portrait ||
        !panel->keepsSensorEnabled || !input->c127SensorEnabled ||
        !input->c026PortraitAtlasAvailable || !panel->redrawC026Portrait) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.sourceAnchor =
            "ReDMCSB REVIVE.C F0282 C162 requires live C127/C026 ownership";
        *outReceipt = receipt;
        return 1;
    }
    receipt.keepsMirrorSensorEnabled = 1;
    receipt.restoreC127Portrait = 1;
    receipt.drawC026Portrait = 1;
    receipt.reopenEligible = 1;
    *outReceipt = receipt;
    return 1;
}

const char *DM1_V1_HocCandidateHandoff_SourceEvidencePc34(void)
{
    return "ReDMCSB REVIVE.C F0282:744-783 clears G0299 and rolls back "
           "C162; F0282:785-806 clears G0299 and disables the first mirror "
           "sensor for C160/C161; PANEL.C F0352 retires C040; DUNGEON.C "
           "F0172/F0174 plus DUNVIEW.C:3913-3928 may restore C127/C026 "
           "only after C162 leaves that source sensor enabled.";
}
