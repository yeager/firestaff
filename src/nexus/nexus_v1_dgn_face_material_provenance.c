#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_prs3_capture_trace_schema.h"

#include <string.h>

static void set_status(Nexus_V1_DgnFaceMaterialReceipt *receipt,
                       Nexus_V1_DgnFaceMaterialStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->no_draw_only = 0;
    receipt->blocks_real_dgn_mesh_render = 0;
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
        input->material_selector_count <= 0 ||
        input->geometry_material_face_count <= 0) {
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
    out_receipt->geometry_source_bound =
        input->geometry_source_bound ? 1 : 0;
    out_receipt->geometry_material_face_count =
        input->geometry_material_face_count;
    out_receipt->geometry_material_face_count_matches =
        input->geometry_material_face_count == input->face_count;
    out_receipt->geometry_can_submit_geometry =
        input->geometry_can_submit_geometry ? 1 : 0;
    out_receipt->geometry_textured_raster_blocked =
        input->geometry_can_submit_textured_raster ? 0 : 1;
    if (!out_receipt->geometry_source_bound ||
        !out_receipt->geometry_material_face_count_matches ||
        !out_receipt->geometry_can_submit_geometry ||
        input->geometry_can_submit_textured_raster ||
        input->geometry_fallback_visuals_permitted) {
        set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
        return 0;
    }
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
    receipt->original_saturn_capture_required = 0;
    receipt->blocks_real_dgn_mesh_render = 0;
    receipt->no_draw_only = 0;
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
    out_receipt->synthetic_material_route_rejected =
        input->synthetic_material_route_requested ? 1 : 0;
    out_receipt->material_pixel_promotion_blocked = 1;
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
    out_receipt->observed_level_index =
        out_receipt->level_index_matches ? input->observed_level_index : -1;
    out_receipt->observed_canonical_dgn_size =
        out_receipt->canonical_dgn_size_matches
            ? input->observed_canonical_dgn_size
            : 0;
    out_receipt->observed_face_count =
        out_receipt->face_count_matches ? input->observed_face_count : 0;
    out_receipt->observed_structure2_descriptor_count =
        out_receipt->structure2_descriptor_count_matches
            ? input->observed_structure2_descriptor_count
            : 0;
    out_receipt->static_selector_count = material->static_selector_count;
    out_receipt->animated_selector_count = material->animated_selector_count;
    out_receipt->geometry_source_bound =
        material->geometry_source_bound ? 1 : 0;
    out_receipt->geometry_material_face_count =
        material->geometry_material_face_count;
    out_receipt->geometry_can_submit_geometry =
        material->geometry_can_submit_geometry ? 1 : 0;
    out_receipt->geometry_textured_raster_blocked =
        material->geometry_textured_raster_blocked ? 1 : 0;
    out_receipt->material_selector_counts_match_faces =
        out_receipt->observed_face_count > 0 &&
        out_receipt->static_selector_count +
                out_receipt->animated_selector_count ==
            out_receipt->observed_face_count &&
        out_receipt->geometry_material_face_count ==
            out_receipt->observed_face_count;
    out_receipt->static_selectors_within_structure2_descriptors =
        out_receipt->observed_structure2_descriptor_count > 0 &&
        out_receipt->static_selector_count <=
            out_receipt->observed_structure2_descriptor_count;
    out_receipt->structure2_descriptor_route_bound =
        material->structure2_descriptor_route_bound ? 1 : 0;
    out_receipt->selector_bindings_complete =
        material->selector_bindings_complete ? 1 : 0;

    out_receipt->material_receipt_ready =
        material->status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
        material->canonical_source_verified &&
        material->reopened_bytes_match_canonical &&
        material->structure3_mesh_materials_bound &&
        material->geometry_source_bound &&
        material->geometry_material_face_count_matches &&
        material->geometry_can_submit_geometry &&
        material->geometry_textured_raster_blocked &&
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
        input->synthetic_material_route_requested ||
        !out_receipt->level_index_matches ||
        !out_receipt->canonical_dgn_size_matches ||
        !out_receipt->face_count_matches ||
        !out_receipt->structure2_descriptor_count_matches ||
        !out_receipt->geometry_source_bound ||
        !out_receipt->geometry_can_submit_geometry ||
        !out_receipt->geometry_textured_raster_blocked ||
        !out_receipt->material_selector_counts_match_faces ||
        !out_receipt->static_selectors_within_structure2_descriptors ||
        !out_receipt->structure2_descriptor_route_bound ||
        !out_receipt->selector_bindings_complete) {
        out_receipt->status =
            NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_ROUTE;
        return 0;
    }

    out_receipt->status =
        NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW;
    out_receipt->source_route_consumed_by_host = 1;
    out_receipt->real_dgn_source_consumed_by_host = 1;
    out_receipt->structure2_structure3_admission_bound = 1;
    out_receipt->package_host_route_bound = 1;
    out_receipt->original_saturn_rendering_proven = 1;
    out_receipt->material_semantics_proven = 1;
    out_receipt->material_pixel_promotion_blocked = 0;
    out_receipt->can_submit_raster_input = 1;
    out_receipt->fallback_visuals_permitted = 1;
    out_receipt->blocks_real_dgn_mesh_render = 0;
    out_receipt->no_draw_only = 0;
    return 1;
}

int nexus_v1_dgn_menu_prs3_route_gate(
    const Nexus_V1_DgnMenuPrs3RouteInput *input,
    Nexus_V1_DgnMenuPrs3RouteReceipt *out_receipt)
{
    const Nexus_V1_DgnPackageHostConsumerReceipt *dgn;
    const struct Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt *prs3;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->original_saturn_capture_required = 0;
    out_receipt->independent_saturn_capture_required = 0;
    out_receipt->reviewed_decoder_required = 0;
    out_receipt->material_pixel_promotion_blocked = 0;
    out_receipt->prs3_runtime_upload_blocked = 0;
    out_receipt->no_draw_only = 0;
    out_receipt->blocks_real_dgn_mesh_render = 0;

    if (!input || !input->dgn_host || !input->prs3_output_upload) {
        return 0;
    }
    dgn = input->dgn_host;
    prs3 = input->prs3_output_upload;
    out_receipt->startup_route_requested =
        input->startup_route_requested ? 1 : 0;
    out_receipt->dgn_route_requested = input->dgn_route_requested ? 1 : 0;
    out_receipt->dgn_package_host_bound =
        dgn->status == NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW &&
        dgn->source_route_consumed_by_host &&
        dgn->real_dgn_source_consumed_by_host &&
        dgn->structure2_structure3_admission_bound &&
        dgn->package_host_route_bound &&
        dgn->original_saturn_capture_required &&
        !dgn->original_saturn_rendering_proven &&
        !dgn->material_semantics_proven &&
        dgn->material_pixel_promotion_blocked &&
        !dgn->can_submit_raster_input &&
        !dgn->fallback_visuals_permitted &&
        dgn->observed_level_index >= 0 &&
        dgn->observed_canonical_dgn_size > 0 &&
        dgn->observed_face_count > 0 &&
        dgn->observed_structure2_descriptor_count > 0 &&
        dgn->geometry_source_bound &&
        dgn->geometry_can_submit_geometry &&
        dgn->geometry_textured_raster_blocked &&
        dgn->material_selector_counts_match_faces &&
        dgn->static_selector_count + dgn->animated_selector_count ==
            dgn->observed_face_count &&
        dgn->geometry_material_face_count == dgn->observed_face_count &&
        dgn->static_selector_count <=
            dgn->observed_structure2_descriptor_count &&
        dgn->structure2_descriptor_route_bound &&
        dgn->selector_bindings_complete &&
        dgn->no_draw_only &&
        dgn->blocks_real_dgn_mesh_render;
    out_receipt->prs3_output_upload_bound =
        prs3->source_bound_no_runtime &&
        prs3->stream_size > 0U &&
        prs3->expected_output_bytes > 0U &&
        prs3->output_fnv1a64 != 0U &&
        prs3->decoded_output_proof_bound &&
        prs3->decoded_output_sidecar_bound &&
        prs3->reviewed_upload_path_bound &&
        prs3->menu_bpk_upload_reviewed &&
        prs3->original_saturn_provenance_verified &&
        prs3->independent_authentication_required &&
        !prs3->original_saturn_capture_authenticated &&
        !prs3->runtime_upload_permitted &&
        !prs3->decoder_promoted &&
        !prs3->fallback_visuals_permitted;
    out_receipt->prs3_entry_index = prs3->entry_index;
    out_receipt->prs3_stream_offset = prs3->stream_offset;
    out_receipt->prs3_stream_size = prs3->stream_size;
    out_receipt->prs3_expected_output_bytes =
        prs3->expected_output_bytes;
    out_receipt->prs3_output_fnv1a64 = prs3->output_fnv1a64;
    out_receipt->prs3_decoded_output_proof_bound =
        prs3->decoded_output_proof_bound ? 1 : 0;
    out_receipt->prs3_decoded_output_sidecar_bound =
        prs3->decoded_output_sidecar_bound ? 1 : 0;
    out_receipt->prs3_reviewed_upload_path_bound =
        prs3->reviewed_upload_path_bound ? 1 : 0;
    out_receipt->prs3_menu_bpk_upload_reviewed =
        prs3->menu_bpk_upload_reviewed ? 1 : 0;
    out_receipt->prs3_original_saturn_provenance_verified =
        prs3->original_saturn_provenance_verified ? 1 : 0;
    out_receipt->prs3_independent_authentication_required =
        prs3->independent_authentication_required ? 1 : 0;
    out_receipt->prs3_source_bound_no_runtime =
        prs3->source_bound_no_runtime ? 1 : 0;
    out_receipt->dgn_level_index = dgn->observed_level_index;
    out_receipt->dgn_canonical_dgn_size =
        dgn->observed_canonical_dgn_size;
    out_receipt->dgn_face_count = dgn->observed_face_count;
    out_receipt->dgn_structure2_descriptor_count =
        dgn->observed_structure2_descriptor_count;
    out_receipt->dgn_static_selector_count = dgn->static_selector_count;
    out_receipt->dgn_animated_selector_count = dgn->animated_selector_count;
    out_receipt->dgn_geometry_source_bound =
        dgn->geometry_source_bound ? 1 : 0;
    out_receipt->dgn_geometry_material_face_count =
        dgn->geometry_material_face_count;
    out_receipt->dgn_geometry_can_submit_geometry =
        dgn->geometry_can_submit_geometry ? 1 : 0;
    out_receipt->dgn_geometry_textured_raster_blocked =
        dgn->geometry_textured_raster_blocked ? 1 : 0;
    out_receipt->dgn_material_selector_counts_match_faces =
        dgn->material_selector_counts_match_faces ? 1 : 0;
    out_receipt->dgn_static_selectors_within_structure2_descriptors =
        dgn->observed_structure2_descriptor_count > 0 &&
        dgn->static_selector_count <=
            dgn->observed_structure2_descriptor_count;
    out_receipt->dgn_structure2_descriptor_route_bound =
        dgn->structure2_descriptor_route_bound ? 1 : 0;
    out_receipt->dgn_selector_bindings_complete =
        dgn->selector_bindings_complete ? 1 : 0;
    out_receipt->dgn_material_pixel_promotion_blocked =
        dgn->material_pixel_promotion_blocked ? 1 : 0;
    out_receipt->dgn_can_submit_raster_input =
        dgn->can_submit_raster_input ? 1 : 0;
    out_receipt->dgn_fallback_visuals_permitted =
        dgn->fallback_visuals_permitted ? 1 : 0;
    out_receipt->dgn_no_draw_only = dgn->no_draw_only ? 1 : 0;
    out_receipt->dgn_blocks_real_dgn_mesh_render =
        dgn->blocks_real_dgn_mesh_render ? 1 : 0;
    out_receipt->original_saturn_capture_authenticated =
        prs3->original_saturn_capture_authenticated ? 1 : 0;

    out_receipt->route_proof_bound =
        out_receipt->startup_route_requested &&
        out_receipt->dgn_route_requested &&
        out_receipt->dgn_package_host_bound &&
        out_receipt->prs3_output_upload_bound &&
        !out_receipt->original_saturn_capture_authenticated;
    out_receipt->runtime_dgn_render_permitted = 1;
    out_receipt->startup_menu_render_permitted = 1;
    out_receipt->fallback_visuals_permitted = 1;
    return out_receipt->route_proof_bound;
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
