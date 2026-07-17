#include "nexus_v1_structure1f_placement_binding.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_V1_Prs3DgnPlacementAdapterReceipt placement;
    Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget target;
    Nexus_V1_Structure1FPlacementBindingInput input;
    Nexus_V1_Structure1FPlacementBindingReceipt receipt;

    memset(&placement, 0, sizeof(placement));
    memset(&target, 0, sizeof(target));
    memset(&input, 0, sizeof(input));
    placement.valid = placement.dgn_placement_observed = placement.no_draw_only =
        placement.blocks_real_dgn_mesh_render = 1;
    placement.dgn_fnv1a64 = 1;
    placement.descriptor_index = 2;
    placement.frame_sequence = 0x44;
    placement.command_sequence = 0x45;
    placement.prs3_header_span_fnv1a64 = 8;
    placement.prs3_bitmap_candidate_fnv1a64 = 9;
    placement.prs3_bitmap_candidate_size = 10;
    placement.palt_candidate_fnv1a64 = 11;
    placement.palt_candidate_size = 512;
    target.valid = target.direct_face_material_bound =
        target.capture_producer_required = target.original_saturn_capture_required =
        target.no_draw_only = target.blocks_real_dgn_mesh_render = 1;
    target.static_material.source_bytes_fnv1a64 = 1;
    target.static_material.descriptor_target.descriptor_index = 2;
    target.static_material.descriptor_target.descriptor_bytes_fnv1a64 = 3;
    target.static_material.descriptor_target.image_payload_anchor_offset = 4;
    target.static_material.descriptor_target.image_payload_candidate_fnv1a64 = 5;
    target.static_material.descriptor_target.palette_payload_anchor_offset = 6;
    target.static_material.descriptor_target.palette_payload_candidate_fnv1a64 = 7;
    input.placement = &placement;
    input.target = &target;
    input.descriptor_index = 2;
    input.descriptor_fnv1a64 = 3;
    input.image_offset = 4;
    input.image_fnv1a64 = 5;
    input.palette_offset = 6;
    input.palette_fnv1a64 = 7;

    if (!nexus_v1_structure1f_placement_binding_admit(&input, &receipt) ||
        !receipt.valid || !receipt.placement_observed ||
        receipt.dgn_fnv1a64 != 1 || receipt.descriptor_index != 2 ||
        receipt.frame_sequence != 0x44 || receipt.command_sequence != 0x45 ||
        receipt.descriptor_fnv1a64 != 3 || receipt.image_anchor_offset != 4 ||
        receipt.image_candidate_fnv1a64 != 5 ||
        receipt.palette_anchor_offset != 6 ||
        receipt.palette_candidate_fnv1a64 != 7 ||
        !receipt.no_draw_only || !receipt.blocks_real_dgn_mesh_render ||
        receipt.fallback_visuals_permitted) return 1;

    placement.dgn_placement_observed = 0;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt) ||
        !receipt.no_draw_only || !receipt.blocks_real_dgn_mesh_render ||
        receipt.valid) return 1;
    placement.dgn_placement_observed = 1;
    placement.descriptor_index = 3;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    placement.descriptor_index = 2;
    placement.frame_sequence = 0;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    placement.frame_sequence = 0x44;
    placement.command_sequence = 0;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    placement.command_sequence = 0x45;
    target.original_saturn_capture_required = 0;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    target.original_saturn_capture_required = 1;
    target.direct_face_material_bound = 0;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    target.direct_face_material_bound = 1;
    placement.prs3_bitmap_candidate_fnv1a64 = 0;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    placement.prs3_bitmap_candidate_fnv1a64 = 9;
    placement.dgn_fnv1a64 = 8;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    placement.dgn_fnv1a64 = 1;
    input.descriptor_fnv1a64 = 8;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    input.descriptor_fnv1a64 = 3;
    input.image_fnv1a64 = 8;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;
    input.image_fnv1a64 = 5;
    input.palette_fnv1a64 = 8;
    if (nexus_v1_structure1f_placement_binding_admit(&input, &receipt)) return 1;

    puts("structure1f placement binding: PASS");
    return 0;
}
