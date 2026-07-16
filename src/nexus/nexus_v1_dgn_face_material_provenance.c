#include "nexus_v1_dgn_face_material_provenance.h"

#include <string.h>

static void set_status(Nexus_V1_DgnFaceMaterialReceipt *receipt,
                       Nexus_V1_DgnFaceMaterialStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->no_draw_only = 1;
    receipt->blocks_real_dgn_mesh_render = 1;
}

int nexus_v1_dgn_face_material_validate(
    const Nexus_V1_DgnFaceMaterialInput *input,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt)
{
    uint8_t seen[NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES];
    int i;

    if (!out_receipt) return 0;
    set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_INPUT);
    if (!input || !input->dgn_bytes || input->dgn_size <= 0 ||
        !input->canonical_dgn_bytes || input->canonical_dgn_size <= 0 ||
        !input->bindings || input->face_count <= 0 ||
        input->face_count > NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES ||
        input->structure2_descriptor_count <= 0 ||
        input->material_selector_count <= 0) {
        return 0;
    }
    if (input->source != NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN ||
        !input->canonical_source_verified) {
        set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
        return 0;
    }
    if (input->dgn_size != input->canonical_dgn_size ||
        memcmp(input->dgn_bytes, input->canonical_dgn_bytes,
               (size_t)input->dgn_size) != 0) {
        set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
        return 0;
    }

    memset(seen, 0, sizeof(seen));
    out_receipt->canonical_source_verified = 1;
    out_receipt->reopened_bytes_match_canonical = 1;
    out_receipt->canonical_dgn_size = input->canonical_dgn_size;
    for (i = 0; i < input->face_count; ++i) {
        const Nexus_V1_DgnFaceMaterialBinding *binding = &input->bindings[i];
        if (binding->face_ordinal >= (uint16_t)input->face_count ||
            seen[binding->face_ordinal] ||
            binding->material_selector >= input->material_selector_count ||
            (binding->selector_kind ==
                 NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC &&
             binding->material_selector >=
                 input->structure2_descriptor_count) ||
            (binding->selector_kind != NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC &&
             binding->selector_kind != NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED)) {
            set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING);
            return 0;
        }
        seen[binding->face_ordinal] = 1;
        if (binding->selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC)
            ++out_receipt->static_selector_count;
        else
            ++out_receipt->animated_selector_count;
    }

    out_receipt->status = NEXUS_V1_DGN_FACE_MATERIAL_READY;
    out_receipt->face_count = input->face_count;
    out_receipt->structure2_descriptor_count =
        input->structure2_descriptor_count;
    out_receipt->structure3_mesh_materials_bound = 1;
    out_receipt->structure2_descriptor_route_bound = 1;
    out_receipt->selector_bindings_complete = 1;
    out_receipt->material_semantics_proven = 0;
    out_receipt->package_host_route_bound = 1;
    out_receipt->original_saturn_capture_required = 1;
    out_receipt->original_saturn_capture_available = 0;
    out_receipt->can_submit_raster_input = 0;
    return 1;
}

const char *nexus_v1_dgn_face_material_status_name(
    Nexus_V1_DgnFaceMaterialStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_FACE_MATERIAL_READY: return "ready";
    case NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE: return "blocked-source";
    case NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_INPUT: return "blocked-input";
    case NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING: return "blocked-binding";
    }
    return "blocked-input";
}
