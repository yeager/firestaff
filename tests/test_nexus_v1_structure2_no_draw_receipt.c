#include "nexus_v1_engine.h"
#include "nexus_v1_viewport.h"

#include <stdio.h>
#include <string.h>

int main(void) {
    Nexus_V1_Engine engine;
    Nexus_Viewport viewport;
    Nexus_V1_DgnViewportRenderReceipt render;
    Nexus_V1_DgnStructure2SourceReceipt source;

    memset(&engine, 0, sizeof(engine));
    engine.level_loaded = 1;
    engine.game.current_level = 0;
    engine.game.party_x = 11;
    engine.game.party_y = 29;
    engine.current_level.geometry_info.dmweb_container = 1;
    engine.current_level_structure2_source.level_index = 1;
    engine.current_level_structure2_source.canonical_hash_verified = 1;
    engine.current_level_structure2_source.loaded_bytes_bound = 1;
    engine.current_level_structure2_source.materialization_bound = 1;
    if (nexus_v1_current_level_structure2_source_receipt(&engine, &source) != 0 ||
        source.canonical_hash_verified || source.loaded_bytes_bound ||
        source.materialization_bound || source.level_index != 0) {
        fprintf(stderr, "Stale Structure2 source receipt escaped the active-level gate\n");
        return 1;
    }
    engine.current_level_structure2_source.level_index = 0;
    engine.current_level_structure2_source.structure2_payload_envelope_valid = 0;
    if (nexus_v1_current_level_structure2_source_receipt(&engine, &source) != 0 ||
        source.structure2_payload_envelope_valid ||
        source.fallback_visuals_permitted) {
        fprintf(stderr, "Invalid Structure2 descriptor envelope escaped the source gate\n");
        return 1;
    }
    engine.dgn_material_plan.external_structure1f_placement_bound = 1;
    engine.dgn_material_plan.external_structure1f_placement_dgn_fnv1a64 = 1U;
    engine.dgn_material_plan.external_structure1f_placement_descriptor_index = 2U;
    engine.dgn_material_plan.external_structure1f_placement_frame_sequence = 3U;
    engine.dgn_material_plan.external_structure1f_placement_command_sequence = 4U;
    engine.dgn_material_plan.external_structure1f_placement_descriptor_fnv1a64 = 5U;
    engine.dgn_material_plan.external_prs3_placement_route_epoch = 6U;
    if (nexus_v1_prepare_dgn_material_plan(&engine, 11, 29, 0) != NULL ||
        engine.dgn_material_plan.receipt.structure2_source_materialization_bound ||
        engine.dgn_material_plan.external_prs3_placement_bound ||
        engine.dgn_material_plan.external_prs3_placement_trace_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_header_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_bitmap_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_bitmap_offset ||
        engine.dgn_material_plan.external_prs3_placement_bitmap_size ||
        engine.dgn_material_plan.external_prs3_placement_palt_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_palt_size ||
        engine.dgn_material_plan.external_prs3_placement_dgn_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_descriptor_index ||
        engine.dgn_material_plan.external_prs3_placement_frame_sequence ||
        engine.dgn_material_plan.external_prs3_placement_command_sequence ||
        engine.dgn_material_plan.external_prs3_placement_route_epoch ||
        engine.dgn_material_plan.external_prs3_placement_descriptor_target_bound ||
        engine.dgn_material_plan.external_prs3_placement_descriptor_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_image_anchor_offset ||
        engine.dgn_material_plan.external_prs3_placement_image_candidate_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_palette_anchor_offset ||
        engine.dgn_material_plan.external_prs3_placement_palette_candidate_fnv1a64 ||
        engine.dgn_material_plan.external_structure1f_placement_bound ||
        engine.dgn_material_plan.external_structure1f_placement_dgn_fnv1a64 ||
        engine.dgn_material_plan.external_structure1f_placement_descriptor_index ||
        engine.dgn_material_plan.external_structure1f_placement_frame_sequence ||
        engine.dgn_material_plan.external_structure1f_placement_command_sequence ||
        engine.dgn_material_plan.external_structure1f_placement_descriptor_fnv1a64 ||
        engine.dgn_material_plan.structure2_source_level_index != -1 ||
        engine.dgn_material_plan.structure2_source_canonical_hash_verified ||
        engine.dgn_material_plan.structure2_source_envelope_valid ||
        engine.dgn_material_plan.receipt.fallback_visuals_permitted) {
        fprintf(stderr, "Invalid Structure2 descriptor envelope reached the material plan\n");
        return 1;
    }
    memset(&engine.current_level_structure2_source, 0,
           sizeof(engine.current_level_structure2_source));

    if (nexus_v1_prepare_dgn_material_plan(&engine, 11, 29, 0) != NULL ||
        engine.dgn_material_plan.receipt.status ==
            NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH ||
        engine.dgn_material_plan.receipt.plan_ready ||
        engine.dgn_material_plan.receipt.command_count != 0 ||
        engine.dgn_material_plan.receipt.structure2_source_materialization_bound ||
        engine.dgn_material_plan.external_prs3_placement_bound ||
        engine.dgn_material_plan.external_prs3_placement_trace_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_header_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_bitmap_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_bitmap_offset ||
        engine.dgn_material_plan.external_prs3_placement_bitmap_size ||
        engine.dgn_material_plan.external_prs3_placement_palt_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_palt_size ||
        engine.dgn_material_plan.external_prs3_placement_dgn_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_descriptor_index ||
        engine.dgn_material_plan.external_prs3_placement_frame_sequence ||
        engine.dgn_material_plan.external_prs3_placement_command_sequence ||
        engine.dgn_material_plan.external_prs3_placement_route_epoch ||
        engine.dgn_material_plan.external_prs3_placement_descriptor_target_bound ||
        engine.dgn_material_plan.external_prs3_placement_descriptor_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_image_anchor_offset ||
        engine.dgn_material_plan.external_prs3_placement_image_candidate_fnv1a64 ||
        engine.dgn_material_plan.external_prs3_placement_palette_anchor_offset ||
        engine.dgn_material_plan.external_prs3_placement_palette_candidate_fnv1a64 ||
        engine.dgn_material_plan.external_structure1f_placement_bound ||
        engine.dgn_material_plan.external_structure1f_placement_dgn_fnv1a64 ||
        engine.dgn_material_plan.external_structure1f_placement_descriptor_index ||
        engine.dgn_material_plan.external_structure1f_placement_frame_sequence ||
        engine.dgn_material_plan.external_structure1f_placement_command_sequence ||
        engine.dgn_material_plan.external_structure1f_placement_descriptor_fnv1a64 ||
        engine.dgn_material_plan.structure2_source_level_index != -1 ||
        engine.dgn_material_plan.structure2_source_canonical_hash_verified ||
        engine.dgn_material_plan.structure2_source_envelope_valid ||
        engine.dgn_material_plan.structure2_floor_command_source_receipt
            .source_command_count != 0 ||
        engine.dgn_material_plan.structure2_floor_command_sources_consumed ||
        engine.dgn_material_plan.structure1f_item_command_binding_receipt
            .bound_regular_inventory_count != 0 ||
        engine.dgn_material_plan.structure1f_item_command_sources_consumed ||
        engine.dgn_material_plan.structure1f_item_floor_material_receipt
            .command_material_count != 0 ||
        engine.dgn_material_plan.structure1f_item_floor_materials_consumed ||
        engine.dgn_material_plan.structure1f_direct_floor_source_receipt
            .floor_command_source_count != 0 ||
        engine.dgn_material_plan.structure1f_direct_floor_sources_consumed ||
        engine.dgn_material_plan.structure1a_owned_cell_source_receipt
            .floor_command_source_count != 0 ||
        engine.dgn_material_plan.structure1a_owned_cell_sources_consumed ||
        engine.dgn_material_plan.structure1a_structure3_topology_candidate_receipt
            .topology_candidate_count != 0 ||
        engine.dgn_material_plan.structure1a_structure3_topology_candidates_consumed ||
        engine.dgn_material_plan.receipt.fallback_visuals_permitted) {
        fprintf(stderr, "Incomplete DGN data did not produce a no-draw plan\n");
        return 1;
    }

    nexus_viewport_init(&viewport);
    nexus_viewport_render(&viewport, &engine);
    if (nexus_viewport_last_dgn_render_receipt(&viewport, &render) != 0 ||
        !render.attempted || !render.used_real_dgn_route || !render.blocked ||
        render.no_draw_structure2_source || render.command_count != 0 ||
        render.fallback_visuals_permitted) {
        fprintf(stderr, "Incomplete DGN data did not produce a no-draw viewport receipt\n");
        return 1;
    }

    printf("Nexus incomplete-data no-draw receipt: OK\n");
    return 0;
}
