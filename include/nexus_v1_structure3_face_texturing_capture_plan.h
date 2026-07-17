#ifndef NEXUS_V1_STRUCTURE3_FACE_TEXTURING_CAPTURE_PLAN_H
#define NEXUS_V1_STRUCTURE3_FACE_TEXTURING_CAPTURE_PLAN_H

#include "nexus_v1_engine.h"
#include "nexus_v1_structure1f_corpus_capture_plan.h"
#include "nexus_v1_prs3_dgn_placement_adapter.h"

typedef struct {
    const Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget *structure1f;
    const Nexus_V1_Structure1FCorpusTraceTarget *corpus_target;
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *placement;
    const Nexus_V1_Prs3Vdp1CaptureReceipt *vdp1_trace;
} Nexus_V1_Structure3FaceTexturingCapturePlanInput;

typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t dgn_fnv1a64;
    uint32_t descriptor_index;
    uint64_t descriptor_fnv1a64;
    uint32_t mesh_index;
    uint64_t mesh_fnv1a64;
    uint32_t face_index;
    uint64_t face_fnv1a64;
    uint64_t bitmap_candidate_fnv1a64;
    uint64_t palette_candidate_fnv1a64;
    uint64_t frame_sequence;
    uint64_t command_sequence;
    int original_saturn_capture_required;
    int capture_only;
    int pixel_semantics_permitted;
    int geometry_semantics_permitted;
    int draw_permitted;
} Nexus_V1_Structure3FaceTexturingCaptureTarget;

int nexus_v1_structure3_face_texturing_capture_plan_build(
    const Nexus_V1_Structure3FaceTexturingCapturePlanInput *input,
    Nexus_V1_Structure3FaceTexturingCaptureTarget *out_target);

#endif
