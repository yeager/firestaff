#ifndef DM1_V1_HOC_CONFIGURED_FINAL_CAPTURE_LIFECYCLE_PC34_COMPAT_H
#define DM1_V1_HOC_CONFIGURED_FINAL_CAPTURE_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_hoc_live_final_runtime_capture_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Configuration originates in the loaded original dungeon/graphics data. */
typedef struct {
    int sourceOwned;
    uint16_t c127MirrorOrdinal;
    uint32_t c026SourceHash;
    uint32_t c040SourceHash;
} DM1_V1_HocConfiguredOriginalEvidencePc34;

typedef struct {
    const DM1_V1_HocLiveFinalRuntimeCaptureReceiptPc34 *currentCapture;
    const DM1_V1_HocConfiguredOriginalEvidencePc34 *configuration;
    const struct DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *prior;
    uint64_t runtimeTick;
} DM1_V1_HocConfiguredFinalCaptureLifecycleInputPc34;

typedef struct DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 {
    int valid;
    int sourceOwned;
    int active;
    int publishCurrentCapture;
    int captureC127Proof;
    int captureC040Panel;
    int captureC026Portrait;
    int clearRevoke;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    uint64_t captureTick;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    const char *sourceAnchor;
} DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34;

int DM1_V1_HocConfiguredFinalCaptureLifecycle_BuildReceiptPc34(
    const DM1_V1_HocConfiguredFinalCaptureLifecycleInputPc34 *input,
    DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *outReceipt);

const char *DM1_V1_HocConfiguredFinalCaptureLifecycle_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
