#ifndef DM1_V1_HOC_CANDIDATE_PRE_M11_HOST_RENDER_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_PRE_M11_HOST_RENDER_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_pre_m11_lifecycle_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidatePreM11LifecycleBridgeReceiptPc34 *bridge;
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *activeHostRoute;
    uint64_t hostTick;
} DM1_V1_HocCandidatePreM11HostRenderInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int renderAtM11Boundary;
    int commandCount;
    DM1_V1_HocCandidateFrameCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t hostTick;
    uint32_t c040SourceHash;
    uint32_t c026SourceHash;
    const char *sourceAnchor;
} DM1_V1_HocCandidatePreM11HostRenderReceiptPc34;

int DM1_V1_HocCandidatePreM11HostRender_BuildReceiptPc34(
    const DM1_V1_HocCandidatePreM11HostRenderInputPc34 *input,
    DM1_V1_HocCandidatePreM11HostRenderReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidatePreM11HostRender_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
