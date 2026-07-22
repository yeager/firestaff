#ifndef DM1_V1_HOC_CANDIDATE_APPLY_PRESENTATION_COMPLETION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_APPLY_PRESENTATION_COMPLETION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_confirmation_apply_bridge_pc34_compat.h"
#include "dm1_v1_hoc_candidate_presentation_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t partyChampionCount;
    uint16_t candidateChampionOrdinal;
    uint16_t mirrorOrdinal;
    int mirrorSourceBound;
    int c040PanelOpen;
    int c127SensorEnabled;
    int c026PortraitVisible;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
} DM1_V1_HocCandidateCompletionStatePc34;

typedef struct {
    const DM1_V1_HocCandidateConfirmationApplyBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidateApplyReceiptPc34 *apply;
    const DM1_V1_HocCandidatePresentationReceiptPc34 *presentation;
    DM1_V1_HocCandidateCompletionStatePc34 state;
} DM1_V1_HocCandidateCompletionInputPc34;

/* Final publication boundary.  This is intentionally a receipt, not a host
 * renderer: it admits only exact source state from bridge -> apply -> panel
 * presentation and cannot manufacture a C026/C040 fallback. */
typedef struct {
    int valid;
    int sourceOwned;
    int atomic;
    int publishC040Cleared;
    int publishCandidateCleared;
    int publishMirrorSensorEnabled;
    int publishC026Portrait;
    int clearStaleC026Portrait;
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidateCompletionReceiptPc34;

int DM1_V1_HocCandidateCompletion_BuildReceiptPc34(
    const DM1_V1_HocCandidateCompletionInputPc34 *input,
    DM1_V1_HocCandidateCompletionReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateCompletion_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
