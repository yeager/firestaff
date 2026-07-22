#ifndef DM1_V1_HOC_LIVE_CANDIDATE_MIRROR_MATERIAL_ROUTE_PC34_COMPAT_H
#define DM1_V1_HOC_LIVE_CANDIDATE_MIRROR_MATERIAL_ROUTE_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_click_presentation_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Decoded source-page evidence supplied by the DM1 graphics loader.  A zero
 * hash or sourceOwned==0 deliberately means that no live visual is admitted. */
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
} DM1_V1_HocLiveMaterialEvidencePc34;

/* C127 is a dungeon sensor record, not a generated visual. */
typedef struct {
    int sourceOwned;
    int sensorType;
    int sensorData;
    int sensorCell;
    int visibleWallCell;
    uint32_t sensorGeneration;
} DM1_V1_HocLiveC127EvidencePc34;

typedef struct {
    const DM1_V1_ChampionMirrorRuntimeRenderDecisionPc34 *mirrorDecision;
    const DM1_V1_HocCandidateClickPresentationReceiptPc34 *candidatePresentation;
    DM1_V1_HocLiveC127EvidencePc34 c127;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
} DM1_V1_HocLiveCandidateMirrorMaterialRouteInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int consumedC127;
    int admitC026Portrait;
    int admitC040Panel;
    int suppressFallbackVisuals;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionOrdinal;
    uint32_t sensorGeneration;
    uint32_t panelGeneration;
    DM1_V1_HocLiveMaterialEvidencePc34 c026;
    DM1_V1_HocLiveMaterialEvidencePc34 c040;
    const char *sourceAnchor;
} DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34;

int DM1_V1_HocLiveCandidateMirrorMaterialRoute_BuildReceiptPc34(
    const DM1_V1_HocLiveCandidateMirrorMaterialRouteInputPc34 *input,
    DM1_V1_HocLiveCandidateMirrorMaterialRouteReceiptPc34 *outReceipt);

const char *DM1_V1_HocLiveCandidateMirrorMaterialRoute_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
