#ifndef DM1_V1_HOC_CANDIDATE_SELECTION_STATE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_SELECTION_STATE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_click_presentation_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* State immediately before a source-owned C127 candidate selection commits. */
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
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
} DM1_V1_HocCandidateSelectionStatePc34;

typedef struct {
    const DM1_V1_HocCandidateClickPresentationReceiptPc34 *presentation;
    DM1_V1_HocCandidateSelectionStatePc34 state;
} DM1_V1_HocCandidateSelectionStateInputPc34;

/* Atomic post-click state publication.  It intentionally does not mutate the
 * sensor generation: only the later C160/C161 finalizer may retire C127. */
typedef struct {
    int valid;
    int sourceOwned;
    int atomic;
    int publishCandidateChampionOrdinal;
    uint16_t nextCandidateChampionOrdinal;
    int publishPartyChampionCount;
    uint16_t nextPartyChampionCount;
    int publishActiveMirror;
    uint16_t nextActiveMirrorOrdinal;
    int nextActiveMirrorSourceBound;
    int openC040Panel;
    int drawC026Portrait;
    int drawC040Panel;
    int keepsC127SensorEnabled;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidateSelectionStateReceiptPc34;

int DM1_V1_HocCandidateSelectionState_BuildReceiptPc34(
    const DM1_V1_HocCandidateSelectionStateInputPc34 *input,
    DM1_V1_HocCandidateSelectionStateReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateSelectionState_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
