#ifndef DM1_V1_HOC_CANDIDATE_RUNTIME_FRAME_ADMISSION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_RUNTIME_FRAME_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_pre_m11_host_render_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *preM11Bridge;
    const DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 *hostRender;
    uint64_t runtimeTick;
} DM1_V1_HocCandidateRuntimeFrameAdmissionInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int admitRuntimeFrame;
    int commandCount;
    DM1_V1_HocCandidateFrameCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t runtimeTick;
    uint32_t c040SourceHash;
    uint32_t c026SourceHash;
    const char *sourceAnchor;
} DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34;

int DM1_V1_HocCandidateRuntimeFrameAdmission_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeFrameAdmissionInputPc34 *input,
    DM1_V1_HocCandidateRuntimeFrameAdmissionReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateRuntimeFrameAdmission_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
