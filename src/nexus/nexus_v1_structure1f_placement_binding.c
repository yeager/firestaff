#include "nexus_v1_structure1f_placement_binding.h"
#include <string.h>

int nexus_v1_structure1f_placement_binding_admit(
    const Nexus_V1_Structure1FPlacementBindingInput *input,
    Nexus_V1_Structure1FPlacementBindingReceipt *out_receipt)
{
    const Nexus_V1_DgnStructure2DescriptorCaptureTarget *descriptor;
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *placement;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->no_draw_only = 1;
    out_receipt->blocks_real_dgn_mesh_render = 1;
    if (!input || !(placement = input->placement) || !input->target ||
        !placement->valid || !placement->dgn_placement_observed ||
        !placement->frame_sequence || !placement->command_sequence ||
        !placement->no_draw_only || !placement->blocks_real_dgn_mesh_render ||
        placement->fallback_visuals_permitted ||
        !placement->prs3_header_span_fnv1a64 ||
        !placement->prs3_bitmap_candidate_fnv1a64 ||
        !placement->prs3_bitmap_candidate_size ||
        !placement->palt_candidate_fnv1a64 ||
        placement->palt_candidate_size != 512U || !input->target->valid) {
        return 0;
    }

    descriptor = &input->target->static_material.descriptor_target;
    if (placement->dgn_fnv1a64 !=
            input->target->static_material.source_bytes_fnv1a64 ||
        placement->descriptor_index != input->descriptor_index ||
        descriptor->descriptor_index != (int)input->descriptor_index ||
        descriptor->descriptor_bytes_fnv1a64 != input->descriptor_fnv1a64 ||
        descriptor->image_payload_anchor_offset != input->image_offset ||
        descriptor->image_payload_candidate_fnv1a64 != input->image_fnv1a64 ||
        descriptor->palette_payload_anchor_offset != input->palette_offset ||
        descriptor->palette_payload_candidate_fnv1a64 != input->palette_fnv1a64 ||
        !input->target->direct_face_material_bound ||
        !input->target->capture_producer_required ||
        !input->target->original_saturn_capture_required ||
        !input->target->no_draw_only ||
        !input->target->blocks_real_dgn_mesh_render ||
        input->target->fallback_visuals_permitted) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->placement_observed = 1;
    out_receipt->dgn_fnv1a64 = placement->dgn_fnv1a64;
    out_receipt->descriptor_index = placement->descriptor_index;
    out_receipt->frame_sequence = placement->frame_sequence;
    out_receipt->command_sequence = placement->command_sequence;
    out_receipt->descriptor_fnv1a64 = descriptor->descriptor_bytes_fnv1a64;
    out_receipt->image_anchor_offset =
        descriptor->image_payload_anchor_offset;
    out_receipt->image_candidate_fnv1a64 =
        descriptor->image_payload_candidate_fnv1a64;
    out_receipt->palette_anchor_offset =
        descriptor->palette_payload_anchor_offset;
    out_receipt->palette_candidate_fnv1a64 =
        descriptor->palette_payload_candidate_fnv1a64;
    return 1;
}
