#ifndef DM1_V1_HOC_CONFIGURED_CAPTURE_M11_GATE_PC34_COMPAT_H
#define DM1_V1_HOC_CONFIGURED_CAPTURE_M11_GATE_PC34_COMPAT_H

#include "dm1_v1_hoc_configured_final_capture_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const DM1_V1_HocConfiguredFinalCaptureLifecycleReceiptPc34 *lifecycle;
    const DM1_V1_HocConfiguredOriginalEvidencePc34 *configuration;
    uint64_t runtimeTick;
} DM1_V1_HocConfiguredCaptureM11GateInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int admitCaptureToM11;
    int captureCurrentC127;
    int captureC040Panel;
    int captureC026Portrait;
    int clearRevoke;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    uint64_t runtimeTick;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    const char *sourceAnchor;
} DM1_V1_HocConfiguredCaptureM11GateReceiptPc34;

int DM1_V1_HocConfiguredCaptureM11Gate_BuildReceiptPc34(
    const DM1_V1_HocConfiguredCaptureM11GateInputPc34 *input,
    DM1_V1_HocConfiguredCaptureM11GateReceiptPc34 *outReceipt);

const char *DM1_V1_HocConfiguredCaptureM11Gate_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
