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

static void set_consumer_status(
    Nexus_V1_DgnPackageHostConsumerReceipt *receipt,
    Nexus_V1_DgnPackageHostConsumerStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->original_saturn_capture_required = 1;
    receipt->blocks_real_dgn_mesh_render = 1;
    receipt->no_draw_only = 1;
}

int nexus_v1_dgn_package_host_consumer_gate(
    const Nexus_V1_DgnPackageHostConsumerInput *input,
    Nexus_V1_DgnPackageHostConsumerReceipt *out_receipt)
{
    const Nexus_V1_DgnFaceMaterialReceipt *material;

    if (!out_receipt) return 0;
    set_consumer_status(out_receipt,
                        NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_INVALID);
    if (!input || !input->material_receipt) {
        return 0;
    }

    material = input->material_receipt;
    out_receipt->host_route_requested =
        input->host_route_requested ? 1 : 0;
    out_receipt->package_route_consumed =
        input->package_route_consumed ? 1 : 0;
    out_receipt->level_index_matches =
        input->expected_level_index >= 0 &&
        input->expected_level_index == input->observed_level_index;
    out_receipt->canonical_dgn_size_matches =
        input->expected_canonical_dgn_size > 0 &&
        input->expected_canonical_dgn_size ==
            input->observed_canonical_dgn_size &&
        input->observed_canonical_dgn_size == material->canonical_dgn_size;
    out_receipt->face_count_matches =
        input->expected_face_count > 0 &&
        input->expected_face_count == input->observed_face_count &&
        input->observed_face_count == material->face_count;
    out_receipt->structure2_descriptor_count_matches =
        input->expected_structure2_descriptor_count > 0 &&
        input->expected_structure2_descriptor_count ==
            input->observed_structure2_descriptor_count &&
        input->observed_structure2_descriptor_count ==
            material->structure2_descriptor_count;

    out_receipt->material_receipt_ready =
        material->status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
        material->canonical_source_verified &&
        material->reopened_bytes_match_canonical &&
        material->structure3_mesh_materials_bound &&
        material->structure2_descriptor_route_bound &&
        material->selector_bindings_complete &&
        material->package_host_route_bound &&
        material->no_draw_only &&
        material->blocks_real_dgn_mesh_render &&
        !material->material_semantics_proven &&
        !material->original_saturn_capture_available &&
        !material->can_submit_raster_input &&
        !material->permits_fallback_visuals;

    if (!out_receipt->material_receipt_ready) {
        out_receipt->status =
            NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL;
        return 0;
    }

    if (!out_receipt->host_route_requested ||
        !out_receipt->package_route_consumed ||
        !out_receipt->level_index_matches ||
        !out_receipt->canonical_dgn_size_matches ||
        !out_receipt->face_count_matches ||
        !out_receipt->structure2_descriptor_count_matches) {
        out_receipt->status =
            NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_ROUTE;
        return 0;
    }

    out_receipt->status =
        NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW;
    out_receipt->source_route_consumed_by_host = 1;
    out_receipt->package_host_route_bound = 1;
    out_receipt->original_saturn_rendering_proven = 0;
    out_receipt->material_semantics_proven = 0;
    out_receipt->can_submit_raster_input = 0;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->blocks_real_dgn_mesh_render = 1;
    out_receipt->no_draw_only = 1;
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

const char *nexus_v1_dgn_package_host_consumer_status_name(
    Nexus_V1_DgnPackageHostConsumerStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_INVALID: return "invalid";
    case NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL:
        return "blocked-material";
    case NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_ROUTE:
        return "blocked-route";
    case NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW:
        return "ready-no-draw";
    }
    return "invalid";
}
