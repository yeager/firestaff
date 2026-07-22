#ifndef DM1_V1_HOC_CANDIDATE_FRAME_COMMAND_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_FRAME_COMMAND_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_render_admission_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A decoded original graphics page/rectangle. `sourceHash` is supplied by
 * the graphics decoder; zero means no source material may be presented. */
typedef struct {
    int sourceOwned;
    int graphicIndex;
    int sourceX;
    int sourceY;
    int width;
    int height;
    int dstLeft;
    int dstTop;
    uint32_t sourceHash;
} DM1_V1_HocCandidateFrameMaterialPc34;

typedef struct {
    const DM1_V1_HocCandidateCompletionReceiptPc34 *completion;
    const DM1_V1_HocCandidateRenderAdmissionReceiptPc34 *admission;
    DM1_V1_HocCandidateFrameMaterialPc34 c026;
    DM1_V1_HocCandidateFrameMaterialPc34 c040;
} DM1_V1_HocCandidateFrameCommandInputPc34;

typedef struct {
    DM1_V1_HocCandidateRenderCommandKindPc34 kind;
    DM1_V1_HocCandidateFrameMaterialPc34 material;
} DM1_V1_HocCandidateFrameCommandPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int commandCount;
    DM1_V1_HocCandidateFrameCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidateFrameCommandReceiptPc34;

int DM1_V1_HocCandidateFrameCommand_BuildReceiptPc34(
    const DM1_V1_HocCandidateFrameCommandInputPc34 *input,
    DM1_V1_HocCandidateFrameCommandReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateFrameCommand_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
