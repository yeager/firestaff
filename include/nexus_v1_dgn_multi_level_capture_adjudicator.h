#ifndef NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_ADJUDICATOR_H
#define NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_ADJUDICATOR_H

#include "nexus_v1_structure1f_corpus_capture_plan.h"
#include "nexus_v1_structure3_face_texturing_capture_plan.h"

#define NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_LEVEL_COUNT 16U
typedef struct {
    const Nexus_V1_Structure1FCorpusTraceTarget *structure1f;
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *structure2;
    const Nexus_V1_Structure3FaceTexturingCaptureTarget *structure3;
} Nexus_V1_DgnMultiLevelCaptureEvidence;
typedef struct { Nexus_V1_DgnMultiLevelCaptureEvidence levels[NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_LEVEL_COUNT]; } Nexus_V1_DgnMultiLevelCaptureAdjudicationInput;
typedef struct {
    int valid; uint32_t level_index; uint64_t dgn_fnv1a64; uint32_t descriptor_index;
    uint64_t trace_fnv1a64; uint32_t trace_size; uint64_t frame_sequence; uint64_t command_sequence; int opaque_original_capture_covered;
} Nexus_V1_DgnMultiLevelCaptureCoverage;
typedef struct Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt {
    int valid; Nexus_V1_DgnMultiLevelCaptureCoverage levels[NEXUS_V1_DGN_MULTI_LEVEL_CAPTURE_LEVEL_COUNT];
    int opaque_original_capture_only; int decoder_promoted; int mesh_semantics_permitted; int render_permitted;
} Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt;
int nexus_v1_dgn_multi_level_capture_adjudicate(const Nexus_V1_DgnMultiLevelCaptureAdjudicationInput *, Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt *);
#endif
