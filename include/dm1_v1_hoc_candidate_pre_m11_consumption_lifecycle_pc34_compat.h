#ifndef DM1_V1_HOC_CANDIDATE_PRE_M11_CONSUMPTION_LIFECYCLE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_PRE_M11_CONSUMPTION_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_host_bridge_consumption_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocCandidateHostBridgeConsumptionReceiptPc34 *consumption;
    const DM1_V1_HocCandidateRuntimeLifecycleHostBridgeReceiptPc34 *activeHostRoute;
    const struct DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 *prior;
    uint64_t hostTick;
} DM1_V1_HocCandidatePreM11ConsumptionLifecycleInputPc34;

typedef struct DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 {
    int valid;
    int sourceOwned;
    int consumeBeforeM11;
    int activeHostRouteProven;
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t consumedTick;
    uint32_t c040SourceHash;
    uint32_t c026SourceHash;
    const char *sourceAnchor;
} DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34;

int DM1_V1_HocCandidatePreM11ConsumptionLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidatePreM11ConsumptionLifecycleInputPc34 *input,
    DM1_V1_HocCandidatePreM11ConsumptionLifecycleReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidatePreM11ConsumptionLifecycle_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
