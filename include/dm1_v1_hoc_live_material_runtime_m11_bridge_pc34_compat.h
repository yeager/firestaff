#ifndef DM1_V1_HOC_LIVE_MATERIAL_RUNTIME_M11_BRIDGE_PC34_COMPAT_H
#define DM1_V1_HOC_LIVE_MATERIAL_RUNTIME_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_hoc_live_material_runtime_frame_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *runtimeFrame;
    uint64_t runtimeTick;
} DM1_V1_HocLiveMaterialRuntimeM11BridgeInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int admitToM11;
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
    uint64_t runtimeTick;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    const char *sourceAnchor;
} DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34;

int DM1_V1_HocLiveMaterialRuntimeM11Bridge_BuildReceiptPc34(
    const DM1_V1_HocLiveMaterialRuntimeM11BridgeInputPc34 *input,
    DM1_V1_HocLiveMaterialRuntimeM11BridgeReceiptPc34 *outReceipt);

const char *DM1_V1_HocLiveMaterialRuntimeM11Bridge_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
