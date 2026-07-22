#ifndef DM1_V1_HOC_CANDIDATE_RENDER_ADMISSION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_RENDER_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_apply_presentation_completion_pc34_compat.h"
#include "dm1_v1_hoc_candidate_click_presentation_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C040_PC34 = 1,
    DM1_V1_HOC_CANDIDATE_RENDER_CLEAR_C026_PC34 = 2,
    DM1_V1_HOC_CANDIDATE_RENDER_DRAW_C026_PC34 = 3
} DM1_V1_HocCandidateRenderCommandKindPc34;

typedef struct {
    DM1_V1_HocCandidateRenderCommandKindPc34 kind;
    int sourceOwned;
    int graphicIndex;
    int left;
    int top;
    int right;
    int bottom;
} DM1_V1_HocCandidateRenderCommandPc34;

typedef struct {
    const DM1_V1_HocCandidateCompletionReceiptPc34 *completion;
    uint16_t mirrorOrdinal;
    int mirrorSourceBound;
    uint16_t candidateChampionOrdinal;
    int c040PanelOpen;
    int c127SensorEnabled;
    int c026PortraitVisible;
    int c026PortraitAtlasAvailable;
    int c040PanelGraphicAvailable;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
} DM1_V1_HocCandidateRenderAdmissionInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int commandCount;
    DM1_V1_HocCandidateRenderCommandPc34 commands[2];
    uint16_t mirrorOrdinal;
    uint32_t sensorGeneration;
    uint32_t presentedPanelGeneration;
    const char *sourceAnchor;
} DM1_V1_HocCandidateRenderAdmissionReceiptPc34;

int DM1_V1_HocCandidateRenderAdmission_BuildReceiptPc34(
    const DM1_V1_HocCandidateRenderAdmissionInputPc34 *input,
    DM1_V1_HocCandidateRenderAdmissionReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateRenderAdmission_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
