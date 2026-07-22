#ifndef DM1_V1_HOC_CANDIDATE_CONFIRMATION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_CONFIRMATION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_selection_state_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source state after F0280 selection and before F0282 command handling. */
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
} DM1_V1_HocCandidateConfirmationStatePc34;

typedef struct {
    const DM1_V1_HocCandidateSelectionStateReceiptPc34 *selection;
    DM1_V1_HocCandidateConfirmationStatePc34 state;
    int16_t command;
    int renameAccepted;
} DM1_V1_HocCandidateConfirmationInputPc34;

/* Pre-apply receipt for C160/C161/C162.  The next generation values are the
 * exact values that the later source-owned apply plan must consume. */
typedef struct {
    int valid;
    int sourceOwned;
    int commitReady;
    int awaitsRename;
    int clearsCandidatePanel;
    int disablesMirrorSensor;
    int restoreC127Portrait;
    int requiresFirstMirrorSensorOwner;
    uint16_t mirrorOrdinal;
    uint16_t nextPartyChampionCount;
    uint16_t nextCandidateChampionOrdinal;
    uint32_t expectedSensorGeneration;
    uint32_t expectedPanelGeneration;
    HocMirrorCandidateFinalizeReceipt_Compat finalization;
    const char *sourceAnchor;
} DM1_V1_HocCandidateConfirmationReceiptPc34;

int DM1_V1_HocCandidateConfirmation_BuildReceiptPc34(
    const DM1_V1_HocCandidateConfirmationInputPc34 *input,
    DM1_V1_HocCandidateConfirmationReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateConfirmation_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
