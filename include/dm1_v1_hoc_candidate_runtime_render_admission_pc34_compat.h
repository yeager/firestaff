#ifndef DM1_V1_HOC_CANDIDATE_RUNTIME_RENDER_ADMISSION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_RUNTIME_RENDER_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_lifecycle_frame_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidateLifecycleFrameBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *priorClearFrame;
} DM1_V1_HocCandidateRuntimeRenderAdmissionInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int admitHostRoute;
    int commandCount;
    DM1_V1_HocCandidateFrameCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t renderTick;
    const char *sourceAnchor;
} DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34;

int DM1_V1_HocCandidateRuntimeRenderAdmission_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeRenderAdmissionInputPc34 *input,
    DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateRuntimeRenderAdmission_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
