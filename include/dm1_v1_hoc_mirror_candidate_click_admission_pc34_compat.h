#ifndef DM1_V1_HOC_MIRROR_CANDIDATE_CLICK_ADMISSION_PC34_COMPAT_H
#define DM1_V1_HOC_MIRROR_CANDIDATE_CLICK_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_click_presentation_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sourceOwned;
    int graphicIndex;
    int sourceX;
    int sourceY;
    int width;
    int height;
    int dstX;
    int dstY;
    uint32_t sourceHash;
} DM1_V1_HocMirrorCandidateSourceMaterialPc34;

typedef struct {
    DM1_V1_ChampionMirrorRuntimeRenderInputPc34 mirrorRuntime;
    DM1_V1_HocCandidateClickPresentationInputPc34 clickPresentation;
    DM1_V1_HocMirrorCandidateSourceMaterialPc34 c026;
    DM1_V1_HocMirrorCandidateSourceMaterialPc34 c040;
} DM1_V1_HocMirrorCandidateClickAdmissionInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int admitCandidateClick;
    int consumedC127;
    int admitC026Portrait;
    int admitC040Panel;
    int suppressFallbackVisuals;
    uint16_t mirrorOrdinal;
    uint32_t panelGeneration;
    DM1_V1_HocMirrorCandidateSourceMaterialPc34 c026;
    DM1_V1_HocMirrorCandidateSourceMaterialPc34 c040;
    DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 mirrorDecision;
    DM1_V1_HocCandidateClickPresentationReceiptPc34 clickReceipt;
    const char *sourceAnchor;
} DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34;

int DM1_V1_HocMirrorCandidateClickAdmission_BuildReceiptPc34(
    const DM1_V1_HocMirrorCandidateClickAdmissionInputPc34 *input,
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 *outReceipt);

/* Runtime M11 path: consume the already source-owned C127/C346/C026
 * decision and reject selection unless its actual C026/C040 surfaces match
 * the same presentation receipt. */
int DM1_V1_HocMirrorCandidateClickAdmission_BuildFromRuntimeDecisionPc34(
    const DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *mirrorDecision,
    const DM1_V1_HocCandidateClickPresentationReceiptPc34 *clickReceipt,
    const DM1_V1_HocMirrorCandidateSourceMaterialPc34 *c026,
    const DM1_V1_HocMirrorCandidateSourceMaterialPc34 *c040,
    DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 *outReceipt);

const char *DM1_V1_HocMirrorCandidateClickAdmission_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
