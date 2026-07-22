#ifndef DM1_V1_HOC_LIVE_MATERIAL_RUNTIME_FRAME_PC34_COMPAT_H
#define DM1_V1_HOC_LIVE_MATERIAL_RUNTIME_FRAME_PC34_COMPAT_H

#include "dm1_v1_hoc_live_candidate_mirror_material_route_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 *currentRoute;
    const struct DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *prior;
    uint64_t runtimeTick;
} DM1_V1_HocLiveMaterialRuntimeFrameInputPc34;

typedef struct DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 {
    int valid;
    int sourceOwned;
    int active;
    int publishCurrent;
    int clearC040;
    int drawC026Portrait;
    int clearC026Portrait;
    int revokeStale;
    int consumedC127;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    uint64_t runtimeTick;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    const char *sourceAnchor;
} DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34;

int DM1_V1_HocLiveMaterialRuntimeFrame_BuildReceiptPc34(
    const DM1_V1_HocLiveMaterialRuntimeFrameInputPc34 *input,
    DM1_V1_HocLiveMaterialRuntimeFrameReceiptPc34 *outReceipt);

const char *DM1_V1_HocLiveMaterialRuntimeFrame_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
