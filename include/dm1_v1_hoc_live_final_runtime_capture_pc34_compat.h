#ifndef DM1_V1_HOC_LIVE_FINAL_RUNTIME_CAPTURE_PC34_COMPAT_H
#define DM1_V1_HOC_LIVE_FINAL_RUNTIME_CAPTURE_PC34_COMPAT_H

#include "dm1_v1_hoc_live_m11_bridge_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocLiveM11BridgeLifecycleReceiptPc34 *lifecycle;
    uint64_t runtimeTick;
} DM1_V1_HocLiveFinalRuntimeCaptureInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int captureCurrentFrame;
    int captureC127Proof;
    int captureC040Panel;
    int captureC026Portrait;
    int captureClearRevoke;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    uint64_t runtimeTick;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    const char *sourceAnchor;
} DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34;

int DM1_V1_HocLiveFinalRuntimeCapture_BuildReceiptPc34(
    const DM1_V1_HocLiveFinalRuntimeCaptureInputPc34 *input,
    DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 *outReceipt);

const char *DM1_V1_HocLiveFinalRuntimeCapture_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
