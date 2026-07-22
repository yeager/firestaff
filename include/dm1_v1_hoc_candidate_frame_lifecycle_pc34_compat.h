#ifndef DM1_V1_HOC_CANDIDATE_FRAME_LIFECYCLE_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_FRAME_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_frame_command_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A portrait restore is valid only after its source-owned clear frame has
 * reached the renderer.  Ticks fence asynchronous host-frame consumption. */
typedef struct {
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *priorClearFrame;
    const DM1_V1_HocCandidateFrameCommandReceiptPc34 *nextPortraitFrame;
    uint64_t priorClearTick;
    uint64_t nextPortraitTick;
} DM1_V1_HocCandidateFrameLifecycleInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int clearAccepted;
    int portraitAdmitted;
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    uint64_t clearTick;
    uint64_t portraitTick;
    uint32_t c040SourceHash;
    uint32_t c026SourceHash;
    const char *sourceAnchor;
} DM1_V1_HocCandidateFrameLifecycleReceiptPc34;

int DM1_V1_HocCandidateFrameLifecycle_BuildReceiptPc34(
    const DM1_V1_HocCandidateFrameLifecycleInputPc34 *input,
    DM1_V1_HocCandidateFrameLifecycleReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateFrameLifecycle_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
