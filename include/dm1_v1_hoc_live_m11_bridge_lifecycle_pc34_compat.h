#ifndef DM1_V1_HOC_LIVE_M11_BRIDGE_LIFECYCLE_PC34_COMPAT_H
#define DM1_V1_HOC_LIVE_M11_BRIDGE_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_hoc_live_material_runtime_m11_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 *currentBridge;
    const struct DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *prior;
    uint64_t runtimeTick;
} DM1_V1_HocLiveM11BridgeLifecycleInputPc34;

typedef struct DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 {
    int valid;
    int sourceOwned;
    int active;
    int currentC127Proof;
    int publishCurrent;
    int clearC040;
    int drawC026Portrait;
    int clearC026Portrait;
    int clearRevoke;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    uint64_t admittedTick;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    const char *sourceAnchor;
} DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34;

int DM1_V1_HocLiveM11BridgeLifecycle_BuildReceiptPc34(
    const DM1_V1_HocLiveM11BridgeLifecycleInputPc34 *input,
    DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *outReceipt);

const char *DM1_V1_HocLiveM11BridgeLifecycle_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
