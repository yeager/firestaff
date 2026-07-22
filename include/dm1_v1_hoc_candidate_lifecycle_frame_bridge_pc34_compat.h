#ifndef DM1_V1_HOC_CANDIDATE_LIFECYCLE_FRAME_BRIDGE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_LIFECYCLE_FRAME_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_frame_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidateFrameLifecycleReceiptPc34 *lifecycle;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *priorClearFrame;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *portraitFrame;
} DM1_V1_HocCandidateLifecycleFrameBridgeInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int publishC026Portrait;
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t clearTick;
    uint64_t portraitTick;
    uint32_t c040SourceHash;
    DM1_V1_HocCandidateFrameCommandPc34 portraitCommand;
    const char *sourceAnchor;
} DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34;

int DM1_V1_HocCandidateLifecycleFrameBridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateLifecycleFrameBridgeInputPc34 *input,
    DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateLifecycleFrameBridge_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
