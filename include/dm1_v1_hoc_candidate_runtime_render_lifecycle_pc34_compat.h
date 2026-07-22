#ifndef DM1_V1_HOC_CANDIDATE_RUNTIME_RENDER_LIFECYCLE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_RUNTIME_RENDER_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_runtime_render_admission_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Host consumption is fenced by the source admission tick.  The host may
 * never replay C040/C026 material from an already consumed render tick. */
typedef struct {
    const DM1_V1_HocCandidateRuntimeRenderAdmissionReceiptPc34 *admission;
    const struct DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 *prior;
    uint64_t hostTick;
} DM1_V1_HocCandidateRuntimeRenderLifecycleInputPc34;

typedef struct DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 {
    int valid;
    int sourceOwned;
    int consumeC040;
    int consumeC026;
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t consumedTick;
    uint32_t c040SourceHash;
    uint32_t c026SourceHash;
    const char *sourceAnchor;
} DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34;

int DM1_V1_HocCandidateRuntimeRenderLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidateRuntimeRenderLifecycleInputPc34 *input,
    DM1_V1_HocCandidateRuntimeRenderLifecycleReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateRuntimeRenderLifecycle_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
