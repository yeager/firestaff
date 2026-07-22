#ifndef DM1_V1_HOC_CANDIDATE_PRESENTATION_RECEIPT_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_PRESENTATION_RECEIPT_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_apply_receipt_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* State after an atomic F0282 apply plan has been committed. */
typedef struct {
    uint16_t partyChampionCount;
    uint16_t candidateChampionOrdinal;
    uint16_t mirrorOrdinal;
    int mirrorSourceBound;
    int c040PanelOpen;
    int c127SensorEnabled;
    int c026PortraitAtlasAvailable;
    int c040PanelGraphicAvailable;
    uint32_t previousSensorGeneration;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
} DM1_V1_HocCandidatePresentationStatePc34;

typedef struct {
    const DM1_V1_HocCandidateApplyReceiptPc34 *apply;
    DM1_V1_HocCandidatePresentationStatePc34 state;
} DM1_V1_HocCandidatePresentationInputPc34;

/* Final visual state following the atomic mutation.  The receipt contains
 * only source C026/C040 actions and never provides substitute artwork. */
typedef struct {
    int valid;
    int sourceOwned;
    int clearC040Panel;
    int clearStalePanelPayload;
    int clearStaleC026Portrait;
    int drawC026Portrait;
    int sensorEnabled;
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidatePresentationReceiptPc34;

int DM1_V1_HocCandidatePresentation_BuildReceiptPc34(
    const DM1_V1_HocCandidatePresentationInputPc34 *input,
    DM1_V1_HocCandidatePresentationReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidatePresentation_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
