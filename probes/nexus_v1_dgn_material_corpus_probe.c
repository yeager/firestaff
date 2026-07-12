#include "nexus_v1_engine.h"

#include <stdio.h>
#include <string.h>

/* Skip-safe real-media probe. It reports typed Structure1B selectors,
 * Structure1G declarations, and the bounded Structure2 descriptor envelope;
 * opaque Structure2 bytes remain unexamined and never become materials. */
int main(int argc, char **argv) {
    Nexus_V1_Engine engine;
    Nexus_V1_DgnMaterialCorpusReceipt receipt;
    const char *root = argc > 1 ? argv[1] : NULL;

    memset(&engine, 0, sizeof(engine));
    memset(&receipt, 0, sizeof(receipt));
    if (nexus_v1_init(&engine, root) < 0) {
        puts("SKIP: Nexus Track 1 data unavailable");
        return 0;
    }
    (void)nexus_v1_inspect_dgn_material_corpus(&engine, &receipt);
    printf("LEV corpus: readable=%d parsed=%d geometry=%d expected=%d\n",
           receipt.readable_level_count, receipt.parsed_level_count,
           receipt.geometry_ready_level_count, receipt.expected_level_count);
    printf("Material refs: floor=%u ceiling=%u wall=%u\n",
           receipt.floor_coverage.command_count,
           receipt.ceiling_coverage.command_count,
           receipt.wall_coverage.command_count);
    printf("Typed DGN: 1F levels=%d entries=%d; 1G present=%d valid=%d "
           "animations=%d sequences=%d images=%d gotos=%d "
           "animated-floors=%d bound=%d\n",
           receipt.structure1f_valid_level_count,
           receipt.structure1f_typed_entry_count,
           receipt.structure1g_present_level_count,
           receipt.structure1g_valid_level_count,
           receipt.structure1g_animated_texture_count,
           receipt.structure1g_sequence_count,
           receipt.structure1g_image_instruction_count,
           receipt.structure1g_goto_instruction_count,
           receipt.structure1g_floor_animation_cell_count,
           receipt.structure1g_floor_animation_bound_count);
    printf("Structure2: valid-levels=%d descriptors=%d 1G-first-bound=%d "
           "payload-envelopes=%d opaque-bytes=%d proven-material-levels=%d "
           "canonical-sources=%d bound-payloads=%d offsets=%d/%d/%d "
           "local-pattern-levels=%d\n",
           receipt.structure2_valid_level_count,
           receipt.structure2_texture_count,
           receipt.structure1g_structure2_first_image_bound_count,
           receipt.structure2_payload_envelope_valid_level_count,
           receipt.structure2_opaque_payload_byte_count,
           receipt.structure2_material_or_image_data_proven_level_count,
           receipt.structure2_canonical_source_verified_level_count,
           receipt.structure2_materialization_bound_level_count,
           receipt.structure2_nonzero_descriptor_offset_count,
           receipt.structure2_descriptor_offsets_in_opaque_payload_count,
           receipt.structure2_descriptor_offsets_outside_opaque_payload_count,
           receipt.structure2_local_payload_offset_pattern_level_count);
    printf("Coverage: floor=%d ceiling=%d wall=%d mns-host=%d bpk-host=%d complete=%d\n",
           receipt.floor_coverage.covered, receipt.ceiling_coverage.covered,
           receipt.wall_coverage.covered,
           receipt.static_mns_host_route_complete,
           receipt.bpk_host_routes_complete,
           receipt.host_route_evidence_complete);
    printf("Containers: floors present=%d format=%d identity=%d host=%d; "
           "walls present=%d format=%d identity=%d host=%d\n",
           receipt.floor_container.source_present,
           receipt.floor_container.format_valid,
           receipt.floor_container.identity_verified,
           receipt.floor_container.host_route_permitted,
           receipt.wall_container.source_present,
           receipt.wall_container.format_valid,
           receipt.wall_container.identity_verified,
           receipt.wall_container.host_route_permitted);
    nexus_v1_shutdown(&engine);
    return receipt.parsed_level_count == receipt.expected_level_count &&
           receipt.geometry_ready_level_count == receipt.expected_level_count &&
           receipt.static_mns_host_route_complete &&
           !receipt.bpk_host_routes_complete
        ? 0 : 1;
}
