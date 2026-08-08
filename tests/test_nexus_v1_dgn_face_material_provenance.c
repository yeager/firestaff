#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    static const uint8_t retail_dgn[] = {0x44, 0x47, 0x4e, 0x00};
    Nexus_V1_DgnFaceMaterialBinding bindings[] = {
        {0, 3, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC},
        {1, 1, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED},
        {2, 0, NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC}
    };
    Nexus_V1_DgnFaceMaterialInput input;
    Nexus_V1_DgnFaceMaterialReceipt receipt;
    Nexus_V1_DgnPackageHostConsumerInput consumer_input;
    Nexus_V1_DgnPackageHostConsumerReceipt consumer;
    Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt prs3_output_upload;
    Nexus_V1_DgnMenuPrs3RouteInput route_input;
    Nexus_V1_DgnMenuPrs3RouteReceipt route_receipt;

    memset(&input, 0, sizeof(input));
    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.dgn_bytes = retail_dgn;
    input.dgn_size = (int)sizeof(retail_dgn);
    input.canonical_dgn_bytes = retail_dgn;
    input.canonical_dgn_size = (int)sizeof(retail_dgn);
    input.canonical_source_verified = 1;
    input.bindings = bindings;
    input.face_count = 3;
    input.structure2_descriptor_count = 4;
    input.material_selector_count = 4;
    input.geometry_source_bound = 1;
    input.geometry_material_face_count = 3;
    input.geometry_can_submit_geometry = 1;
    input.geometry_can_submit_textured_raster = 0;
    input.geometry_fallback_visuals_permitted = 0;

    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
               receipt.canonical_source_verified &&
               receipt.reopened_bytes_match_canonical &&
               receipt.canonical_dgn_size == (int)sizeof(retail_dgn) &&
               receipt.face_count == 3 &&
               receipt.structure2_descriptor_count == 4 &&
               receipt.static_selector_count == 2 &&
               receipt.animated_selector_count == 1 &&
               receipt.geometry_source_bound &&
               receipt.geometry_material_face_count == 3 &&
               receipt.geometry_material_face_count_matches &&
               receipt.geometry_can_submit_geometry &&
               receipt.geometry_textured_raster_blocked &&
               receipt.structure3_mesh_materials_bound &&
               receipt.structure2_descriptor_route_bound &&
               receipt.selector_bindings_complete &&
               !receipt.material_semantics_proven &&
               receipt.package_host_route_bound &&
               !receipt.no_draw_only &&
               !receipt.blocks_real_dgn_mesh_render &&
               receipt.original_saturn_capture_required &&
               !receipt.original_saturn_capture_available &&
               !receipt.can_submit_raster_input &&
               !receipt.permits_fallback_visuals,
           "canonical retail DGN material bindings stop at the package/host no-draw boundary");

    {
        uint8_t reopened_dgn[sizeof(retail_dgn)];

        memcpy(reopened_dgn, retail_dgn, sizeof(reopened_dgn));
        reopened_dgn[2] ^= 0x01U;
        input.dgn_bytes = reopened_dgn;
        expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
                   receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
                   !receipt.reopened_bytes_match_canonical &&
                   !receipt.can_submit_raster_input &&
                   !receipt.permits_fallback_visuals,
               "a changed launcher-reopened DGN buffer cannot inherit canonical admission");
        input.dgn_bytes = retail_dgn;
    }

    input.canonical_dgn_size = (int)sizeof(retail_dgn) - 1;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input,
           "a shortened canonical catalog buffer cannot admit a reopened DGN buffer");
    input.canonical_dgn_size = (int)sizeof(retail_dgn);

    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_SYNTHETIC;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input && !receipt.permits_fallback_visuals,
           "synthetic DGN fixtures cannot enter the raster-input boundary");

    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.canonical_source_verified = 0;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE,
           "unverified retail bytes cannot be promoted by their source label");

    input.canonical_source_verified = 1;
    bindings[2].face_ordinal = 1;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING,
           "duplicate face ordinals fail closed");
    bindings[2].face_ordinal = 2;
    input.structure2_descriptor_count = 3;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING &&
               !receipt.package_host_route_bound &&
               !receipt.no_draw_only &&
               !receipt.blocks_real_dgn_mesh_render &&
               !receipt.permits_fallback_visuals,
           "static Structure3 selectors must resolve into Structure2 descriptors");
    input.structure2_descriptor_count = 4;
    input.geometry_material_face_count = 2;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input &&
               !receipt.permits_fallback_visuals,
           "material bindings require a matching parser-validated geometry material-face count");
    input.geometry_material_face_count = 3;
    input.geometry_can_submit_textured_raster = 1;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input &&
               !receipt.permits_fallback_visuals,
           "pre-promoted textured geometry cannot enter the material source path");
    input.geometry_can_submit_textured_raster = 0;
    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.package_host_route_bound &&
               receipt.geometry_source_bound &&
               receipt.geometry_material_face_count_matches &&
               receipt.geometry_textured_raster_blocked &&
               receipt.structure2_descriptor_route_bound &&
               receipt.selector_bindings_complete &&
               !receipt.material_semantics_proven &&
               !receipt.blocks_real_dgn_mesh_render &&
               !receipt.can_submit_raster_input &&
               !receipt.permits_fallback_visuals,
           "Structure2/Structure3 binding proof is not drawable material semantics");

    memset(&consumer_input, 0, sizeof(consumer_input));
    consumer_input.material_receipt = &receipt;
    consumer_input.host_route_requested = 1;
    consumer_input.package_route_consumed = 1;
    consumer_input.expected_level_index = 0;
    consumer_input.observed_level_index = 0;
    consumer_input.expected_canonical_dgn_size = (int)sizeof(retail_dgn);
    consumer_input.observed_canonical_dgn_size = (int)sizeof(retail_dgn);
    consumer_input.expected_face_count = 3;
    consumer_input.observed_face_count = 3;
    consumer_input.expected_structure2_descriptor_count = 4;
    consumer_input.observed_structure2_descriptor_count = 4;

    expect(nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) == 0 &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               consumer.host_route_requested &&
               consumer.package_route_consumed &&
               consumer.level_index_matches &&
               consumer.canonical_dgn_size_matches &&
               consumer.face_count_matches &&
               consumer.structure2_descriptor_count_matches &&
               consumer.observed_level_index == 0 &&
               consumer.observed_canonical_dgn_size ==
                   (int)sizeof(retail_dgn) &&
               consumer.observed_face_count == 3 &&
               consumer.observed_structure2_descriptor_count == 4 &&
               consumer.static_selector_count == 2 &&
               consumer.animated_selector_count == 1 &&
               consumer.geometry_source_bound &&
               consumer.geometry_material_face_count == 3 &&
               consumer.geometry_can_submit_geometry &&
               consumer.geometry_textured_raster_blocked &&
               consumer.material_selector_counts_match_faces &&
               consumer.static_selectors_within_structure2_descriptors &&
               consumer.structure2_descriptor_route_bound &&
               consumer.selector_bindings_complete &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               !consumer.synthetic_material_route_rejected &&
               !consumer.structure2_structure3_admission_bound &&
               !consumer.package_host_route_bound &&
               !consumer.original_saturn_capture_required &&
               !consumer.original_saturn_rendering_proven &&
               !consumer.material_semantics_proven &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               !consumer.blocks_real_dgn_mesh_render &&
               !consumer.no_draw_only,
           "host route consumption is blocked because the material receipt no longer carries no-draw provenance");

    memset(&prs3_output_upload, 0, sizeof(prs3_output_upload));
    prs3_output_upload.entry_index = 4U;
    prs3_output_upload.stream_offset = 0x200U;
    prs3_output_upload.stream_size = 0x40U;
    prs3_output_upload.expected_output_bytes = 0x80U;
    prs3_output_upload.output_fnv1a64 = 0x12345678U;
    prs3_output_upload.decoded_output_proof_bound = 1;
    prs3_output_upload.decoded_output_sidecar_bound = 1;
    prs3_output_upload.reviewed_upload_path_bound = 1;
    prs3_output_upload.menu_bpk_upload_reviewed = 1;
    prs3_output_upload.original_saturn_provenance_verified = 1;
    prs3_output_upload.independent_authentication_required = 1;
    prs3_output_upload.source_bound_no_runtime = 1;
    memset(&route_input, 0, sizeof(route_input));
    route_input.dgn_host = &consumer;
    route_input.prs3_output_upload = &prs3_output_upload;
    route_input.startup_route_requested = 1;
    route_input.dgn_route_requested = 1;
    expect(nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) == 0 &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.startup_route_requested &&
               route_receipt.dgn_route_requested &&
               !route_receipt.route_proof_bound &&
               !route_receipt.original_saturn_capture_required &&
               !route_receipt.independent_saturn_capture_required &&
               !route_receipt.original_saturn_capture_authenticated &&
               !route_receipt.reviewed_decoder_required &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               !route_receipt.material_pixel_promotion_blocked &&
               !route_receipt.prs3_runtime_upload_blocked &&
               route_receipt.fallback_visuals_permitted &&
               !route_receipt.no_draw_only &&
               !route_receipt.blocks_real_dgn_mesh_render &&
               route_receipt.prs3_decoded_output_proof_bound &&
               route_receipt.prs3_decoded_output_sidecar_bound &&
               route_receipt.prs3_reviewed_upload_path_bound &&
               route_receipt.prs3_menu_bpk_upload_reviewed &&
               route_receipt.prs3_original_saturn_provenance_verified &&
               route_receipt.prs3_independent_authentication_required &&
               route_receipt.prs3_source_bound_no_runtime &&
               route_receipt.dgn_level_index == 0 &&
               route_receipt.dgn_canonical_dgn_size ==
                   (int)sizeof(retail_dgn) &&
               route_receipt.dgn_face_count == 3 &&
               route_receipt.dgn_structure2_descriptor_count == 4 &&
               route_receipt.dgn_static_selector_count == 2 &&
               route_receipt.dgn_animated_selector_count == 1 &&
               route_receipt.dgn_geometry_source_bound &&
               route_receipt.dgn_geometry_material_face_count == 3 &&
               route_receipt.dgn_geometry_can_submit_geometry &&
               route_receipt.dgn_geometry_textured_raster_blocked &&
               route_receipt.dgn_material_selector_counts_match_faces &&
               route_receipt
                   .dgn_static_selectors_within_structure2_descriptors &&
               route_receipt.dgn_structure2_descriptor_route_bound &&
               route_receipt.dgn_selector_bindings_complete &&
               route_receipt.dgn_material_pixel_promotion_blocked &&
               !route_receipt.dgn_can_submit_raster_input &&
               !route_receipt.dgn_fallback_visuals_permitted &&
               !route_receipt.dgn_no_draw_only &&
               !route_receipt.dgn_blocks_real_dgn_mesh_render &&
               route_receipt.prs3_entry_index == 4U &&
               route_receipt.prs3_stream_offset == 0x200U &&
               route_receipt.prs3_stream_size == 0x40U &&
               route_receipt.prs3_expected_output_bytes == 0x80U &&
               route_receipt.prs3_output_fnv1a64 == 0x12345678U,
           "DGN material host route plus PRS3 output/upload proof stays no-runtime");
    route_input.dgn_route_requested = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires an explicit DGN route request");
    route_input.dgn_route_requested = 1;
    prs3_output_upload.fallback_visuals_permitted = 1;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               !route_receipt.prs3_output_upload_bound &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof rejects PRS3 evidence with fallback visuals");
    prs3_output_upload.fallback_visuals_permitted = 0;
    prs3_output_upload.output_fnv1a64 = 0U;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               !route_receipt.prs3_output_upload_bound &&
               route_receipt.prs3_stream_size == 0x40U &&
               route_receipt.prs3_expected_output_bytes == 0x80U &&
               route_receipt.prs3_output_fnv1a64 == 0U &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires a concrete PRS3 output fingerprint");
    prs3_output_upload.output_fnv1a64 = 0x12345678U;

    prs3_output_upload.decoded_output_sidecar_bound = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               !route_receipt.prs3_output_upload_bound &&
               !route_receipt.prs3_decoded_output_sidecar_bound &&
               route_receipt.prs3_output_fnv1a64 == 0x12345678U &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires the decoded output sidecar binding");
    prs3_output_upload.decoded_output_sidecar_bound = 1;

    prs3_output_upload.menu_bpk_upload_reviewed = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               !route_receipt.prs3_output_upload_bound &&
               route_receipt.prs3_reviewed_upload_path_bound &&
               !route_receipt.prs3_menu_bpk_upload_reviewed &&
               route_receipt.prs3_original_saturn_provenance_verified &&
               route_receipt.prs3_independent_authentication_required &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires reviewed MENU.BPK upload evidence");
    prs3_output_upload.menu_bpk_upload_reviewed = 1;

    prs3_output_upload.source_bound_no_runtime = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               !route_receipt.prs3_output_upload_bound &&
               route_receipt.prs3_decoded_output_proof_bound &&
               route_receipt.prs3_decoded_output_sidecar_bound &&
               !route_receipt.prs3_source_bound_no_runtime &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires a no-runtime PRS3 source boundary");
    prs3_output_upload.source_bound_no_runtime = 1;

    consumer.geometry_material_face_count = 2;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_geometry_material_face_count == 2 &&
               route_receipt.dgn_material_selector_counts_match_faces &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN geometry material-count identity");
    consumer.geometry_material_face_count = 3;

    consumer.geometry_source_bound = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               !route_receipt.dgn_geometry_source_bound &&
               route_receipt.dgn_geometry_can_submit_geometry &&
               route_receipt.dgn_geometry_textured_raster_blocked &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN geometry source route");
    consumer.geometry_source_bound = 1;

    consumer.geometry_can_submit_geometry = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_geometry_source_bound &&
               !route_receipt.dgn_geometry_can_submit_geometry &&
               route_receipt.dgn_geometry_textured_raster_blocked &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN geometry admission");
    consumer.geometry_can_submit_geometry = 1;

    consumer.geometry_textured_raster_blocked = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_geometry_source_bound &&
               route_receipt.dgn_geometry_can_submit_geometry &&
               !route_receipt.dgn_geometry_textured_raster_blocked &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN textured-raster blocker");
    consumer.geometry_textured_raster_blocked = 1;

    consumer.material_pixel_promotion_blocked = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               !route_receipt.dgn_material_pixel_promotion_blocked &&
               !route_receipt.dgn_can_submit_raster_input &&
               !route_receipt.dgn_fallback_visuals_permitted &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN pixel-promotion blocker");
    consumer.material_pixel_promotion_blocked = 1;

    consumer.can_submit_raster_input = 1;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_material_pixel_promotion_blocked &&
               route_receipt.dgn_can_submit_raster_input &&
               !route_receipt.dgn_fallback_visuals_permitted &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN raster-input blocker");
    consumer.can_submit_raster_input = 0;

    consumer.fallback_visuals_permitted = 1;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_material_pixel_promotion_blocked &&
               !route_receipt.dgn_can_submit_raster_input &&
               route_receipt.dgn_fallback_visuals_permitted &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN fallback blocker");
    consumer.fallback_visuals_permitted = 0;

    consumer.no_draw_only = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               !route_receipt.dgn_no_draw_only &&
               !route_receipt.dgn_blocks_real_dgn_mesh_render &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN no-draw host boundary");
    consumer.no_draw_only = 1;

    consumer.blocks_real_dgn_mesh_render = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_no_draw_only &&
               !route_receipt.dgn_blocks_real_dgn_mesh_render &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained real-DGN mesh render blocker");
    consumer.blocks_real_dgn_mesh_render = 1;

    consumer.original_saturn_capture_required = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_no_draw_only &&
               route_receipt.dgn_blocks_real_dgn_mesh_render &&
               !route_receipt.route_proof_bound &&
               !route_receipt.original_saturn_capture_required &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN original-Saturn capture requirement");
    consumer.original_saturn_capture_required = 1;

    consumer.original_saturn_rendering_proven = 1;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_no_draw_only &&
               route_receipt.dgn_blocks_real_dgn_mesh_render &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof rejects pre-promoted DGN Saturn rendering");
    consumer.original_saturn_rendering_proven = 0;

    consumer.material_semantics_proven = 1;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_no_draw_only &&
               route_receipt.dgn_blocks_real_dgn_mesh_render &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof rejects pre-promoted DGN material semantics");
    consumer.material_semantics_proven = 0;

    consumer.observed_structure2_descriptor_count = 1;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_structure2_descriptor_count == 1 &&
               !route_receipt
                    .dgn_static_selectors_within_structure2_descriptors &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires static selectors to remain within Structure2 descriptors");
    consumer.observed_structure2_descriptor_count = 4;

    consumer.selector_bindings_complete = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               !route_receipt.dgn_selector_bindings_complete &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained selector binding completeness");
    consumer.selector_bindings_complete = 1;

    consumer.structure2_descriptor_route_bound = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               !route_receipt.dgn_structure2_descriptor_route_bound &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained Structure2 descriptor source route");
    consumer.structure2_descriptor_route_bound = 1;

    consumer.observed_face_count = 0;
    expect(!nexus_v1_dgn_menu_prs3_route_gate(
               &route_input, &route_receipt) &&
               !route_receipt.dgn_package_host_bound &&
               route_receipt.prs3_output_upload_bound &&
               route_receipt.dgn_face_count == 0 &&
               !route_receipt.route_proof_bound &&
               route_receipt.runtime_dgn_render_permitted &&
               route_receipt.startup_menu_render_permitted &&
               route_receipt.fallback_visuals_permitted,
           "joined DGN/PRS3 route proof requires retained DGN face-count identity");
    consumer.observed_face_count = 3;

    consumer_input.observed_face_count = 2;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.face_count_matches &&
               consumer.observed_face_count == 0 &&
               !consumer.material_selector_counts_match_faces &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               !consumer.blocks_real_dgn_mesh_render,
           "stale host face-count metadata cannot consume the package route");
    consumer_input.observed_face_count = 3;

    consumer_input.host_route_requested = 0;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.host_route_requested &&
               consumer.package_route_consumed &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               !consumer.package_host_route_bound &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted,
           "host route consumption requires an explicit real-DGN host request");
    consumer_input.host_route_requested = 1;

    consumer_input.observed_level_index = 1;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.level_index_matches &&
               consumer.observed_level_index == -1 &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               !consumer.package_host_route_bound &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted,
           "host route consumption requires retained DGN level identity");
    consumer_input.observed_level_index = 0;

    consumer_input.observed_canonical_dgn_size =
        (int)sizeof(retail_dgn) - 1;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.canonical_dgn_size_matches &&
               consumer.observed_canonical_dgn_size == 0 &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               !consumer.package_host_route_bound &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted,
           "host route consumption requires retained canonical DGN size identity");
    consumer_input.observed_canonical_dgn_size = (int)sizeof(retail_dgn);

    consumer_input.observed_structure2_descriptor_count = 3;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.structure2_descriptor_count_matches &&
               consumer.observed_structure2_descriptor_count == 0 &&
               !consumer.static_selectors_within_structure2_descriptors &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               !consumer.package_host_route_bound &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted,
           "host route consumption requires retained Structure2 descriptor-count identity");
    consumer_input.observed_structure2_descriptor_count = 4;

    receipt.material_semantics_proven = 1;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               strcmp(nexus_v1_dgn_package_host_consumer_status_name(
                          consumer.status),
                      "blocked-material") == 0,
           "consumer rejects receipts that claim unreviewed material semantics");
    receipt.material_semantics_proven = 0;

    consumer_input.package_route_consumed = 0;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.package_route_consumed &&
               !consumer.package_host_route_bound &&
               !consumer.real_dgn_source_consumed_by_host &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               strcmp(nexus_v1_dgn_package_host_consumer_status_name(
                          consumer.status),
                      "blocked-material") == 0,
           "host route consumption requires an explicit package-consumed bit");
    consumer_input.package_route_consumed = 1;

    consumer_input.synthetic_material_route_requested = 1;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               consumer.synthetic_material_route_rejected &&
               !consumer.source_route_consumed_by_host &&
               !consumer.real_dgn_source_consumed_by_host &&
               !consumer.structure2_structure3_admission_bound &&
               consumer.material_pixel_promotion_blocked &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               !consumer.blocks_real_dgn_mesh_render,
           "synthetic material host routes cannot consume real Structure3 material admission");
    consumer_input.synthetic_material_route_requested = 0;

    bindings[1].material_selector = 4;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING &&
               strcmp(nexus_v1_dgn_face_material_status_name(receipt.status),
                      "blocked-binding") == 0,
           "out-of-range material selectors fail closed with stable status");

    if (failures) {
        fprintf(stderr, "Nexus DGN face/material provenance: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN face/material provenance passed");
    return 0;
}
