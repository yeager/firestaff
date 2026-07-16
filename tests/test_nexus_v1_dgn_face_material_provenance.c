#include "nexus_v1_dgn_face_material_provenance.h"

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

    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
               receipt.canonical_source_verified &&
               receipt.reopened_bytes_match_canonical &&
               receipt.canonical_dgn_size == (int)sizeof(retail_dgn) &&
               receipt.face_count == 3 &&
               receipt.structure2_descriptor_count == 4 &&
               receipt.static_selector_count == 2 &&
               receipt.animated_selector_count == 1 &&
               receipt.structure3_mesh_materials_bound &&
               receipt.structure2_descriptor_route_bound &&
               receipt.selector_bindings_complete &&
               !receipt.material_semantics_proven &&
               receipt.package_host_route_bound &&
               receipt.no_draw_only &&
               receipt.blocks_real_dgn_mesh_render &&
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
               receipt.no_draw_only &&
               receipt.blocks_real_dgn_mesh_render &&
               !receipt.permits_fallback_visuals,
           "static Structure3 selectors must resolve into Structure2 descriptors");
    input.structure2_descriptor_count = 4;
    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.package_host_route_bound &&
               receipt.structure2_descriptor_route_bound &&
               receipt.selector_bindings_complete &&
               !receipt.material_semantics_proven &&
               receipt.blocks_real_dgn_mesh_render &&
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
               &consumer_input, &consumer) == 1 &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW &&
               consumer.material_receipt_ready &&
               consumer.host_route_requested &&
               consumer.package_route_consumed &&
               consumer.level_index_matches &&
               consumer.canonical_dgn_size_matches &&
               consumer.face_count_matches &&
               consumer.structure2_descriptor_count_matches &&
               consumer.source_route_consumed_by_host &&
               consumer.package_host_route_bound &&
               consumer.original_saturn_capture_required &&
               !consumer.original_saturn_rendering_proven &&
               !consumer.material_semantics_proven &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               consumer.blocks_real_dgn_mesh_render &&
               consumer.no_draw_only,
           "host route consumes bounded real DGN receipt without opening rendering");

    consumer_input.observed_face_count = 2;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_ROUTE &&
               consumer.material_receipt_ready &&
               !consumer.face_count_matches &&
               !consumer.source_route_consumed_by_host &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               consumer.blocks_real_dgn_mesh_render,
           "stale host face-count metadata cannot consume the package route");
    consumer_input.observed_face_count = 3;

    receipt.material_semantics_proven = 1;
    expect(!nexus_v1_dgn_package_host_consumer_gate(
               &consumer_input, &consumer) &&
               consumer.status ==
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL &&
               !consumer.material_receipt_ready &&
               !consumer.source_route_consumed_by_host &&
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
                   NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_ROUTE &&
               consumer.material_receipt_ready &&
               !consumer.package_route_consumed &&
               !consumer.package_host_route_bound &&
               !consumer.can_submit_raster_input &&
               !consumer.fallback_visuals_permitted &&
               strcmp(nexus_v1_dgn_package_host_consumer_status_name(
                          consumer.status),
                      "blocked-route") == 0,
           "host route consumption requires an explicit package-consumed bit");
    consumer_input.package_route_consumed = 1;

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
