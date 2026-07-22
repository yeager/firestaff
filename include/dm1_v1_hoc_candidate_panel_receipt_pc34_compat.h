#ifndef DM1_V1_HOC_CANDIDATE_PANEL_RECEIPT_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_PANEL_RECEIPT_PC34_COMPAT_H

#include "dm1_v1_resurrection_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB ownership: REVIVE.C F0280/F0282 publishes and clears G0299;
 * PANEL.C F0344-F0347/F0352 redraws C040; DUNGEON.C F0172/F0174 and
 * DUNVIEW.C:3913-3928 restore C127/C026 only while its sensor remains live. */
typedef enum {
    DM1_V1_HOC_CANDIDATE_PANEL_REDRAW_PC34 = 1,
    DM1_V1_HOC_CANDIDATE_PANEL_CLOSE_PC34 = 2,
    DM1_V1_HOC_CANDIDATE_PANEL_REOPEN_PC34 = 3
} DM1_V1_HocCandidatePanelTransitionPc34;

typedef struct {
    DM1_V1_HocCandidatePanelTransitionPc34 transition;
    const HocMirrorCandidateRuntimeReceipt_Compat *activeReceipt;
    const HocMirrorCandidateRuntimeReceipt_Compat *reopenReceipt;
    uint16_t activeMirrorOrdinal;
    int activeMirrorSourceBound;
    uint16_t c127SensorOrdinal;
    int c127SensorEnabled;
    int c026PortraitAtlasAvailable;
    int c040PanelGraphicAvailable;
    uint32_t activePanelGeneration;
    uint32_t presentedPanelGeneration;
} DM1_V1_HocCandidatePanelReceiptInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int redrawC040Panel;
    int redrawC026Portrait;
    int clearC040Panel;
    int clearStalePanelPayload;
    int restoreC127Portrait;
    int opensCandidatePanel;
    int keepsSensorEnabled;
    int disablesSensor;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t nextPanelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidatePanelReceiptPc34;

/* DM1-owned C040 redraw/close/reopen receipt.  It refuses stale generations,
 * wrong C127 identity, unavailable source graphics, and sensor-disabled
 * reopen attempts.  No host or generated panel/portrait is admitted. */
int DM1_V1_HocCandidatePanel_BuildReceiptPc34(
    const DM1_V1_HocCandidatePanelReceiptInputPc34 *input,
    DM1_V1_HocCandidatePanelReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidatePanel_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
