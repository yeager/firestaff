#ifndef DM1_V1_HOC_CANDIDATE_CONFIRMATION_APPLY_BRIDGE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_CONFIRMATION_APPLY_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_confirmation_pc34_compat.h"
#include "dm1_v1_hoc_candidate_apply_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Live pre-apply state.  This is intentionally narrower than M11 state and
 * contains only the ReDMCSB owners required by F0282. */
typedef struct {
    uint16_t partyChampionCount;
    uint16_t candidateChampionOrdinal;
    uint16_t activeMirrorOrdinal;
    int activeMirrorSourceBound;
    int c040PanelOpen;
    int c127SensorEnabled;
    uint16_t c127SensorOrdinal;
    int c026PortraitAtlasAvailable;
    int c040PanelGraphicAvailable;
    int firstMirrorSensorOwnerAvailable;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
} DM1_V1_HocCandidateConfirmationApplyBridgeStatePc34;

typedef struct {
    const DM1_V1_HocCandidateConfirmationReceiptPc34 *confirmation;
    DM1_V1_HocCandidateConfirmationApplyBridgeStatePc34 state;
} DM1_V1_HocCandidateConfirmationApplyBridgeInputPc34;

/* The bridge materializes the exact handoff and pre-apply snapshot for
 * DM1_V1_HocCandidateApply_BuildReceiptPc34.  Both are valid together or
 * neither is exported. */
typedef struct {
    int valid;
    int sourceOwned;
    int atomic;
    DM1_V1_HocCandidateHandoffReceiptPc34 handoff;
    DM1_V1_HocCandidateApplyStatePc34 applyState;
    const char *sourceAnchor;
} DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34;

int DM1_V1_HocCandidateConfirmationApplyBridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateConfirmationApplyBridgeInputPc34 *input,
    DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateConfirmationApplyBridge_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
