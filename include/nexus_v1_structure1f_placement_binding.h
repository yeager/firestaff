#ifndef NEXUS_V1_STRUCTURE1F_PLACEMENT_BINDING_H
#define NEXUS_V1_STRUCTURE1F_PLACEMENT_BINDING_H
#include "nexus_v1_engine.h"
#include "nexus_v1_prs3_dgn_placement_adapter.h"
typedef struct {
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *placement;
    const Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget *target;
    uint32_t descriptor_index;
    uint64_t descriptor_fnv1a64;
    uint32_t image_offset;
    uint64_t image_fnv1a64;
    uint32_t palette_offset;
    uint64_t palette_fnv1a64;
} Nexus_V1_Structure1FPlacementBindingInput;

typedef struct Nexus_V1_Structure1FPlacementBindingReceipt {
    int valid;
    int placement_observed;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    uint64_t dgn_fnv1a64;
    uint32_t descriptor_index;
    uint64_t frame_sequence;
    uint64_t command_sequence;
    uint64_t descriptor_fnv1a64;
    uint32_t image_anchor_offset;
    uint64_t image_candidate_fnv1a64;
    uint32_t palette_anchor_offset;
    uint64_t palette_candidate_fnv1a64;
} Nexus_V1_Structure1FPlacementBindingReceipt;
int nexus_v1_structure1f_placement_binding_admit(const Nexus_V1_Structure1FPlacementBindingInput *, Nexus_V1_Structure1FPlacementBindingReceipt *);
#endif
