#ifndef DM1_V1_HOC_CANDIDATE_HOST_BRIDGE_CONSUMPTION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_HOST_BRIDGE_CONSUMPTION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_runtime_lifecycle_host_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *hostBridge;
    uint64_t hostTick;
} DM1_V1_HocCandidateHostBridgeConsumptionInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int consumeBeforeM11;
    int commandCount;
    DM1_V1_HocCandidateFrameCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t hostTick;
    const char *sourceAnchor;
} DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34;

int DM1_V1_HocCandidateHostBridgeConsumption_BuildReceiptPc34(
    const DM1_V1_HocCandidateHostBridgeConsumptionInputPc34 *input,
    DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateHostBridgeConsumption_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
