#ifndef NEXUS_V1_PRS3_DGN_PLACEMENT_ADAPTER_H
#define NEXUS_V1_PRS3_DGN_PLACEMENT_ADAPTER_H

#include "nexus_v1_engine.h"
#include "nexus_v1_prs3_vdp1_consumer_evidence.h"

typedef struct {
    const Nexus_V1_DgnActiveLevelRendererSourceReceipt *dgn;
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *prs3;
    const Nexus_V1_Prs3Vdp1CaptureReceipt *capture;
    uint64_t trace_fnv1a64;
    uint32_t trace_size;
    uint64_t traced_dgn_fnv1a64;
    uint32_t descriptor_index;
    uint64_t descriptor_fnv1a64;
    int descriptor_envelope_valid;
    int16_t traced_xa, traced_ya, traced_xb, traced_yb;
    int16_t traced_xc, traced_yc, traced_xd, traced_yd;
} Nexus_V1_Prs3DgnPlacementAdapterInput;

struct Nexus_V1_Prs3DgnPlacementAdapterReceipt {
    int valid;
    int trace_bound;
    int dgn_source_bound;
    int descriptor_envelope_bound;
    int command_coordinates_bound;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    uint64_t trace_fnv1a64;
    uint32_t trace_size;
    uint64_t dgn_fnv1a64;
    uint32_t descriptor_index;
    uint64_t descriptor_fnv1a64;
    uint64_t frame_sequence;
    uint64_t command_sequence;
    int dgn_placement_observed;
    uint32_t prs3_entry_index;
    uint64_t prs3_header_span_fnv1a64;
    uint64_t prs3_bitmap_candidate_fnv1a64;
    uint32_t prs3_bitmap_candidate_offset;
    uint32_t prs3_bitmap_candidate_size;
    uint64_t palt_candidate_fnv1a64;
    uint32_t palt_candidate_size;
};

int nexus_v1_prs3_dgn_placement_adapter_admit(
    const Nexus_V1_Prs3DgnPlacementAdapterInput *input,
    Nexus_V1_Prs3DgnPlacementAdapterReceipt *out_receipt);

#endif
