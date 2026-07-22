#ifndef DM1_V1_HOC_CANDIDATE_APPLY_RECEIPT_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_APPLY_RECEIPT_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_handoff_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source-state snapshot immediately before the post-F0282 handoff applies.
 * Every field corresponds to state owned by REVIVE.C/PANEL.C/DUNGEON.C. */
typedef struct {
    uint16_t partyChampionCount;
    uint16_t candidateChampionOrdinal;
    uint16_t mirrorOrdinal;
    int mirrorSourceBound;
    int c040PanelOpen;
    int c127SensorEnabled;
    int c026PortraitAtlasAvailable;
    uint32_t presentedPanelGeneration;
    uint32_t closePanelGeneration;
} DM1_V1_HocCandidateApplyStatePc34;

typedef struct {
    const DM1_V1_HocCandidateHandoffReceiptPc34 *handoff;
    DM1_V1_HocCandidateApplyStatePc34 state;
} DM1_V1_HocCandidateApplyInputPc34;

/* An atomic mutation plan for the source state.  Hosts must apply all of the
 * listed writes or none of them; missing C026/C127/C040 material is no-plan. */
typedef struct {
    int valid;
    int sourceOwned;
    int atomic;
    int clearG0299CandidateOrdinal;
    int writePartyChampionCount;
    uint16_t nextPartyChampionCount;
    int closeC040Panel;
    int clearStalePanelPayload;
    int writeMirrorSensorEnabled;
    int nextMirrorSensorEnabled;
    int clearStaleC026Portrait;
    int drawC026Portrait;
    uint16_t mirrorOrdinal;
    uint32_t nextPresentedPanelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidateApplyReceiptPc34;

int DM1_V1_HocCandidateApply_BuildReceiptPc34(
    const DM1_V1_HocCandidateApplyInputPc34 *input,
    DM1_V1_HocCandidateApplyReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateApply_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
