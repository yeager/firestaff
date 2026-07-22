#ifndef DM1_V1_HOC_CANDIDATE_HANDOFF_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_HANDOFF_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_panel_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB REVIVE.C F0282 is the sole C160/C161/C162 finalizer.  This
 * companion receipt consumes the source-owned F0873 transition plus the
 * C040 close receipt, retaining the C127 owner until the close is complete. */
typedef struct {
    const HocMirrorCandidateRuntimeReceipt_Compat *transition;
    const DM1_V1_HocCandidatePanelReceiptPc34 *panelClose;
    uint16_t activeMirrorOrdinal;
    int activeMirrorSourceBound;
    uint16_t c127SensorOrdinal;
    int c127SensorEnabled;
    int c026PortraitAtlasAvailable;
} DM1_V1_HocCandidateHandoffInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int candidateStateCleared;
    int c040PanelRetired;
    int clearStalePanelPayload;
    int disablesMirrorSensor;
    int keepsMirrorSensorEnabled;
    int clearStalePortrait;
    int restoreC127Portrait;
    int drawC026Portrait;
    int reopenEligible;
    uint16_t mirrorOrdinal;
    uint16_t nextPartyChampionCount;
    uint16_t nextCandidateChampionOrdinal;
    const char *sourceAnchor;
} DM1_V1_HocCandidateHandoffReceiptPc34;

/* Builds the post-C160/C161/C162 state handoff.  A C161 waiting for rename
 * is intentionally not a close handoff.  C162 retains portrait ownership
 * only if the matching C127 sensor remains enabled and C026 is available. */
int DM1_V1_HocCandidateHandoff_BuildReceiptPc34(
    const DM1_V1_HocCandidateHandoffInputPc34 *input,
    DM1_V1_HocCandidateHandoffReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateHandoff_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
