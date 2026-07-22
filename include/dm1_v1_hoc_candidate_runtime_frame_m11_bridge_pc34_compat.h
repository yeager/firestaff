#ifndef DM1_V1_HOC_CANDIDATE_RUNTIME_FRAME_M11_BRIDGE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_RUNTIME_FRAME_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_runtime_frame_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidateRuntimeFrameLifecycleReceiptPc34 *lifecycle;
    uint64_t runtimeTick;
} DM1_V1_HocCandidateRuntimeFrameM11BridgeInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int admitToM11;
    int publishCurrent;
    int clearRevoke;
    int commandCount;
    DM1_V1_HocCandidateFrameCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t runtimeTick;
    uint32_t c040SourceHash;
    uint32_t c026SourceHash;
    const char *sourceAnchor;
} DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34;

int DM1_V1_HocCandidateRuntimeFrameM11Bridge_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeFrameM11BridgeInputPc34 *input,
    DM1_V1_HocCandidateRuntimeFrameM11BridgeReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateRuntimeFrameM11Bridge_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
